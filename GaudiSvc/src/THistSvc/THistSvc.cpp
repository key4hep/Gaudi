#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   TODO: To be removed, since it comes from ROOT TMathBase.h
#pragma warning( disable : 2259 )
#endif

#include "THistSvc.h"

#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IIoComponentMgr.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Property.h"
#include "boost/algorithm/string/case_conv.hpp"

#include "TDirectory.h"
#include "TError.h"
#include "TFile.h"
#include "TGraph.h"
#include "TKey.h"
#include "TROOT.h"

#include <cstdio>
#include <sstream>
#include <streambuf>

using namespace std;

DECLARE_COMPONENT( THistSvc )

namespace
{

  template <typename InputIterator, typename OutputIterator, typename UnaryOperation, typename UnaryPredicate>
  OutputIterator transform_if( InputIterator first, InputIterator last, OutputIterator result, UnaryOperation op,
                               UnaryPredicate pred )
  {
    while ( first != last ) {
      if ( pred( *first ) ) *result++ = op( *first );
      ++first;
    }
    return result;
  }

  constexpr struct select1st_t {
    template <typename T, typename S>
    const T& operator()( const std::pair<T, S>& p ) const
    {
      return p.first;
    }
  } select1st{};
}

//*************************************************************************//

THistSvc::THistSvc( const std::string& name, ISvcLocator* svc ) : base_class( name, svc )
{
  m_compressionLevel.declareUpdateHandler( &THistSvc::setupCompressionLevel, this );
  m_outputfile.declareUpdateHandler( &THistSvc::setupOutputFile, this );
  m_inputfile.declareUpdateHandler( &THistSvc::setupInputFile, this );
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::initialize() {
  GlobalDirectoryRestore restore(m_svcMut);

  StatusCode status = Service::initialize();

  if ( status.isFailure() ) {
    error() << "initializing service" << endmsg;
    return status;
  }

  StatusCode st( StatusCode::SUCCESS );

  try {
    setupOutputFile( m_outputfile );
  } catch ( GaudiException& err ) {
    error() << "Caught: " << err << endmsg;
    st = StatusCode::FAILURE;
  }

  try {
    setupInputFile( m_inputfile );
  } catch ( GaudiException& err ) {
    error() << "Caught: " << err << endmsg;
    st = StatusCode::FAILURE;
  }

  // Protect against multiple instances of TROOT
  if ( !gROOT ) {
    static TROOT root( "root", "ROOT I/O" );
    //    gDebug = 99;
  } else {
    if ( msgLevel( MSG::VERBOSE ) ) verbose() << "ROOT already initialized, debug = " << gDebug << endmsg;
  }

  if ( service( "IncidentSvc", p_incSvc, true ).isFailure() ) {
    error() << "unable to get the IncidentSvc" << endmsg;
    st = StatusCode::FAILURE;
  } else {
    p_incSvc->addListener( this, "EndEvent", 100, true );
  }

  if ( service( "FileMgr", p_fileMgr, true ).isFailure() ) {
    error() << "unable to get the FileMgr" << endmsg;
    st = StatusCode::FAILURE;
  } else {
    debug() << "got the FileMgr" << endmsg;
  }

  // Register open/close callback actions

  using namespace std::placeholders;
  auto boa = [this]( const Io::FileAttr* fa, const std::string& caller ) { return this->rootOpenAction( fa, caller ); };
  if ( p_fileMgr->regAction( boa, Io::OPEN, Io::ROOT ).isFailure() ) {
    error() << "unable to register ROOT file open action with FileMgr" << endmsg;
  }
  auto bea = [this]( const Io::FileAttr* fa, const std::string& caller ) {
    return this->rootOpenErrAction( fa, caller );
  };
  if ( p_fileMgr->regAction( bea, Io::OPEN_ERR, Io::ROOT ).isFailure() ) {
    error() << "unable to register ROOT file open Error action with FileMgr" << endmsg;
  }

  m_okToConnect = true;

  if ( m_delayConnect ) {
    if ( !m_inputfile.value().empty() ) {
      setupInputFile( m_inputfile );
    }
    if ( !m_outputfile.value().empty() ) {
      setupOutputFile( m_outputfile );
    }

    m_delayConnect = false;
  }
  m_alreadyConnectedOutFiles.clear();
  m_alreadyConnectedInFiles.clear();

  IIoComponentMgr* iomgr = nullptr;

  if ( service( "IoComponentMgr", iomgr, true ).isFailure() ) {
    error() << "unable to get the IoComponentMgr" << endmsg;
    st = StatusCode::FAILURE;
  } else {

    if ( !iomgr->io_register( this ).isSuccess() ) {
      error() << "could not register with the I/O component manager !" << endmsg;
      st = StatusCode::FAILURE;
    } else {
      bool all_good = true;
      // register input/output files...
      for ( const auto& reg : m_files ) {
        const std::string& fname = reg.second.first->GetName();
        const IIoComponentMgr::IoMode::Type iomode =
            ( reg.second.second == THistSvc::READ ? IIoComponentMgr::IoMode::READ : IIoComponentMgr::IoMode::WRITE );
        if ( !iomgr->io_register( this, iomode, fname ).isSuccess() ) {
          warning() << "could not register file [" << fname << "] with the I/O component manager..." << endmsg;
          all_good = false;
        } else {
          info() << "registered file [" << fname << "]... [ok]" << endmsg;
        }
      }
      if ( !all_good ) {
        error() << "problem while registering input/output files with "
                << "the I/O component manager !" << endmsg;
        st = StatusCode::FAILURE;
      }
    }
  }

  if ( st.isFailure() ) {
    fatal() << "Unable to initialize THistSvc" << endmsg;
  }

  return st;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::reinitialize()
{

  GlobalDirectoryRestore restore(m_svcMut);
  m_log << MSG::WARNING << "reinitialize not implemented" << endmsg;
  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::finalize()
{

  GlobalDirectoryRestore restore(m_svcMut);

  dump();

  if ( msgLevel( MSG::DEBUG ) ) debug() << "THistSvc::finalize" << endmsg;

#ifndef NDEBUG
  if (m_log.level() <= MSG::DEBUG) {
  for (const auto& uid : m_uidsX ) {

      TObject* to = uid.second.obj;

      string dirname( "none" );
      if ( to && to->IsA()->InheritsFrom( "TTree" ) ) {
        TTree* tr = dynamic_cast<TTree*>( to );
        if ( tr->GetDirectory() != 0 ) {
          dirname = tr->GetDirectory()->GetPath();
        }
      } else if ( to && to->IsA()->InheritsFrom( "TGraph" ) ) {
        if ( !uid.second.temp ) {
          dirname = uid.second.file->GetPath();
          string id2( uid.second.id );
          id2.erase( 0, id2.find( "/", 1 ) );
          id2.erase( id2.rfind( "/" ), id2.length() );
          if ( id2.find( "/" ) == 0 ) {
            id2.erase( 0, 1 );
          }
          dirname += id2;
        } else {
          dirname = "/tmp";
        }
      } else if ( to && to->IsA()->InheritsFrom( "TH1" ) ) {
        TH1* th = dynamic_cast<TH1*>( to );
        if ( th == 0 ) {
          error() << "Couldn't dcast: " << uid.first << endmsg;
        } else {
          if ( th->GetDirectory() != 0 ) {
            dirname = th->GetDirectory()->GetPath();
          }
        }
      } else if ( !to ) {
        warning() << uid.first << " has NULL TObject ptr" << endmsg;
      }

      debug() << "uid: \"" << uid.first << "\"  temp: " << uid.second.temp << "  dir: " << dirname << endmsg;
    }
  }
#endif

  StatusCode sc = write();
  if ( sc.isFailure() ) {
    error() << "problems writing histograms" << endmsg;
  }

  if ( m_print ) {
    info() << "Listing contents of ROOT files: " << endmsg;
  }
  vector<TFile*> deleted_files;
  for ( auto& itr : m_files ) {

    if ( find( deleted_files.begin(), deleted_files.end(), itr.second.first ) == deleted_files.end() ) {
      deleted_files.push_back( itr.second.first );

#ifndef NDEBUG
      if ( msgLevel( MSG::DEBUG ) )
        debug() << "finalizing stream/file " << itr.first << ":" << itr.second.first->GetName() << endmsg;
#endif
    } else {
#ifndef NDEBUG
      if ( msgLevel( MSG::DEBUG ) ) debug() << "already finalized stream " << itr.first << endmsg;
#endif
      continue;
    }

    if ( m_print && msgLevel( MSG::INFO ) ) {

      info() << "==> File: " << itr.second.first->GetName() << "  stream: " << itr.first << endmsg;

      itr.second.first->Print( "base" );
    }

    string tmpfn = itr.second.first->GetName();

    p_fileMgr->close( itr.second.first, name() );

    IIncidentSvc* pi = nullptr;
    if ( service( "IncidentSvc", pi ).isFailure() ) {
      error() << "Unable to get the IncidentSvc" << endmsg;
      return StatusCode::FAILURE;
    }

    if ( itr.second.second == SHARE ) {

      // Merge File
      void* vf = nullptr;
      int r    = p_fileMgr->open( Io::ROOT, name(), m_sharedFiles[itr.first], Io::WRITE | Io::APPEND, vf, "HIST" );

      if ( r ) {
        error() << "unable to open Final Output File: \"" << m_sharedFiles[itr.first] << "\" for merging" << endmsg;
        return StatusCode::FAILURE;
      }

      TFile* outputfile = (TFile*)vf;
      pi->fireIncident( FileIncident( name(), IncidentType::WroteToOutputFile, m_sharedFiles[itr.first] ) );

      if ( msgLevel( MSG::DEBUG ) ) debug() << "THistSvc::write()::Merging Rootfile " << endmsg;

      vf = nullptr;
      r  = p_fileMgr->open( Io::ROOT, name(), tmpfn, Io::READ, vf, "HIST" );

      if ( r ) {
        error() << "unable to open temporary file: \"" << tmpfn << endmsg;
        return StatusCode::FAILURE;
      }

      TFile* inputfile = (TFile*)vf;

      outputfile->SetCompressionLevel( inputfile->GetCompressionLevel() );

      MergeRootFile( outputfile, inputfile );

      outputfile->Write();
      p_fileMgr->close( outputfile, name() );
      p_fileMgr->close( inputfile, name() );

      if ( msgLevel( MSG::DEBUG ) ) debug() << "Trying to remove temporary file \"" << tmpfn << "\"" << endmsg;

      std::remove( tmpfn.c_str() );
    }
    delete itr.second.first;
  }

  m_sharedFiles.clear();
  m_fileStreams.clear();
  m_files.clear();
  m_uidsX.clear();
  m_idsX.clear();
  m_tobjs.clear();

  return Service::finalize();
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

bool THistSvc::browseTDir( TDirectory* dir ) const
{

  if ( !dir ) {
    std::cerr << "TDirectory == 0" << std::endl;
    return false;
  }

  GlobalDirectoryRestore restore(m_svcMut);

  dir->cd();

  cout << "-> " << dir->GetPath() << "  " << dir->GetListOfKeys()->GetSize() << endl;

  //  TIter nextkey(dir->GetListOfKeys());
  TIter nextkey( dir->GetList() );
  while ( TKey* key = (TKey*)nextkey() ) {

    TObject* obj = key->ReadObj();
    if ( !obj ) {
      cout << key->GetName() << " obj==0" << endl;
      continue;
    }
    //    if (obj->IsA()->InheritsFrom("TDirectory")) {
    cout << "  Key: " << key->GetName() << "   "
         << " tit: " << obj->GetTitle() << "   "
         << " (" << key->GetClassName() << ")" << endl;
    //    }
  }

  nextkey = dir->GetListOfKeys();
  while ( TKey* key = (TKey*)nextkey() ) {

    TObject* obj = key->ReadObj();
    if ( !obj ) {
      cout << key->GetName() << " obj==0" << endl;
      continue;
    }
    if ( obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      TDirectory* tt = dynamic_cast<TDirectory*>( obj );
      browseTDir( tt );
    }
  }

  return true;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTHists(TDirectory *td, TList & tl, bool rcs) const {
  GlobalDirectoryRestore restore(m_svcMut);

  gErrorIgnoreLevel = kBreak;

  if ( !td->cd() ) {
    error() << "getTHists: No such TDirectory \"" << td->GetPath() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "getTHists: \"" << td->GetPath() << "\": found " << td->GetListOfKeys()->GetSize() << " keys" << endmsg;

  TIter nextkey( td->GetListOfKeys() );
  while ( TKey* key = (TKey*)nextkey() ) {
    auto& log = debug();
    if ( msgLevel( MSG::DEBUG ) ) log << "  key: " << key->GetName();
    TObject* obj = key->ReadObj();
    if ( obj != 0 && obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " (" << obj->IsA()->GetName() << ")";
    } else if ( obj != 0 && obj->IsA()->InheritsFrom( "TH1" ) ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " (" << obj->IsA()->GetName() << ")";
      tl.Add( obj );
    } else if ( obj != 0 ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " [" << obj->IsA()->GetName() << "]";
    }
    if ( msgLevel( MSG::DEBUG ) ) log << endmsg;
  }

  // operate recursively
  if ( rcs ) {
    nextkey = td->GetListOfKeys();
    while ( TKey* key = (TKey*)nextkey() ) {
      TObject* obj = key->ReadObj();
      if ( obj && obj->IsA()->InheritsFrom( "TDirectory" ) ) {
        TDirectory* tt = dynamic_cast<TDirectory*>( obj );
        getTHists( tt, tl, rcs );
      }
    }
  }

  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::getTHists( const std::string& dir, TList& tl, bool rcs ) const
{

  GlobalDirectoryRestore restore(m_svcMut);

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream, rem, r2;
  parseString( dir, stream, rem );

  auto itr = m_files.find( stream );
  if ( itr != m_files.end() ) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if ( msgLevel( MSG::DEBUG ) )
      debug() << "getTHists: \"" << dir << "\" looks like a stream name."
              << " associated TFile: \"" << itr->second.first->GetName() << "\"" << endmsg;

    if ( gDirectory->cd( r2.c_str() ) ) {
      m_curstream = stream;
      sc          = getTHists( gDirectory, tl, rcs );
      m_curstream = "";
      return sc;
    } else {
      if ( msgLevel( MSG::DEBUG ) ) debug() << "getTHists: no such TDirectory \"" << r2 << "\"" << endmsg;
    }

  } else {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "getTHists: stream \"" << stream << "\" not found" << endmsg;
  }

  if ( !gDirectory->cd( dir.c_str() ) ) {
    error() << "getTHists: No such TDirectory/stream \"" << dir << "\"" << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    sc = getTHists( gDirectory, tl, rcs );
  }

  return sc;
}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTTrees(TDirectory *td, TList & tl, bool rcs) const {
  GlobalDirectoryRestore restore(m_svcMut);

  gErrorIgnoreLevel = kBreak;

  if ( !td->cd() ) {
    error() << "getTTrees: No such TDirectory \"" << td->GetPath() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "getTHists: \"" << td->GetPath() << "\": found " << td->GetListOfKeys()->GetSize() << " keys" << endmsg;

  TIter nextkey( td->GetListOfKeys() );
  while ( TKey* key = (TKey*)nextkey() ) {
    auto& log = debug();
    if ( msgLevel( MSG::DEBUG ) ) log << "  key: " << key->GetName();
    TObject* obj = key->ReadObj();
    if ( obj != 0 && obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " (" << obj->IsA()->GetName() << ")";
    } else if ( obj != 0 && obj->IsA()->InheritsFrom( "TTree" ) ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " (" << obj->IsA()->GetName() << ")";
      tl.Add( obj );
    } else if ( obj != 0 ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " [" << obj->IsA()->GetName() << "]";
    }
    log << endmsg;
  }

  // operate recursively
  if ( rcs ) {
    nextkey = td->GetListOfKeys();
    while ( TKey* key = (TKey*)nextkey() ) {
      TObject* obj = key->ReadObj();
      if ( obj && obj->IsA()->InheritsFrom( "TDirectory" ) ) {
        TDirectory* tt = dynamic_cast<TDirectory*>( obj );
        getTTrees( tt, tl, rcs );
      }
    }
  }

  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getTTrees(const std::string& dir, TList & tl, bool rcs) const {
  GlobalDirectoryRestore restore(m_svcMut);

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream, rem, r2;
  parseString( dir, stream, rem );

  auto itr = m_files.find( stream );
  if ( itr != m_files.end() ) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if ( msgLevel( MSG::DEBUG ) )
      debug() << "getTTrees: \"" << dir << "\" looks like a stream name."
              << " associated TFile: \"" << itr->second.first->GetName() << "\"" << endmsg;

    if ( gDirectory->cd( r2.c_str() ) ) {
      return getTTrees( gDirectory, tl, rcs );
    }
    if ( msgLevel( MSG::DEBUG ) ) debug() << "getTTrees: no such TDirectory \"" << r2 << "\"" << endmsg;
  } else {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "getTTrees: stream \"" << stream << "\" not found" << endmsg;
  }

  if ( !gDirectory->cd( dir.c_str() ) ) {
    error() << "getTTrees: No such TDirectory/stream \"" << dir << "\"" << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    sc = getTTrees( gDirectory, tl, rcs );
  }
  return sc;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::getTHists( TDirectory* td, TList& tl, bool rcs, bool reg )
{

  GlobalDirectoryRestore restore(m_svcMut);

  gErrorIgnoreLevel = kBreak;

  if ( !td->cd() ) {
    error() << "getTHists: No such TDirectory \"" << td->GetPath() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "getTHists: \"" << td->GetPath() << "\": found " << td->GetListOfKeys()->GetSize() << " keys" << endmsg;

  TIter nextkey( td->GetListOfKeys() );
  while ( TKey* key = (TKey*)nextkey() ) {
    auto& log = debug();
    if ( msgLevel( MSG::DEBUG ) ) log << "  key: " << key->GetName();
    TObject* obj = key->ReadObj();
    if ( obj && obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " (" << obj->IsA()->GetName() << ")";
    } else if ( obj && obj->IsA()->InheritsFrom( "TH1" ) ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " (" << obj->IsA()->GetName() << ")";
      tl.Add( obj );
      if ( reg && m_curstream != "" ) {
        string dir = td->GetPath();
        string fil = td->GetFile()->GetName();
        dir.erase( 0, fil.length() + 1 );
        string id = "/" + m_curstream;
        if ( dir == "/" ) {
          id = id + "/" + key->GetName();
        } else {
          id = id + dir + "/" + key->GetName();
        }
        if ( !exists( id ) ) {
          if ( msgLevel( MSG::DEBUG ) ) log << "  reg as \"" << id << "\"";
          regHist( id ).ignore();
        } else {
          if ( msgLevel( MSG::DEBUG ) ) log << "  already registered";
        }
      }
    } else if ( obj ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " [" << obj->IsA()->GetName() << "]";
    }
    if ( msgLevel( MSG::DEBUG ) ) log << endmsg;
  }

  // operate recursively
  if ( rcs ) {
    nextkey = td->GetListOfKeys();
    while ( TKey* key = (TKey*)nextkey() ) {
      TObject* obj = key->ReadObj();
      if ( obj && obj->IsA()->InheritsFrom( "TDirectory" ) ) {
        TDirectory* tt = dynamic_cast<TDirectory*>( obj );
        getTHists( tt, tl, rcs, reg );
      }
    }
  }

  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::getTHists( const std::string& dir, TList& tl, bool rcs, bool reg )
{

  GlobalDirectoryRestore restore(m_svcMut);

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream, rem, r2;
  parseString( dir, stream, rem );

  auto itr = m_files.find( stream );
  if ( itr != m_files.end() ) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if ( msgLevel( MSG::DEBUG ) )
      debug() << "getTHists: \"" << dir << "\" looks like a stream name."
              << " associated TFile: \"" << itr->second.first->GetName() << "\"" << endmsg;

    if ( gDirectory->cd( r2.c_str() ) ) {
      m_curstream = stream;
      sc          = getTHists( gDirectory, tl, rcs, reg );
      m_curstream.clear();
      return sc;
    }
    if ( msgLevel( MSG::DEBUG ) ) debug() << "getTHists: no such TDirectory \"" << r2 << "\"" << endmsg;

  } else {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "getTHists: stream \"" << stream << "\" not found" << endmsg;
  }

  if ( !gDirectory->cd( dir.c_str() ) ) {
    error() << "getTHists: No such TDirectory/stream \"" << dir << "\"" << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    if ( reg ) {
      warning() << "Unable to register histograms automatically "
                << "without a valid stream name" << endmsg;
      reg = false;
    }
    sc = getTHists( gDirectory, tl, rcs, reg );
  }

  return sc;
}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::getTTrees( TDirectory* td, TList& tl, bool rcs, bool reg )
{

  GlobalDirectoryRestore restore(m_svcMut);

  gErrorIgnoreLevel = kBreak;

  if ( !td->cd() ) {
    error() << "getTTrees: No such TDirectory \"" << td->GetPath() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "getTHists: \"" << td->GetPath() << "\": found " << td->GetListOfKeys()->GetSize() << " keys" << endmsg;

  TIter nextkey( td->GetListOfKeys() );
  while ( TKey* key = (TKey*)nextkey() ) {
    auto& log = debug();
    if ( msgLevel( MSG::DEBUG ) ) log << "  key: " << key->GetName();
    TObject* obj = key->ReadObj();
    if ( obj && obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " (" << obj->IsA()->GetName() << ")";
    } else if ( obj && obj->IsA()->InheritsFrom( "TTree" ) ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " (" << obj->IsA()->GetName() << ")";
      tl.Add( obj );
      if ( reg && m_curstream != "" ) {
        string dir = td->GetPath();
        string fil = td->GetFile()->GetName();
        dir.erase( 0, fil.length() + 1 );
        string id = "/" + m_curstream;
        if ( dir == "/" ) {
          id = id + "/" + key->GetName();
        } else {
          id = id + dir + "/" + key->GetName();
        }
        if ( !exists( id ) ) {
          if ( msgLevel( MSG::DEBUG ) ) log << "  reg as \"" << id << "\"";
          regHist( id ).ignore();
        } else {
          if ( msgLevel( MSG::DEBUG ) ) log << "  already registered";
        }
      }
    } else if ( obj != 0 ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " [" << obj->IsA()->GetName() << "]";
    }
    if ( msgLevel( MSG::DEBUG ) ) log << endmsg;
  }

  // operate recursively
  if ( rcs ) {
    nextkey = td->GetListOfKeys();
    while ( TKey* key = (TKey*)nextkey() ) {
      TObject* obj = key->ReadObj();
      if ( obj && obj->IsA()->InheritsFrom( "TDirectory" ) ) {
        TDirectory* tt = dynamic_cast<TDirectory*>( obj );
        getTTrees( tt, tl, rcs, reg );
      }
    }
  }

  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::getTTrees( const std::string& dir, TList& tl, bool rcs, bool reg )
{

  GlobalDirectoryRestore restore(m_svcMut);

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream, rem, r2;
  parseString( dir, stream, rem );

  auto itr = m_files.find( stream );
  if ( itr != m_files.end() ) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if ( msgLevel( MSG::DEBUG ) )
      debug() << "getTTrees: \"" << dir << "\" looks like a stream name."
              << " associated TFile: \"" << itr->second.first->GetName() << "\"" << endmsg;

    if ( gDirectory->cd( r2.c_str() ) ) {
      return getTTrees( gDirectory, tl, rcs, reg );
    } else {
      if ( msgLevel( MSG::DEBUG ) ) debug() << "getTTrees: no such TDirectory \"" << r2 << "\"" << endmsg;
    }

  } else {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "getTTrees: stream \"" << stream << "\" not found" << endmsg;
  }

  if ( !gDirectory->cd( dir.c_str() ) ) {
    error() << "getTTrees: No such TDirectory/stream \"" << dir << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  return getTTrees( gDirectory, tl, rcs, reg );
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::deReg( TObject* obj )
{

  auto itr = m_tobjs.find(obj);
  if (itr != m_tobjs.end()) {
    THistID hid = itr->second.first->at(itr->second.second);

    auto itr2 = m_uidsX.find(hid.id);
    if (itr2 == m_uidsX.end()) {
      m_log << MSG::ERROR << "Problems deregistering TObject \""
            << obj->GetName()
            << "\" with id \"" << hid.id << "\"" << endmsg;
      return StatusCode::FAILURE;
    }

    std::string id, root, rem;
    parseString( hid.id, root, rem );

    auto mitr = m_idsX.equal_range(rem);
    auto itr3 = std::find_if( mitr.first, mitr.second, [&](idXMap::const_reference i) 
                                   { return i.second.obj == obj; } ) ;
    if (itr3 == mitr.second ) {
      m_log << MSG::ERROR << "Problems deregistering TObject \""
            << obj->GetName()
            << "\" with id \"" << hid.id << "\"" << endmsg;
      return StatusCode::FAILURE;
    }

    m_tobjs.erase(itr);
    m_uidsX.erase(itr2);
    m_idsX.erase(itr3);

    return StatusCode::SUCCESS;

  } else {
    error() << "Cannot unregister TObject \"" << obj->GetName() << "\": not known to THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::deReg( const std::string& id )
{

  auto itr = m_uidsX.find(id);
  if (itr == m_uidsX.end()) {
    m_log << MSG::ERROR << "Problems deregistering id \""
          << id << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  TObject* obj = itr->second.obj;
  return deReg( obj );
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::regHist( const std::string& id )
{

  TH1 *hist = nullptr;
  THistID* hid;
  return regHist_i(hist, id, false, hid);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regHist(const std::string& id, TH1* hist) {
  THistID* hid;
  return regHist_i(hist, id, false, hid);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regHist(const std::string& id, TH2* hist) {
  THistID* hid;
  return regHist_i(hist, id, false, hid);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regHist(const std::string& id, TH3* hist) {
  THistID* hid;
  return regHist_i(hist, id, false, hid);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regTree(const std::string& id) {
  TTree *hist = nullptr;  
  THistID* hid;
  return regHist_i(hist, id, false, hid);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regTree(const std::string& id, TTree* hist) {
  THistID* hid;
  StatusCode sc = regHist_i(hist, id, false, hid);
  if (hist && sc.isSuccess()) {
    if (m_autoSave != 0) hist->SetAutoSave(m_autoSave);
    hist->SetAutoFlush(m_autoFlush);
  }
  return sc;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regGraph(const std::string& id) {
  TGraph *hist = nullptr;
  THistID* hid;
  return regHist_i(hist, id, false, hid);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::regGraph( const std::string& id, TGraph* hist )
{
  if ( strcmp( hist->GetName(), "Graph" ) == 0 ) {

    std::string id2( id );
    string::size_type i = id2.rfind( "/" );
    if ( i != string::npos ) {
      id2.erase( 0, i + 1 );
    }

    info() << "setting name of TGraph id: \"" << id << "\" to \"" << id2 << "\" since it is unset" << endmsg;
    hist->SetName( id2.c_str() );
  }

  THistID* hid;
  return regHist_i(hist, id, false, hid);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getHist(const std::string& id, TH1*& hist, size_t ind) const {
  return getHist_i(id, hist, ind);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getHist(const std::string& id, TH2*& hist, size_t ind) const {
  return getHist_i(id, hist, ind);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getHist(const std::string& id, TH3*& hist, size_t ind) const {
  return getHist_i(id, hist, ind);
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

std::vector<std::string> THistSvc::getHists() const
{

  std::vector<std::string> names;
  names.reserve(m_uidsX.size());
  transform_if( std::begin(m_uidsX), std::end(m_uidsX),
                std::back_inserter(names), select1st,
                [](uidXMap::const_reference i) { 
                    return i.second.obj->IsA()->InheritsFrom("TH1"); }
  );
  return names;
}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::getTree( const std::string& id, TTree*& hist ) const { return getHist_i( id, hist ); }

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

std::vector<std::string> THistSvc::getTrees() const
{

  std::vector<std::string> names;
  names.reserve(m_uidsX.size());
  transform_if( std::begin(m_uidsX), std::end(m_uidsX),
                std::back_inserter(names),
                select1st,
                [](uidXMap::const_reference i) { 
                    return i.second.obj->IsA()->InheritsFrom("TTree"); }
  );
  return names;
}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::getGraph( const std::string& id, TGraph*& hist ) const { return getHist_i( id, hist ); }

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

std::vector<std::string> THistSvc::getGraphs() const
{

  std::vector<std::string> names;
  names.reserve(m_uidsX.size());
  transform_if( std::begin(m_uidsX), std::end(m_uidsX),
                std::back_inserter(names), select1st,
                [](uidXMap::const_reference i) { 
                    return i.second.obj->IsA()->InheritsFrom("TGraph"); }
  );
  return names;
}
//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::readHist( const std::string& id, TH1*& hist ) const { return readHist_i( id, hist ); }

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::readHist( const std::string& id, TH2*& hist ) const { return readHist_i( id, hist ); }

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::readHist( const std::string& id, TH3*& hist ) const { return readHist_i( id, hist ); }

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::readTree( const std::string& id, TTree*& hist ) const { return readHist_i( id, hist ); }

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

bool THistSvc::findStream( const string& id, string& stream, string& rem, TFile*& file ) const
{

  auto pos = id.find( "/" );

  if (pos == string::npos) {
    // no "/" in id
    stream = "temp";
    rem = id;
  } else if (pos != 0) {
    // id does not start with "/"
    stream = "temp";
    rem    = id;
  } else {
    // id starts with "/"

    auto pos2 = id.find( "/", pos + 1 );

    if (pos2 == string::npos) {
      // need at least 2 "/" in format "/STREAM/name" or "/STREAM/dir/name"
      m_log << MSG::ERROR << "badly formed Hist/Tree id: \"" << id << "\""
            << endmsg;
      return false;
    }

    parseString( id, stream, rem );
  }

  if ( stream == "temp" ) {
    file = nullptr;
    return true;
  }

  auto itr = m_files.find( stream );
  file     = ( itr != m_files.end() ? itr->second.first : nullptr );
  if ( !file ) {
    warning() << "no stream \"" << stream << "\" associated with id: \"" << id << "\"" << endmsg;
  }

  return true;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void THistSvc::parseString( const string& id, string& root, string& rem ) const
{
  auto pos = id.find( "/" );

  if ( pos == string::npos ) {
    root.clear();
    rem = id;
  } else if ( pos == 0 ) {
    parseString( id.substr( 1 ), root, rem );
  } else {
    root = id.substr( 0, pos );
    rem  = id.substr( pos + 1 );
  }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void THistSvc::setupCompressionLevel( Gaudi::Details::PropertyBase& /* cl */ )
{

  warning() << "\"CompressionLevel\" Property has been deprecated. "
            << "Set it via the \"CL=\" parameter in the \"Output\" Property" << endmsg;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void THistSvc::setupInputFile( Gaudi::Details::PropertyBase& /*m_inputfile*/ )
{

  if ( FSMState() < Gaudi::StateMachine::CONFIGURED || !m_okToConnect ) {

    debug() << "Delaying connection of Input Files until Initialize"
            << ". now in " << FSMState() << endmsg;

    m_delayConnect = true;
  } else {

    debug() << "Now connecting of Input Files" << endmsg;

    StatusCode sc = StatusCode::SUCCESS;

    for ( const auto& itr : m_inputfile.value() ) {
      if ( m_alreadyConnectedInFiles.end() != m_alreadyConnectedInFiles.find( itr ) ) continue;
      if ( connect( itr ).isFailure() ) {
        sc = StatusCode::FAILURE;
      } else {
        m_alreadyConnectedInFiles.insert( itr );
      }
    }

    if ( !sc.isSuccess() ) {
      throw GaudiException( "Problem connecting inputfile !!", name(), StatusCode::FAILURE );
    }
  }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void THistSvc::setupOutputFile( Gaudi::Details::PropertyBase& /*m_outputfile*/ )
{
  if ( FSMState() < Gaudi::StateMachine::CONFIGURED || !m_okToConnect ) {
    debug() << "Delaying connection of Input Files until Initialize"
            << ". now in " << FSMState() << endmsg;
    m_delayConnect = true;
  } else {

    StatusCode sc = StatusCode::SUCCESS;
    for ( const auto& itr : m_outputfile.value() ) {
      if ( m_alreadyConnectedOutFiles.end() != m_alreadyConnectedOutFiles.find( itr ) ) continue;
      if ( connect( itr ).isFailure() ) {
        sc = StatusCode::FAILURE;
      } else {
        m_alreadyConnectedOutFiles.insert( itr );
      }
    }

    if ( !sc.isSuccess() ) {
      throw GaudiException( "Problem connecting outputfile !!", name(), StatusCode::FAILURE );
    }
  }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void THistSvc::updateFiles()
{

  // If TTrees grow beyond TTree::fgMaxTreeSize, a new file is
  // automatically created by root, and the old one closed. We
  // need to migrate all the UIDs over to show the correct file
  // pointer. This is ugly.

  if ( msgLevel( MSG::DEBUG ) ) debug() << "updateFiles()" << endmsg;


  for (auto uitr=m_uidsX.begin(); uitr != m_uidsX.end(); ++uitr) {
#ifndef NDEBUG
    if ( msgLevel( MSG::VERBOSE ) )
      verbose() << " update: " << uitr->first << " " << uitr->second.id << " " << uitr->second.mode << endmsg;
#endif
    TObject* to    = uitr->second.obj;
    TFile* oldFile = uitr->second.file;
    if ( !to ) {
      warning() << uitr->first << ": TObject == 0" << endmsg;
    } else if ( uitr->second.temp || uitr->second.mode == READ ) {
// do nothing - no need to check how big the file is since we
// are just reading it.
#ifndef NDEBUG
      if ( msgLevel( MSG::VERBOSE ) ) verbose() << "     skipping" << endmsg;
#endif

    } else if ( to->IsA()->InheritsFrom( "TTree" ) ) {
      TTree* tr      = dynamic_cast<TTree*>( to );
      TFile* newFile = tr->GetCurrentFile();

      if ( oldFile != newFile ) {
        std::string newFileName = newFile->GetName();
        std::string oldFileName, streamName, rem;
        TFile* dummy = nullptr;
        findStream( uitr->second.id, streamName, rem, dummy );

        for ( auto& itr : m_files ) {
          if ( itr.second.first == oldFile ) itr.second.first = newFile;
        }

        for (auto uitr2 = uitr; uitr2 != m_uidsX.end(); ++uitr2) {
          if (uitr2->second.file == oldFile) {
            uitr2->second.file = newFile;
          }
        }

        auto sitr = std::find_if( std::begin( m_fileStreams ), std::end( m_fileStreams ),
                                  [&]( streamMap::const_reference s ) { return s.second == streamName; } );
        if ( sitr != std::end( m_fileStreams ) ) oldFileName = sitr->first;

#ifndef NDEBUG
        if ( msgLevel( MSG::DEBUG ) )
          debug() << "migrating uid: " << uitr->second.id << "   stream: " << streamName
                  << "   oldFile: " << oldFileName << "   newFile: " << newFileName << endmsg;
#endif

        if ( !oldFileName.empty() ) {
          auto i = m_fileStreams.lower_bound( oldFileName );
          while ( i != std::end( m_fileStreams ) && i->first == oldFileName ) {

#ifndef NDEBUG
            if ( msgLevel( MSG::DEBUG ) )
              debug() << "changing filename \"" << i->first << "\" to \"" << newFileName << "\" for stream \""
                      << i->second << "\"" << endmsg;
#endif
            std::string nm = std::move( i->second );
            i              = m_fileStreams.erase( i );
            m_fileStreams.emplace( newFileName, std::move( nm ) );
          }

        } else {
          error() << "Problems updating fileStreams with new file name" << endmsg;
        }
      }
    }
  }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::write()
{

  updateFiles();

  std::for_each( m_files.begin(), m_files.end(), []( std::pair<const std::string, std::pair<TFile*, Mode>>& i ) {
    auto mode = i.second.second;
    auto file = i.second.first;
    if ( mode == WRITE || mode == UPDATE || mode == SHARE ) {
      file->Write( "", TObject::kOverwrite );
    } else if ( mode == APPEND ) {
      file->Write( "" );
    }
  } );

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "THistSvc::write()::List of Files connected in ROOT " << endmsg;
    TSeqCollection* filelist = gROOT->GetListOfFiles();
    for ( int ii = 0; ii < filelist->GetEntries(); ii++ ) {
      debug() << "THistSvc::write()::List of Files connected in ROOT: \"" << filelist->At( ii )->GetName() << "\""
              << endmsg;
    }
  }

  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::connect( const std::string& ident )
{

  auto loc      = ident.find( " " );
  string stream = ident.substr( 0, loc );
  char typ( 0 );
  typedef std::pair<std::string, std::string> Prop;
  std::vector<Prop> props;
  string filename, db_typ( "ROOT" );
  int cl( 1 );

  if ( loc != string::npos ) {
    using Parser = Gaudi::Utils::AttribStringParser;
    for ( auto attrib : Parser( ident.substr( loc + 1 ) ) ) {
      auto TAG = boost::algorithm::to_upper_copy( attrib.tag );
      auto VAL = boost::algorithm::to_upper_copy( attrib.value );

      if ( TAG == "FILE" || TAG == "DATAFILE" ) {
        filename = attrib.value;
        removeDoubleSlash( filename );
      } else if ( TAG == "OPT" ) {
        if ( VAL == "APPEND" || VAL == "UPDATE" ) {
          typ = 'A';
        } else if ( VAL == "CREATE" || VAL == "NEW" || VAL == "WRITE" ) {
          typ = 'N';
        } else if ( VAL == "RECREATE" ) {
          typ = 'R';
        } else if ( VAL == "SHARE" ) {
          typ = 'S';
        } else if ( VAL == "OLD" || VAL == "READ" ) {
          typ = 'O';
        } else {
          error() << "Unknown OPT: \"" << attrib.value << "\"" << endmsg;
          typ = 0;
        }
      } else if ( TAG == "TYP" ) {
        db_typ = std::move( attrib.value );
      } else if ( TAG == "CL" ) {
        cl = std::stoi( attrib.value );
      } else {
        props.emplace_back( attrib.tag, attrib.value );
      }
    }
  }

  if ( stream == "temp" ) {
    error() << "in JobOption \"" << ident << "\": stream name \"temp\" reserved." << endmsg;
    return StatusCode::FAILURE;
  }

  if ( db_typ != "ROOT" ) {
    error() << "in JobOption \"" << ident << "\": technology type \"" << db_typ << "\" not supported." << endmsg;
    return StatusCode::FAILURE;
  }

  if ( m_files.find( stream ) != m_files.end() ) {
    error() << "in JobOption \"" << ident << "\":\n stream \"" << stream << "\" already connected to file: \""
            << m_files[stream].first->GetName() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  Mode newMode;
  if ( typ == 'O' ) {
    newMode = THistSvc::READ;
  } else if ( typ == 'N' ) {
    newMode = THistSvc::WRITE;
  } else if ( typ == 'A' ) {
    newMode = THistSvc::APPEND;
  } else if ( typ == 'R' ) {
    newMode = THistSvc::UPDATE;
  } else if ( typ == 'S' ) {
    newMode = THistSvc::SHARE;
  } else {
    // something else?
    error() << "No OPT= specified or unknown access mode in: " << ident << endmsg;
    return StatusCode::FAILURE;
  }

  // Is this file already connected to another stream?
  if ( m_fileStreams.find( filename ) != m_fileStreams.end() ) {
    auto fitr = m_fileStreams.equal_range( filename );

    const std::string& oldstream = fitr.first->second;

    const auto& f_info = m_files[oldstream];

    if ( newMode != f_info.second ) {
      error() << "in JobOption \"" << ident << "\":\n file \"" << filename << "\" already opened by stream: \""
              << oldstream << "\" with different access mode." << endmsg;
      return StatusCode::FAILURE;
    } else {
      TFile* f2       = f_info.first;
      m_files[stream] = make_pair( f2, newMode );
      if ( msgLevel( MSG::DEBUG ) )
        debug() << "Connecting stream: \"" << stream << "\" to previously opened TFile: \"" << filename << "\""
                << endmsg;
      return StatusCode::SUCCESS;
    }
  }

  IIncidentSvc* pi = nullptr;
  if ( service( "IncidentSvc", pi ).isFailure() ) {
    error() << "Unable to get the IncidentSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  void* vf = nullptr;
  TFile* f = nullptr;

  if ( newMode == THistSvc::READ ) {
    // old file

    int r = p_fileMgr->open( Io::ROOT, name(), filename, Io::READ, vf, "HIST" );

    if ( r != 0 ) {
      error() << "Unable to open ROOT file " << filename << " for reading" << endmsg;
      return StatusCode::FAILURE;
    }

    f = (TFile*)vf;

    // FIX ME!
    pi->fireIncident( FileIncident( name(), "BeginHistFile", filename ) );

  } else if ( newMode == THistSvc::WRITE ) {
    // new file. error if file exists

    int r = p_fileMgr->open( Io::ROOT, name(), filename, ( Io::WRITE | Io::CREATE | Io::EXCL ), vf, "HIST" );

    if ( r != 0 ) {
      error() << "Unable to open ROOT file " << filename << " for writing" << endmsg;
      return StatusCode::FAILURE;
    }

    f = (TFile*)vf;

  } else if ( newMode == THistSvc::APPEND ) {
    // update file

    int r = p_fileMgr->open( Io::ROOT, name(), filename, ( Io::WRITE | Io::APPEND ), vf, "HIST" );
    if ( r != 0 ) {
      error() << "unable to open file \"" << filename << "\" for appending" << endmsg;
      return StatusCode::FAILURE;
    }

    f = (TFile*)vf;

  } else if ( newMode == THistSvc::SHARE ) {
    // SHARE file type
    // For SHARE files, all data will be stored in a temp file and will be merged into the target file
    // in write() when finalize(), this help to solve some confliction. e.g. with storegate

    static int ishared  = 0;
    string realfilename = filename;
    filename            = "tmp_THistSvc_" + std::to_string( ishared++ ) + ".root";

    if ( msgLevel( MSG::DEBUG ) )
      debug() << "Creating temp file \"" << filename << "\" and realfilename=" << realfilename << endmsg;
    m_sharedFiles[stream] = realfilename;

    int r = p_fileMgr->open( Io::ROOT, name(), filename, ( Io::WRITE | Io::CREATE | Io::EXCL ), vf, "HIST" );

    if ( r != 0 ) {
      error() << "Unable to open ROOT file " << filename << " for writing" << endmsg;
      return StatusCode::FAILURE;
    }

    f = (TFile*)vf;

  } else if ( newMode == THistSvc::UPDATE ) {
    // update file

    int r = p_fileMgr->open( Io::ROOT, name(), filename, ( Io::WRITE | Io::CREATE ), vf, "HIST" );

    if ( r != 0 ) {
      error() << "Unable to open ROOT file " << filename << " for appending" << endmsg;
      return StatusCode::FAILURE;
    }

    f = (TFile*)vf;
  }

  m_files[stream] = make_pair( f, newMode );
  m_fileStreams.insert( make_pair( filename, stream ) );

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "Opening TFile \"" << filename << "\"  stream: \"" << stream << "\"  mode: \"" << typ << "\""
            << " comp level: " << cl << endmsg;

  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

TDirectory* THistSvc::changeDir( const THistSvc::THistID& hid ) const
{

  string uid  = hid.id;
  TFile* file = hid.file;
  string stream, fdir, bdir, dir, id;

  if ( file ) {
    file->cd( "/" );
  } else {
    gROOT->cd();
  }

  fdir = uid;
  bdir = dirname( fdir );

  while ( ( dir = dirname( fdir ) ) != "" ) {
    if ( !gDirectory->GetKey( dir.c_str() ) ) {
      gDirectory->mkdir( dir.c_str() );
    }
    gDirectory->cd( dir.c_str() );
  }

  return gDirectory;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

std::string THistSvc::dirname( std::string& dir ) const
{

  string::size_type i = dir.find( "/" );

  if ( i == string::npos ) return {};

  if ( i == 0 ) {
    dir.erase( 0, 1 );
    return dirname( dir );
  }

  string root = dir.substr( 0, i );
  dir.erase( 0, i );

  return root;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

THistSvc::GlobalDirectoryRestore::GlobalDirectoryRestore(THistSvcMutex_t& mut) : m_lock(mut) {
  m_gd = gDirectory;
  m_gf = gFile;
  m_ge = gErrorIgnoreLevel;
}

THistSvc::GlobalDirectoryRestore::~GlobalDirectoryRestore()
{
  gDirectory        = m_gd;
  gFile             = m_gf;
  gErrorIgnoreLevel = m_ge;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void THistSvc::removeDoubleSlash( std::string& id ) const
{

  while ( id.find( "//" ) != std::string::npos ) {
    id.replace( id.find( "//" ), 2, "/" );
  }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void THistSvc::MergeRootFile( TDirectory* target, TDirectory* source )
{

  if ( msgLevel( MSG::DEBUG ) ) debug() << "Target path: " << target->GetPath() << endmsg;
  TString path( (char*)strstr( target->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  source->cd( path );
  TDirectory* current_sourcedir = gDirectory;

  // loop over all keys in this directory
  TList* lkeys = current_sourcedir->GetListOfKeys();
  int nkeys    = lkeys->GetEntries();
  TKey* key;
  for ( int jj = 0; jj < nkeys; jj++ ) {
    key                     = (TKey*)lkeys->At( jj );
    string pathnameinsource = current_sourcedir->GetPath() + string( "/" ) + key->GetName();
    if ( msgLevel( MSG::DEBUG ) ) debug() << "Reading Key:" << pathnameinsource << endmsg;
    // key->Dump();
    // TObject *obj=key->ReadObj();
    TObject* obj = source->Get( pathnameinsource.c_str() );

    if ( obj ) {
      if ( obj->IsA()->InheritsFrom( "TDirectory" ) ) {
        // it's a subdirectory

        if ( msgLevel( MSG::DEBUG ) ) debug() << "Found subdirectory " << obj->GetName() << endmsg;

        // create a new subdir of same name and title in the target file
        target->cd();
        TDirectory* newtargetdir = target->mkdir( obj->GetName(), obj->GetTitle() );

        MergeRootFile( newtargetdir, source );

      } else if ( obj->IsA()->InheritsFrom( "TTree" ) ) {
        if ( msgLevel( MSG::DEBUG ) ) debug() << "Found TTree " << obj->GetName() << endmsg;
        TTree* mytree = dynamic_cast<TTree*>( obj );
        int nentries  = (int)mytree->GetEntries();
        mytree->SetBranchStatus( "*", 1 );

        if ( msgLevel( MSG::DEBUG ) ) debug() << "Dumping TTree " << nentries << " entries" << endmsg;
        // mytree->Print();
        // for (int ij=0; ij<nentries; ij++) {
        // debug() << "Dumping TTree Show( " << ij <<" )"
        //<< endmsg;
        // mytree->Show(ij);
        //}
        target->cd();
        mytree->CloneTree();

        // debug() << "Writing TTree to target file: ( "
        //<< mycopiedtree->Write(key->GetName()) <<" ) bytes written"
        //<< endmsg;

      } else {
        target->cd();
        obj->Write( key->GetName() );
      }
    }

  } // while ( ( TKey *key = (TKey*)nextkey() ) )

  // save modifications to target file
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

bool THistSvc::exists( const std::string& name ) const
{

  TH1* h;
  return getHist_i(name,h,0,true).isSuccess();

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void THistSvc::handle( const Incident& /* inc */ )
{

  if ( signaledStop ) return;

  if ( m_maxFileSize.value() == -1 ) return;

  // convert to bytes.
  Long64_t mfs      = (Long64_t)m_maxFileSize.value() * (Long64_t)1048576;
  Long64_t mfs_warn = mfs * 95 / 100;

  updateFiles();

  map<string, pair<TFile*, Mode>>::const_iterator itr;
  for ( const auto& f : m_files ) {
    TFile* tf = f.second.first;

#ifndef NDEBUG
    if ( msgLevel( MSG::DEBUG ) )
      debug() << "stream: " << f.first << "  name: " << tf->GetName() << "  size: " << tf->GetSize() << endmsg;
#endif

    // Signal job to terminate if output file is too large
    if ( tf->GetSize() > mfs ) {

      signaledStop = true;

      fatal() << "file \"" << tf->GetName() << "\" associated with stream \"" << f.first
              << "\" has exceeded the max file size of " << m_maxFileSize.value() << "MB. Terminating Job." << endmsg;

      IEventProcessor* evt = nullptr;
      if ( service( "ApplicationMgr", evt, true ).isSuccess() ) {
        evt->stopRun();
        evt->release();
      } else {
        abort();
      }
    } else if ( tf->GetSize() > mfs_warn ) {
      warning() << "file \"" << tf->GetName() << "\" associated with stream \"" << f.first
                << "\" is at 95% of its maximum allowable file size of " << m_maxFileSize.value() << "MB" << endmsg;
    }
  }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

/** helper function to recursively copy the layout of a TFile into a new TFile
 */
void THistSvc::copyFileLayout( TDirectory* dst, TDirectory* src )
{

  if ( msgLevel( MSG::DEBUG ) ) debug() << "copyFileLayout() to dst path: " << dst->GetPath() << endmsg;

  // strip out URLs
  TString path( (char*)strstr( dst->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  src->cd( path );
  TDirectory* cur_src_dir = gDirectory;

  // loop over all keys in this directory
  TList* key_list = cur_src_dir->GetListOfKeys();
  int n           = key_list->GetEntries();
  for ( int j = 0; j < n; ++j ) {
    TKey* k                        = (TKey*)key_list->At( j );
    const std::string src_pathname = cur_src_dir->GetPath() + std::string( "/" ) + k->GetName();
    TObject* o                     = src->Get( src_pathname.c_str() );

    if ( o && o->IsA()->InheritsFrom( "TDirectory" ) ) {
      if ( msgLevel( MSG::VERBOSE ) ) verbose() << " subdir [" << o->GetName() << "]..." << endmsg;
      dst->cd();
      TDirectory* dst_dir = dst->mkdir( o->GetName(), o->GetTitle() );
      copyFileLayout( dst_dir, src );
    }
  } // loop over keys
  return;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//
/** @brief callback method to reinitialize the internal state of
 *         the component for I/O purposes (e.g. upon @c fork(2))
 */
StatusCode THistSvc::io_reinit()
{
  bool all_good = true;
  if ( msgLevel( MSG::DEBUG ) ) debug() << "reinitializing I/O..." << endmsg;

  // retrieve the I/O component manager...

  IIoComponentMgr* iomgr = nullptr;

  if ( service( "IoComponentMgr", iomgr, true ).isFailure() ) {
    error() << "could not retrieve I/O component manager !" << endmsg;
    return StatusCode::FAILURE;
  }

  GlobalDirectoryRestore restore(m_svcMut);
  // to hide the expected errors upon closing the files whose
  // file descriptors have been swept under the rug...
  gErrorIgnoreLevel = kFatal;

  for ( auto& ifile : m_files ) {
    TFile* f          = ifile.second.first;
    std::string fname = f->GetName();
    if ( msgLevel( MSG::DEBUG ) )
      debug() << "file [" << fname << "] mode: [" << f->GetOption() << "] r:" << f->GetFileBytesRead()
              << " w:" << f->GetFileBytesWritten() << " cnt:" << f->GetFileCounter() << endmsg;

    if ( ifile.second.second == READ ) {
      if ( msgLevel( MSG::DEBUG ) ) debug() << "  TFile opened in READ mode: not reassigning names" << endmsg;
      continue;
    }

    if ( !iomgr->io_retrieve( this, fname ).isSuccess() ) {
      error() << "could not retrieve new name for [" << fname << "] !!" << endmsg;
      all_good = false;
      continue;
    } else {
      if ( msgLevel( MSG::DEBUG ) ) debug() << "got a new name [" << fname << "]..." << endmsg;
    }
    // create a new TFile
    // TFile *newfile = TFile::Open (fname.c_str(), f->GetOption());

    void* vf;
    Option_t* opts = f->GetOption();
    int r          = p_fileMgr->open( Io::ROOT, name(), fname, Io::WRITE, vf, "HIST" );
    if ( r != 0 ) {
      error() << "unable to open file \"" << fname << "\" for writing" << endmsg;
      return StatusCode::FAILURE;
    }
    TFile* newfile = (TFile*)vf;
    newfile->SetOption( opts );

    if ( ifile.second.second != THistSvc::READ ) {
      copyFileLayout( newfile, f );
      ifile.second.first = newfile;
    }

    // loop over all uids and migrate them to the new file
    // XXX FIXME: this double loop sucks...
    for ( auto& uid : m_uidsX ) {
      THistID& hid = uid.second;
      if ( hid.file != f ) continue;
      TDirectory* olddir = this->changeDir( hid );
      hid.file           = newfile;
      // side-effect: create needed directories...
      TDirectory* newdir = this->changeDir( hid );
      TClass* cl         = hid.obj->IsA();

      // migrate the objects to the new file.
      // thanks to the object model of ROOT, it is super easy.
      if ( cl->InheritsFrom( "TTree" ) ) {
        dynamic_cast<TTree*>( hid.obj )->SetDirectory( newdir );
        dynamic_cast<TTree*>( hid.obj )->Reset();
      } else if ( cl->InheritsFrom( "TH1" ) ) {
        dynamic_cast<TH1*>( hid.obj )->SetDirectory( newdir );
        dynamic_cast<TH1*>( hid.obj )->Reset();
      } else if ( cl->InheritsFrom( "TGraph" ) ) {
        olddir->Remove( hid.obj );
        newdir->Append( hid.obj );
      } else {
        error() << "id: \"" << hid.id << "\" is not a inheriting from a class "
                << "we know how to handle (received [" << cl->GetName() << "], "
                << "expected [TTree, TH1 or TGraph]) !" << endmsg << "attaching to current dir [" << newdir->GetPath()
                << "] "
                << "nonetheless..." << endmsg;
        olddir->Remove( hid.obj );
        newdir->Append( hid.obj );
      }
    }
    f->ReOpen( "READ" );
    p_fileMgr->close( f, name() );
    f = newfile;
  }

  return all_good ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::rootOpenAction( const Io::FileAttr* fa, const std::string& caller )
{

  if ( fa->tech() != Io::ROOT ) {
    // This should never happen
    return StatusCode::SUCCESS;
  }

  if ( fa->desc() != "HIST" ) {
    return StatusCode::SUCCESS;
  }

  p_incSvc->fireIncident( FileIncident( caller, "OpenHistFile", fa->name() ) );

  if ( fa->flags().isRead() ) {
    p_incSvc->fireIncident( FileIncident( caller, "BeginHistFile", fa->name() ) );
  } else if ( fa->flags().isWrite() ) {
    p_incSvc->fireIncident( FileIncident( caller, IncidentType::BeginOutputFile, fa->name() ) );
  } else {
    // for Io::RW
    p_incSvc->fireIncident( FileIncident( caller, IncidentType::BeginOutputFile, fa->name() ) );
  }

  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode THistSvc::rootOpenErrAction( const Io::FileAttr* fa, const std::string& caller )
{

  if ( fa->tech() != Io::ROOT ) {
    // This should never happen
    return StatusCode::SUCCESS;
  }

  if ( fa->desc() != "HIST" ) {
    return StatusCode::SUCCESS;
  }

  if ( fa->flags().isRead() ) {
    p_incSvc->fireIncident( FileIncident( caller, IncidentType::FailInputFile, fa->name() ) );
  } else if ( fa->flags().isWrite() ) {
    p_incSvc->fireIncident( FileIncident( caller, IncidentType::FailOutputFile, fa->name() ) );
  } else {
    // for Io::RW
    p_incSvc->fireIncident( FileIncident( caller, "FailRWFile", fa->name() ) );
  }

  return StatusCode::SUCCESS;
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

size_t
THistSvc::findHistID(const std::string& id, const THistID*& hid, 
                     const size_t& index) const {

  GlobalDirectoryRestore restore(m_svcMut);

  std::string idr(id);
  removeDoubleSlash( idr );

  hid = 0;

  if (idr.find("/") == 0) {
    // fully specified name, starts with "/"
    auto itr = m_uids.find(idr);
    if (itr == m_uids.end()) {
      // no matches found
      return 0;
    } else {
      // one or more matches found (clones).
      if (index >= itr->second->size()) {
        error() << "no index " << index << " found for Hist "
                << idr << endmsg;
        return 0;
      }
      hid = & (itr->second->at(index));
      //      return itr->second->size();
      return 1;
    }

  } else {
    // name not fully specified.
    auto mitr = m_ids.equal_range(idr);

    if (mitr.first == mitr.second) {
      // not found
      return 0;
    } else if ( distance(mitr.first,mitr.second) == 1 ) {
      // one found
      if (index >= mitr.first->second->size()) {
        error() << "no index " << index << " found for Hist "
                << idr << endmsg;
        return 0;
      }
      hid = & (mitr.first->second->at(0));
      return 1;
    } else {
      // multiple matches
      return distance(mitr.first, mitr.second);
    }

  }

}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::regSharedHist(const std::string& id, std::unique_ptr<TH1>& hist, 
                        LockedHandle<TH1>& lh) {

  return regSharedHist_i(id,hist,lh);

}

StatusCode
THistSvc::regSharedHist(const std::string& id, std::unique_ptr<TH2>& hist, 
                        LockedHandle<TH2>& lh) {

  return regSharedHist_i(id,hist,lh);

}

StatusCode
THistSvc::regSharedHist(const std::string& id, std::unique_ptr<TH3>& hist, 
                        LockedHandle<TH3>& lh) {

  return regSharedHist_i(id,hist,lh);

}


//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

StatusCode
THistSvc::getSharedHist(const std::string& name, LockedHandle<TH1>& hist) const {

  GlobalDirectoryRestore restore(m_svcMut);

  const THistID* hid;
  size_t i = findHistID(name, hid);

  //  THistID* hid = const_cast<THistID*>(hid_c);

  if (i == 1) {

    if ( !hid->shared ) {
      error() << "getSharedHist: found Hist with id \"" << name
              << "\", but it's not marked as shared"
              << endmsg;
      return StatusCode::FAILURE;
    }
    TH1* h1 = dynamic_cast< TH1*> ( hid->obj );
    hist = LockedHandle<TH1> (h1, hid->mutex);

    debug() << "getSharedHist: found THistID: " << *hid << endmsg;

    return StatusCode::SUCCESS;
  } else if (i == 0) {
    error() << "no histograms matching id \"" << name << "\" found"
            << endmsg;
    return StatusCode::FAILURE;
  } else {
    info() << "multiple matches for id \"" << name << "\" found [" << i 
           << "], probably from different streams"
           << endmsg;
    return StatusCode::FAILURE;
  }


}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *//

void
THistSvc::dump() const {

  ostringstream ost;

  // list< vector<THistID> >
  ost << "m_hlist:  size: " << m_hlist.size() << "\n";
  for (auto& vh : m_hlist) {
    ost << " - " << vh->at(0) << " :: [" << vh << "] " << vh->size() <<  " {";
    for (auto &e : *vh) {
      TObject *o = e.obj;
      ost << "[" << o << "]";
    }
    ost << "}\n";
  }

  // map uid -> vector<THistID>*
  ost << "\n" << "m_uids: " << m_uids.size() << "\n";
  for (auto& e : m_uids) {
    ost << " - " << e.first << "  [" << e.second << "]" << endl;
  }

  // multimap id -> vector<THistID>*
  ost << "\n" << "m_ids: " << m_ids.size() << "\n";
  for (auto& e : m_ids) {
    ost << " - " << e.first << "  [" << e.second << "]" << endl;
  }

  // map TObject* -> THistID*
  ost << "\n" << "m_tobjs: " << m_tobjs.size() << "\n";
  for (auto &e : m_tobjs) {
    TObject* o = e.first;
    THistID& i = e.second.first->at(e.second.second);
    ost << " - " << o << " -> " << i << endl;
  }


  info() << "dumping THistSvc contents\n" << ost.str() << endmsg;

}
    

void
THistSvc::dumpVHID(const vhid_t* vi) const {
  debug() << "vHID: " << vi << " [" << vi->size() << "]";
  for (auto &e : *vi) {
    debug() << " " << &e;
  }
  debug() << endmsg;
}

