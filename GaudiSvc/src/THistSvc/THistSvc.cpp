/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// system headers
#include <cassert>
#include <cstdio>
#include <sstream>
#include <streambuf>

// boost headers
#include "boost/algorithm/string/case_conv.hpp"

// ROOT headers
#include "TClass.h"
#include "TDirectory.h"
#include "TError.h"
#include "TFile.h"
#include "TGraph.h"
#include "TKey.h"
#include "TROOT.h"

// Gaudi headers
#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IIoComponentMgr.h"
#include "GaudiKernel/ISvcLocator.h"
#include <Gaudi/Property.h>

// local headers
#include "THistSvc.h"

// FIXME - leak sanitizers generate a number of warnings here
//
// Appears to be an issue with the mutex ownsership with LockedHandle ?
// clang-format off
// Direct leak of 40 byte(s) in 1 object(s) allocated from:
//    #0 0x7f09944fcda8 in operator new(unsigned long) /afs/cern.ch/cms/CAF/CMSCOMM/COMM_ECAL/dkonst/GCC/build/contrib/gcc-8.2.0/src/gcc/8.2.0/libsanitizer/lsan/lsan_interceptors.cc:229
//    #1 0x7f0987aa2e6d in StatusCode THistSvc::regHist_i<TH1>(std::unique_ptr<TH1, std::default_delete<TH1> >, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, bool, THistSvc::THistID*&) ../GaudiSvc/src/THistSvc/THistSvc.icc:139
//    #2 0x7f0987aa3375 in LockedHandle<TH1, std::mutex> THistSvc::regShared_i<TH1>(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::unique_ptr<TH1, std::default_delete<TH1> >) ../GaudiSvc/src/THistSvc/THistSvc.icc:264
//    #3 0x7f0987a8f8ef in THistSvc::regShared(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, std::unique_ptr<TH1, std::default_delete<TH1> >, LockedHandle<TH1, std::mutex>&) ../GaudiSvc/src/THistSvc/THistSvc.cpp:430
// clang-format on
//
// These leaks are currently suppressed in Gaudi/job/Gaudi-LSan.supp - remove entry there to reactivate

DECLARE_COMPONENT( THistSvc )

namespace {
  template <typename InputIterator, typename OutputIterator, typename UnaryOperation, typename UnaryPredicate>
  OutputIterator transform_if( InputIterator first, InputIterator last, OutputIterator result, UnaryOperation op,
                               UnaryPredicate pred ) {
    while ( first != last ) {
      if ( pred( *first ) ) *result++ = op( *first );
      ++first;
    }
    return result;
  }

  constexpr struct select1st_t {
    template <typename T, typename S>
    const T& operator()( const std::pair<T, S>& p ) const {
      return p.first;
    }
  } select1st{};
} // namespace

//*************************************************************************//

StatusCode THistSvc::initialize() {
  GlobalDirectoryRestore restore( m_svcMut );

  StatusCode status = Service::initialize();

  if ( status.isFailure() ) {
    error() << "initializing service" << endmsg;
    return status;
  }

  StatusCode st( StatusCode::SUCCESS );

  try {
    setupOutputFile();
  } catch ( GaudiException& err ) {
    error() << "Caught: " << err << endmsg;
    st = StatusCode::FAILURE;
  }

  try {
    setupInputFile();
  } catch ( GaudiException& err ) {
    error() << "Caught: " << err << endmsg;
    st = StatusCode::FAILURE;
  }

  // Protect against multiple instances of TROOT
  if ( !gROOT ) {
    static TROOT root( "root", "ROOT I/O" );
    // gDebug = 99;
  } else {
    if ( msgLevel( MSG::VERBOSE ) ) { verbose() << "ROOT already initialized, debug = " << gDebug << endmsg; }
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
    if ( msgLevel( MSG::DEBUG) ) {
      debug() << "got the FileMgr" << endmsg;
    }
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
    if ( !m_inputfile.value().empty() ) setupInputFile();
    if ( !m_outputfile.value().empty() ) setupOutputFile();
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
        const std::string&                  fname = reg.second.first->GetName();
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

  // Cache a pointer to the TTree dictionary.
  m_ttreeClass = TClass::GetClass( "TTree" );
  if ( ( m_ttreeClass == nullptr ) || ( m_ttreeClass->IsLoaded() == false ) ) {
    error() << "Could not access/load the dictionary for TTree!" << endmsg;
    st = StatusCode::FAILURE;
  }

  if ( st.isFailure() ) { fatal() << "Unable to initialize THistSvc" << endmsg; }

  return st;
}

StatusCode THistSvc::reinitialize() {
  GlobalDirectoryRestore restore( m_svcMut );
  warning() << "reinitialize not implemented" << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode THistSvc::finalize() {
  GlobalDirectoryRestore restore( m_svcMut );

  if ( msgLevel( MSG::DEBUG ) ) {
    dump();
    debug() << "THistSvc::finalize" << endmsg;
  }

#ifndef NDEBUG
  if ( msgLevel( MSG::DEBUG ) ) {
    const std::map<uidMap_t::key_type, uidMap_t::mapped_type> sorted_uids{begin( m_uids ), end( m_uids )};
    for ( const auto& itr : sorted_uids ) {
      THistID& thid = itr.second->at( 0 );
      TObject* tobj = thid.obj;

      std::string dirname( "none" );
      if ( tobj && tobj->IsA()->InheritsFrom( "TTree" ) ) {
        TTree* tree = dynamic_cast<TTree*>( tobj );
        if ( tree->GetDirectory() != 0 ) { dirname = tree->GetDirectory()->GetPath(); }
      } else if ( tobj && tobj->IsA()->InheritsFrom( "TGraph" ) ) {
        if ( !thid.temp ) {
          dirname = thid.file->GetPath();
          std::string id2( thid.id );
          id2.erase( 0, id2.find( "/", 1 ) );
          id2.erase( id2.rfind( "/" ), id2.length() );
          if ( id2.find( "/" ) == 0 ) { id2.erase( 0, 1 ); }
          dirname += id2;
        } else {
          dirname = "/tmp";
        }
      } else if ( tobj && tobj->IsA()->InheritsFrom( "TH1" ) ) {
        TH1* th = dynamic_cast<TH1*>( tobj );
        if ( th == nullptr ) {
          error() << "Couldn't dcast: " << itr.first << endmsg;
        } else {
          if ( th->GetDirectory() != 0 ) { dirname = th->GetDirectory()->GetPath(); }
        }
      } else if ( !tobj ) {
        warning() << itr.first << " has NULL TObject ptr" << endmsg;
      }
      debug() << "finalize: " << thid << endmsg;
    }
  }
#endif

  if ( writeObjectsToFile().isFailure() ) { error() << "problems writing histograms" << endmsg; }

  if ( m_print ) { info() << "Listing contents of ROOT files: " << endmsg; }
  std::vector<TFile*> deleted_files;
  for ( auto& itr : m_files ) {
    if ( std::find( deleted_files.begin(), deleted_files.end(), itr.second.first ) == deleted_files.end() ) {
      deleted_files.push_back( itr.second.first );

#ifndef NDEBUG
      if ( msgLevel( MSG::DEBUG ) ) {
        debug() << "finalizing stream/file " << itr.first << ":" << itr.second.first->GetName() << endmsg;
      }
#endif
    } else {
#ifndef NDEBUG
      if ( msgLevel( MSG::DEBUG ) ) { debug() << "already finalized stream " << itr.first << endmsg; }
#endif
      continue;
    }

    if ( m_print && msgLevel( MSG::INFO ) ) {
      info() << "==> File: " << itr.second.first->GetName() << "  stream: " << itr.first << endmsg;

      itr.second.first->Print( "base" );
    }

    std::string tmpfn = itr.second.first->GetName();

    p_fileMgr->close( itr.second.first, name() );

    IIncidentSvc* pIncidentSvc = nullptr;
    if ( service( "IncidentSvc", pIncidentSvc ).isFailure() ) {
      error() << "Unable to get the IncidentSvc" << endmsg;
      return StatusCode::FAILURE;
    }

    if ( itr.second.second == SHARE ) {
      // Merge File
      void* vfile = nullptr;
      int   returncode =
          p_fileMgr->open( Io::ROOT, name(), m_sharedFiles[itr.first], Io::WRITE | Io::APPEND, vfile, "HIST" );

      if ( returncode ) {
        error() << "unable to open Final Output File: \"" << m_sharedFiles[itr.first] << "\" for merging" << endmsg;
        return StatusCode::FAILURE;
      }

      TFile* outputfile = (TFile*)vfile;
      pIncidentSvc->fireIncident( FileIncident( name(), IncidentType::WroteToOutputFile, m_sharedFiles[itr.first] ) );

      if ( msgLevel( MSG::DEBUG ) ) { debug() << "THistSvc::writeObjectsToFile()::Merging Rootfile " << endmsg; }

      vfile      = nullptr;
      returncode = p_fileMgr->open( Io::ROOT, name(), tmpfn, Io::READ, vfile, "HIST" );

      if ( returncode ) {
        error() << "unable to open temporary file: \"" << tmpfn << endmsg;
        return StatusCode::FAILURE;
      }

      TFile* inputfile = (TFile*)vfile;

      outputfile->SetCompressionLevel( inputfile->GetCompressionLevel() );

      MergeRootFile( outputfile, inputfile );

      outputfile->Write();
      p_fileMgr->close( outputfile, name() );
      p_fileMgr->close( inputfile, name() );

      if ( msgLevel( MSG::DEBUG ) ) { debug() << "Trying to remove temporary file \"" << tmpfn << "\"" << endmsg; }

      std::remove( tmpfn.c_str() );
    }
    delete itr.second.first;
  }

  m_files.clear();
  m_sharedFiles.clear();
  m_fileStreams.clear();
  m_hlist.clear(); // vhid* is deleted in m_tobjs
  m_uids.clear();  // vhid* is deleted in m_tobjs
  m_ids.clear();   // vhid* is deleted in m_tobjs

  for ( auto& obj : m_tobjs ) {
    // TObject*'s are already dealt with through root file i/o
    // only delete vector if this object is index 0
    if ( obj.second.second == 0 ) {
      delete obj.second.first; // delete vhid*
    }
  }
  m_tobjs.clear();

  return Service::finalize();
}

//*************************************************************************//

StatusCode THistSvc::regHist( const std::string& id ) {
  std::unique_ptr<TH1> hist = nullptr;
  return regHist_i( std::move( hist ), id, false );
}

StatusCode THistSvc::regHist( const std::string& id, std::unique_ptr<TH1> hist ) {
  return regHist_i( std::move( hist ), id, false );
}

StatusCode THistSvc::regHist( const std::string& id, std::unique_ptr<TH1> hist, TH1* hist_ptr ) {
  // This is only to support a common use case where the histogram is used after its registration
  if ( hist_ptr != nullptr ) { hist_ptr = hist.get(); }
  return regHist_i( std::move( hist ), id, false );
}

StatusCode THistSvc::regHist( const std::string& id, TH1* hist_ptr ) {
  std::unique_ptr<TH1> hist( hist_ptr );
  return regHist_i( std::move( hist ), id, false );
}

StatusCode THistSvc::getHist( const std::string& id, TH1*& hist, size_t ind ) const {
  hist = getHist_i<TH1>( id, ind );
  if ( hist != nullptr ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::getHist( const std::string& id, TH2*& hist, size_t ind ) const {
  hist = getHist_i<TH2>( id, ind );
  if ( hist != nullptr ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::getHist( const std::string& id, TH3*& hist, size_t ind ) const {
  hist = getHist_i<TH3>( id, ind );
  if ( hist != nullptr ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::regTree( const std::string& id ) {
  std::unique_ptr<TTree> tree = nullptr;
  return regHist_i( std::move( tree ), id, false );
}

StatusCode THistSvc::regTree( const std::string& id, std::unique_ptr<TTree> tree ) {
  StatusCode sc = regHist_i( std::move( tree ), id, false );
  TTree*     tr = nullptr;
  if ( getTree( id, tr ).isSuccess() && sc.isSuccess() ) {
    if ( m_autoSave != 0 ) { tr->SetAutoSave( m_autoSave ); }
    tr->SetAutoFlush( m_autoFlush );
  }
  return sc;
}

StatusCode THistSvc::regTree( const std::string& id, TTree* tree_ptr ) {
  std::unique_ptr<TTree> tree( tree_ptr );
  StatusCode             sc = regHist_i( std::move( tree ), id, false );
  TTree*                 tr = nullptr;
  if ( getTree( id, tr ).isSuccess() && sc.isSuccess() ) {
    if ( m_autoSave != 0 ) { tr->SetAutoSave( m_autoSave ); }
    tr->SetAutoFlush( m_autoFlush );
  }
  return sc;
}

StatusCode THistSvc::getTree( const std::string& id, TTree*& tree ) const {
  tree = getHist_i<TTree>( id );
  if ( tree != nullptr ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::regGraph( const std::string& id ) {
  std::unique_ptr<TGraph> graph = std::make_unique<TGraph>();
  return regHist_i( std::move( graph ), id, false );
}

StatusCode THistSvc::regGraph( const std::string& id, std::unique_ptr<TGraph> graph ) {
  if ( strcmp( graph->GetName(), "Graph" ) == 0 ) {
    std::string            id2( id );
    std::string::size_type i = id2.rfind( "/" );
    if ( i != std::string::npos ) { id2.erase( 0, i + 1 ); }

    info() << "setting name of TGraph id: \"" << id << "\" to \"" << id2 << "\" since it is unset" << endmsg;
    graph->SetName( id2.c_str() );
  }

  return regHist_i( std::move( graph ), id, false );
}

StatusCode THistSvc::regGraph( const std::string& id, TGraph* graph_ptr ) {
  std::unique_ptr<TGraph> graph( graph_ptr );
  if ( strcmp( graph->GetName(), "Graph" ) == 0 ) {
    std::string            id2( id );
    std::string::size_type i = id2.rfind( "/" );
    if ( i != std::string::npos ) { id2.erase( 0, i + 1 ); }

    info() << "setting name of TGraph id: \"" << id << "\" to \"" << id2 << "\" since it is unset" << endmsg;
    graph->SetName( id2.c_str() );
  }

  return regHist_i( std::move( graph ), id, false );
}

StatusCode THistSvc::getGraph( const std::string& id, TGraph*& graph ) const {
  graph = getHist_i<TGraph>( id );
  if ( graph != nullptr ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::regEfficiency( const std::string& id ) {
  std::unique_ptr<TEfficiency> eff = nullptr;
  return regHist_i( std::move( eff ), id, false );
}

StatusCode THistSvc::regEfficiency( const std::string& id, std::unique_ptr<TEfficiency> eff ) {
  return regHist_i( std::move( eff ), id, false );
}

StatusCode THistSvc::regEfficiency( const std::string& id, TEfficiency* eff_ptr ) {
  std::unique_ptr<TEfficiency> eff( eff_ptr );
  return regHist_i( std::move( eff ), id, false );
}

StatusCode THistSvc::getEfficiency( const std::string& id, TEfficiency*& eff ) const {
  eff = getHist_i<TEfficiency>( id );
  if ( eff != nullptr ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::regShared( const std::string& id, std::unique_ptr<TH1> hist, LockedHandle<TH1>& lh ) {
  lh = regShared_i<TH1>( id, std::move( hist ) );
  if ( lh ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::regShared( const std::string& id, std::unique_ptr<TH2> hist, LockedHandle<TH2>& lh ) {
  lh = regShared_i<TH2>( id, std::move( hist ) );
  if ( lh ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::regShared( const std::string& id, std::unique_ptr<TH3> hist, LockedHandle<TH3>& lh ) {
  lh = regShared_i<TH3>( id, std::move( hist ) );
  if ( lh ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::regShared( const std::string& id, std::unique_ptr<TGraph> graph, LockedHandle<TGraph>& lh ) {
  lh = regShared_i<TGraph>( id, std::move( graph ) );
  if ( lh ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::regShared( const std::string& id, std::unique_ptr<TEfficiency> eff,
                                LockedHandle<TEfficiency>& lh ) {
  lh = regShared_i<TEfficiency>( id, std::move( eff ) );
  if ( lh ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::getShared( const std::string& name, LockedHandle<TH1>& lh ) const {
  lh = getShared_i<TH1>( name );
  if ( lh ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::getShared( const std::string& name, LockedHandle<TH2>& lh ) const {
  lh = getShared_i<TH2>( name );
  if ( lh ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::getShared( const std::string& name, LockedHandle<TH3>& lh ) const {
  lh = getShared_i<TH3>( name );
  if ( lh ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::getShared( const std::string& name, LockedHandle<TGraph>& lh ) const {
  lh = getShared_i<TGraph>( name );
  if ( lh ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::getShared( const std::string& name, LockedHandle<TEfficiency>& lh ) const {
  lh = getShared_i<TEfficiency>( name );
  if ( lh ) {
    return StatusCode::SUCCESS;
  } else {
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::deReg( const std::string& id ) {
  auto itr = m_uids.find( id );
  if ( itr == m_uids.end() ) {
    error() << "Problem deregistering id \"" << id << "\": not found in registry" << endmsg;
    return StatusCode::FAILURE;
  }

  vhid_t* vh = itr->second;

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "will deregister " << vh->size() << " elements of id \"" << id << "\"" << endmsg;
  }
  StatusCode sc( StatusCode::SUCCESS );
  // vh will get deleted in deReg once empty, so we cannot query the list size in the loop
  size_t vh_size = vh->size();
  while ( vh_size-- ) {
    if ( deReg( vh->back().obj ).isFailure() ) {
      sc = StatusCode::FAILURE;
      error() << "Problems deRegistering " << vh->size() << " element of id \"" << id << "\"" << endmsg;
      break;
    }
  }

  return sc;
}

StatusCode THistSvc::deReg( TObject* obj ) {
  objMap_t::iterator obj_itr = m_tobjs.find( obj );
  if ( obj_itr != m_tobjs.end() ) {
    vhid_t* vhid = obj_itr->second.first;
    THistID hid  = obj_itr->second.first->at( obj_itr->second.second );

    auto uid_itr = m_uids.find( hid.id );
    if ( uid_itr == m_uids.end() ) {
      error() << "Problems deregistering TObject \"" << obj->GetName() << "\" with id \"" << hid.id
              << "\": not in uidMap" << endmsg;
      return StatusCode::FAILURE;
    }

    if ( vhid->size() == 1 ) {
      // We are the last object, so we have to delete vhid properly
      if ( msgLevel( MSG::DEBUG ) ) {
        debug() << "vhid for " << hid.id << " is empty. deleting" << endmsg;
      }

      std::string root, rem;
      parseString( hid.id, root, rem );

      auto mitr   = m_ids.equal_range( rem );
      auto id_itr = std::find_if( mitr.first, mitr.second,
                                  [&]( idMap_t::const_reference i ) { return i.second->at( 0 ).obj == obj; } );
      if ( id_itr == mitr.second ) {
        error() << "Problems deregistering TObject \"" << obj->GetName() << "\" with id \"" << hid.id
                << "\": not in idMap" << endmsg;
        return StatusCode::FAILURE;
      }

      auto hlist_itr = std::find( m_hlist.begin(), m_hlist.end(), vhid );
      if ( hlist_itr == m_hlist.end() ) {
        error() << "Problems deregistering TObject \"" << obj->GetName() << "\" with id \"" << hid.id
                << "\": not in hlist" << endmsg;
        return StatusCode::FAILURE;
      }

      vhid->erase( vhid->begin() + obj_itr->second.second );
      m_tobjs.erase( obj_itr );

      m_uids.erase( uid_itr );
      m_ids.erase( id_itr );
      m_hlist.erase( hlist_itr );

      delete vhid;

    } else if ( vhid->size() > 1 ) {
      m_tobjs.erase( obj_itr );
      vhid->erase( vhid->begin() + obj_itr->second.second );

      // vector of THistID is still not empty (i.e. other instances with same name registered)
    } else {
      error() << "Deregistration failed unexpectedly. (bug in THistSvc?)" << endmsg;
    }
    return StatusCode::SUCCESS;
  } else {
    error() << "Cannot unregister TObject \"" << obj->GetName() << "\": not known to THistSvc" << endmsg;
    return StatusCode::FAILURE;
  }
}

StatusCode THistSvc::merge( const std::string& name ) {
  uidMap_t::iterator itr = m_uids.find( name );
  if ( itr == m_uids.end() ) {
    error() << "merge: id \"" << name << "\" not found" << endmsg;
    return StatusCode::FAILURE;
  }

  return merge( itr->second );
}

StatusCode THistSvc::merge( TObject* obj ) {
  objMap_t::iterator itr = m_tobjs.find( obj );
  if ( itr != m_tobjs.end() ) {
    return merge( itr->second.first );
  } else {
    error() << "merge: unknown object " << obj << endmsg;
    return StatusCode::FAILURE;
  }
}

bool THistSvc::exists( const std::string& name ) const { return existsHist( name ); }

bool THistSvc::existsHist( const std::string& name ) const { return ( getHist_i<TH1>( name, 0, true ) != nullptr ); }

bool THistSvc::existsEfficiency( const std::string& name ) const {
  return ( getHist_i<TEfficiency>( name, 0, true ) != nullptr );
}

bool THistSvc::existsGraph( const std::string& name ) const {
  return ( getHist_i<TGraph>( name, 0, true ) != nullptr );
}

bool THistSvc::existsTree( const std::string& name ) const { return ( getHist_i<TTree>( name, 0, true ) != nullptr ); }

std::vector<std::string> THistSvc::getHists() const {
  std::vector<std::string> names;
  names.reserve( m_uids.size() );
  transform_if( std::begin( m_uids ), std::end( m_uids ), std::back_inserter( names ), select1st,
                []( uidMap_t::const_reference i ) { return i.second->at( 0 ).obj->IsA()->InheritsFrom( "TH1" ); } );
  return names;
}

std::vector<std::string> THistSvc::getTrees() const {
  std::vector<std::string> names;
  names.reserve( m_uids.size() );
  transform_if( std::begin( m_uids ), std::end( m_uids ), std::back_inserter( names ), select1st,
                []( uidMap_t::const_reference i ) { return i.second->at( 0 ).obj->IsA()->InheritsFrom( "TTree" ); } );
  return names;
}

std::vector<std::string> THistSvc::getGraphs() const {
  std::vector<std::string> names;
  names.reserve( m_uids.size() );
  transform_if( std::begin( m_uids ), std::end( m_uids ), std::back_inserter( names ), select1st,
                []( uidMap_t::const_reference i ) { return i.second->at( 0 ).obj->IsA()->InheritsFrom( "TGraph" ); } );
  return names;
}

std::vector<std::string> THistSvc::getEfficiencies() const {
  std::vector<std::string> names;
  names.reserve( m_uids.size() );
  transform_if(
      std::begin( m_uids ), std::end( m_uids ), std::back_inserter( names ), select1st,
      []( uidMap_t::const_reference i ) { return i.second->at( 0 ).obj->IsA()->InheritsFrom( "TEfficiency" ); } );
  return names;
}

StatusCode THistSvc::getTHists( TDirectory* td, TList& tl, bool rcs ) const {
  GlobalDirectoryRestore restore( m_svcMut );

  gErrorIgnoreLevel = kBreak;

  if ( !td->cd() ) {
    error() << "getTHists: No such TDirectory \"" << td->GetPath() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "getTHists: \"" << td->GetPath() << "\": found " << td->GetListOfKeys()->GetSize() << " keys" << endmsg;
  }

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
        getTHists( tt, tl, rcs ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      }
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode THistSvc::getTHists( const std::string& dir, TList& tl, bool rcs ) const {

  GlobalDirectoryRestore restore( m_svcMut );

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream, rem, r2;
  parseString( dir, stream, rem );

  auto itr = m_files.find( stream );
  if ( itr != m_files.end() ) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << "getTHists: \"" << dir << "\" looks like a stream name."
              << " associated TFile: \"" << itr->second.first->GetName() << "\"" << endmsg;
    }

    if ( gDirectory->cd( r2.c_str() ) ) {
      m_curstream = stream;
      sc          = getTHists( gDirectory, tl, rcs );
      m_curstream = "";
      return sc;
    } else {
      if ( msgLevel( MSG::DEBUG ) ) { debug() << "getTHists: no such TDirectory \"" << r2 << "\"" << endmsg; }
    }

  } else {
    if ( msgLevel( MSG::DEBUG ) ) { debug() << "getTHists: stream \"" << stream << "\" not found" << endmsg; }
  }

  if ( !gDirectory->cd( dir.c_str() ) ) {
    error() << "getTHists: No such TDirectory/stream \"" << dir << "\"" << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    sc = getTHists( gDirectory, tl, rcs );
  }

  return sc;
}

StatusCode THistSvc::getTHists( TDirectory* td, TList& tl, bool rcs, bool reg ) {
  GlobalDirectoryRestore restore( m_svcMut );

  gErrorIgnoreLevel = kBreak;

  if ( !td->cd() ) {
    error() << "getTHists: No such TDirectory \"" << td->GetPath() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "getTHists: \"" << td->GetPath() << "\": found " << td->GetListOfKeys()->GetSize() << " keys" << endmsg;
  }

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
        std::string dir = td->GetPath();
        std::string fil = td->GetFile()->GetName();
        dir.erase( 0, fil.length() + 1 );
        std::string id = "/" + m_curstream;
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
        getTHists( tt, tl, rcs, reg ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      }
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode THistSvc::getTHists( const std::string& dir, TList& tl, bool rcs, bool reg ) {
  GlobalDirectoryRestore restore( m_svcMut );

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream, rem, r2;
  parseString( dir, stream, rem );

  auto itr = m_files.find( stream );
  if ( itr != m_files.end() ) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << "getTHists: \"" << dir << "\" looks like a stream name."
              << " associated TFile: \"" << itr->second.first->GetName() << "\"" << endmsg;
    }

    if ( gDirectory->cd( r2.c_str() ) ) {
      m_curstream = stream;
      sc          = getTHists( gDirectory, tl, rcs, reg );
      m_curstream.clear();
      return sc;
    }
    if ( msgLevel( MSG::DEBUG ) ) { debug() << "getTHists: no such TDirectory \"" << r2 << "\"" << endmsg; }
  } else {
    if ( msgLevel( MSG::DEBUG ) ) { debug() << "getTHists: stream \"" << stream << "\" not found" << endmsg; }
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

StatusCode THistSvc::getTTrees( TDirectory* td, TList& tl, bool rcs ) const {
  GlobalDirectoryRestore restore( m_svcMut );

  gErrorIgnoreLevel = kBreak;

  if ( !td->cd() ) {
    error() << "getTTrees: No such TDirectory \"" << td->GetPath() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "getTHists: \"" << td->GetPath() << "\": found " << td->GetListOfKeys()->GetSize() << " keys" << endmsg;
  }

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
        getTTrees( tt, tl, rcs ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      }
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode THistSvc::getTTrees( const std::string& dir, TList& tl, bool rcs ) const {
  GlobalDirectoryRestore restore( m_svcMut );

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream, rem, r2;
  parseString( dir, stream, rem );

  auto itr = m_files.find( stream );
  if ( itr != m_files.end() ) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << "getTTrees: \"" << dir << "\" looks like a stream name."
              << " associated TFile: \"" << itr->second.first->GetName() << "\"" << endmsg;
    }

    if ( gDirectory->cd( r2.c_str() ) ) { return getTTrees( gDirectory, tl, rcs ); }
    if ( msgLevel( MSG::DEBUG ) ) { debug() << "getTTrees: no such TDirectory \"" << r2 << "\"" << endmsg; }
  } else {
    if ( msgLevel( MSG::DEBUG ) ) { debug() << "getTTrees: stream \"" << stream << "\" not found" << endmsg; }
  }

  if ( !gDirectory->cd( dir.c_str() ) ) {
    error() << "getTTrees: No such TDirectory/stream \"" << dir << "\"" << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    sc = getTTrees( gDirectory, tl, rcs );
  }
  return sc;
}

StatusCode THistSvc::getTTrees( TDirectory* td, TList& tl, bool rcs, bool reg ) {
  GlobalDirectoryRestore restore( m_svcMut );

  gErrorIgnoreLevel = kBreak;

  if ( !td->cd() ) {
    error() << "getTTrees: No such TDirectory \"" << td->GetPath() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "getTHists: \"" << td->GetPath() << "\": found " << td->GetListOfKeys()->GetSize() << " keys" << endmsg;
  }

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
        std::string dir = td->GetPath();
        std::string fil = td->GetFile()->GetName();
        dir.erase( 0, fil.length() + 1 );
        std::string id = "/" + m_curstream;
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
        getTTrees( tt, tl, rcs, reg ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      }
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode THistSvc::getTTrees( const std::string& dir, TList& tl, bool rcs, bool reg ) {
  GlobalDirectoryRestore restore( m_svcMut );

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream, rem, r2;
  parseString( dir, stream, rem );

  auto itr = m_files.find( stream );
  if ( itr != m_files.end() ) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << "getTTrees: \"" << dir << "\" looks like a stream name."
              << " associated TFile: \"" << itr->second.first->GetName() << "\"" << endmsg;
    }

    if ( gDirectory->cd( r2.c_str() ) ) {
      return getTTrees( gDirectory, tl, rcs, reg );
    } else {
      if ( msgLevel( MSG::DEBUG ) ) { debug() << "getTTrees: no such TDirectory \"" << r2 << "\"" << endmsg; }
    }
  } else {
    if ( msgLevel( MSG::DEBUG ) ) { debug() << "getTTrees: stream \"" << stream << "\" not found" << endmsg; }
  }

  if ( !gDirectory->cd( dir.c_str() ) ) {
    error() << "getTTrees: No such TDirectory/stream \"" << dir << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  return getTTrees( gDirectory, tl, rcs, reg );
}

StatusCode THistSvc::getTEfficiencies( TDirectory* td, TList& tl, bool rcs ) const {
  GlobalDirectoryRestore restore( m_svcMut );

  gErrorIgnoreLevel = kBreak;

  if ( !td->cd() ) {
    error() << "getTEfficiencies: No such TDirectory \"" << td->GetPath() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "getTEfficiencies: \"" << td->GetPath() << "\": found " << td->GetListOfKeys()->GetSize() << " keys"
            << endmsg;
  }

  TIter nextkey( td->GetListOfKeys() );
  while ( TKey* key = (TKey*)nextkey() ) {
    auto& log = debug();
    if ( msgLevel( MSG::DEBUG ) ) log << "  key: " << key->GetName();
    TObject* obj = key->ReadObj();
    if ( obj != 0 && obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " (" << obj->IsA()->GetName() << ")";
    } else if ( obj != 0 && obj->IsA()->InheritsFrom( "TEfficiency" ) ) {
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
        getTHists( tt, tl, rcs ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      }
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode THistSvc::getTEfficiencies( const std::string& dir, TList& tl, bool rcs ) const {

  GlobalDirectoryRestore restore( m_svcMut );

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream, rem, r2;
  parseString( dir, stream, rem );

  auto itr = m_files.find( stream );
  if ( itr != m_files.end() ) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << "getTEfficiencies: \"" << dir << "\" looks like a stream name."
              << " associated TFile: \"" << itr->second.first->GetName() << "\"" << endmsg;
    }

    if ( gDirectory->cd( r2.c_str() ) ) {
      m_curstream = stream;
      sc          = getTEfficiencies( gDirectory, tl, rcs );
      m_curstream = "";
      return sc;
    } else {
      if ( msgLevel( MSG::DEBUG ) ) { debug() << "getTEfficiencies: no such TDirectory \"" << r2 << "\"" << endmsg; }
    }

  } else {
    if ( msgLevel( MSG::DEBUG ) ) { debug() << "getTEfficiencies: stream \"" << stream << "\" not found" << endmsg; }
  }

  if ( !gDirectory->cd( dir.c_str() ) ) {
    error() << "getTEfficiencies: No such TDirectory/stream \"" << dir << "\"" << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    sc = getTHists( gDirectory, tl, rcs );
  }

  return sc;
}

StatusCode THistSvc::getTEfficiencies( TDirectory* td, TList& tl, bool rcs, bool reg ) {
  GlobalDirectoryRestore restore( m_svcMut );

  gErrorIgnoreLevel = kBreak;

  if ( !td->cd() ) {
    error() << "getTEfficiencies: No such TDirectory \"" << td->GetPath() << "\"" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "getTEfficiencies: \"" << td->GetPath() << "\": found " << td->GetListOfKeys()->GetSize() << " keys"
            << endmsg;
  }

  TIter nextkey( td->GetListOfKeys() );
  while ( TKey* key = (TKey*)nextkey() ) {
    auto& log = debug();
    if ( msgLevel( MSG::DEBUG ) ) log << "  key: " << key->GetName();
    TObject* obj = key->ReadObj();
    if ( obj && obj->IsA()->InheritsFrom( "TDirectory" ) ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " (" << obj->IsA()->GetName() << ")";
    } else if ( obj && obj->IsA()->InheritsFrom( "TEfficiency" ) ) {
      if ( msgLevel( MSG::DEBUG ) ) log << " (" << obj->IsA()->GetName() << ")";
      tl.Add( obj );
      if ( reg && m_curstream != "" ) {
        std::string dir = td->GetPath();
        std::string fil = td->GetFile()->GetName();
        dir.erase( 0, fil.length() + 1 );
        std::string id = "/" + m_curstream;
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
        getTEfficiencies( tt, tl, rcs, reg ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
      }
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode THistSvc::getTEfficiencies( const std::string& dir, TList& tl, bool rcs, bool reg ) {
  GlobalDirectoryRestore restore( m_svcMut );

  gErrorIgnoreLevel = kBreak;

  StatusCode sc;

  std::string stream, rem, r2;
  parseString( dir, stream, rem );

  auto itr = m_files.find( stream );
  if ( itr != m_files.end() ) {
    r2 = itr->second.first->GetName();
    r2 += ":/";
    r2 += rem;

    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << "getTEfficiencies: \"" << dir << "\" looks like a stream name."
              << " associated TFile: \"" << itr->second.first->GetName() << "\"" << endmsg;
    }

    if ( gDirectory->cd( r2.c_str() ) ) {
      m_curstream = stream;
      sc          = getTEfficiencies( gDirectory, tl, rcs, reg );
      m_curstream.clear();
      return sc;
    }
    if ( msgLevel( MSG::DEBUG ) ) { debug() << "getTEfficiencies: no such TDirectory \"" << r2 << "\"" << endmsg; }
  } else {
    if ( msgLevel( MSG::DEBUG ) ) { debug() << "getTEfficiencies: stream \"" << stream << "\" not found" << endmsg; }
  }

  if ( !gDirectory->cd( dir.c_str() ) ) {
    error() << "getTEfficiencies: No such TDirectory/stream \"" << dir << "\"" << endmsg;
    sc = StatusCode::FAILURE;
  } else {
    if ( reg ) {
      warning() << "Unable to register histograms automatically "
                << "without a valid stream name" << endmsg;
      reg = false;
    }
    sc = getTEfficiencies( gDirectory, tl, rcs, reg );
  }

  return sc;
}

//*************************************************************************//

void THistSvc::handle( const Incident& /* inc */ ) {
  if ( m_signaledStop ) return;

  if ( m_maxFileSize.value() == -1 ) return;

  // convert to bytes.
  Long64_t mfs      = (Long64_t)m_maxFileSize.value() * (Long64_t)1048576;
  Long64_t mfs_warn = mfs * 95 / 100;

  updateFiles();

  for ( const auto& f : m_files ) {
    TFile* tf = f.second.first;

#ifndef NDEBUG
    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << "stream: " << f.first << "  name: " << tf->GetName() << "  size: " << tf->GetSize() << endmsg;
    }
#endif

    // Signal job to terminate if output file is too large
    if ( tf->GetSize() > mfs ) {

      m_signaledStop = true;

      fatal() << "file \"" << tf->GetName() << "\" associated with stream \"" << f.first
              << "\" has exceeded the max file size of " << m_maxFileSize.value() << "MB. Terminating Job." << endmsg;

      IEventProcessor* evt = nullptr;
      if ( service( "ApplicationMgr", evt, true ).isSuccess() ) {
        evt->stopRun().ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
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

/** @brief callback method to reinitialize the internal state of
 *         the component for I/O purposes (e.g. upon @c fork(2))
 */
StatusCode THistSvc::io_reinit() {
  bool all_good = true;
  if ( msgLevel( MSG::DEBUG ) ) { debug() << "reinitializing I/O..." << endmsg; }

  // retrieve the I/O component manager...

  IIoComponentMgr* iomgr = nullptr;

  if ( service( "IoComponentMgr", iomgr, true ).isFailure() ) {
    error() << "could not retrieve I/O component manager !" << endmsg;
    return StatusCode::FAILURE;
  }

  GlobalDirectoryRestore restore( m_svcMut );
  // to hide the expected errors upon closing the files whose
  // file descriptors have been swept under the rug...
  gErrorIgnoreLevel = kFatal;

  for ( auto& ifile : m_files ) {
    TFile*      f     = ifile.second.first;
    std::string fname = f->GetName();
    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << "file [" << fname << "] mode: [" << f->GetOption() << "] r:" << f->GetFileBytesRead()
              << " w:" << f->GetFileBytesWritten() << " cnt:" << f->GetFileCounter() << endmsg;
    }

    if ( ifile.second.second == READ ) {
      if ( msgLevel( MSG::DEBUG ) ) { debug() << "  TFile opened in READ mode: not reassigning names" << endmsg; }
      continue;
    }

    if ( !iomgr->io_retrieve( this, fname ).isSuccess() ) {
      error() << "could not retrieve new name for [" << fname << "] !!" << endmsg;
      all_good = false;
      continue;
    } else if ( fname.empty() ) {
      if ( msgLevel( MSG::DEBUG ) ) { debug() << "empty new name for [" << fname << "], skipping..." << endmsg; }
      continue;
    } else {
      if ( msgLevel( MSG::DEBUG ) ) { debug() << "got a new name [" << fname << "]..." << endmsg; }
    }

    void*     vf   = nullptr;
    Option_t* opts = f->GetOption();
    int       r    = p_fileMgr->open( Io::ROOT, name(), fname, Io::WRITE, vf, "HIST" );
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
    for ( auto& uid : m_uids ) {
      for ( auto& hid : *uid.second ) {
        if ( hid.file != f ) continue;
        TDirectory* olddir = this->changeDir( hid );
        hid.file           = newfile;
        // side-effect: create needed directories...
        TDirectory* newdir = this->changeDir( hid );
        TClass*     cl     = hid.obj->IsA();

        // migrate the objects to the new file.
        // thanks to the object model of ROOT, it is super easy.
        if ( cl->InheritsFrom( "TTree" ) ) {
          dynamic_cast<TTree&>( *hid.obj ).SetDirectory( newdir );
          dynamic_cast<TTree&>( *hid.obj ).Reset();
        } else if ( cl->InheritsFrom( "TH1" ) ) {
          dynamic_cast<TH1&>( *hid.obj ).SetDirectory( newdir );
          dynamic_cast<TH1&>( *hid.obj ).Reset();
        } else if ( cl->InheritsFrom( "TEfficiency" ) ) {
          dynamic_cast<TEfficiency&>( *hid.obj ).SetDirectory( newdir );
        } else if ( cl->InheritsFrom( "TGraph" ) ) {
          olddir->Remove( hid.obj );
          newdir->Append( hid.obj );
        } else {
          error() << "id: \"" << hid.id << "\" is not a inheriting from a class "
                  << "we know how to handle (received [" << cl->GetName() << "], "
                  << "expected [TTree, TH1, TGraph or TEfficiency]) !" << endmsg << "attaching to current dir ["
                  << newdir->GetPath() << "] "
                  << "nonetheless..." << endmsg;
          olddir->Remove( hid.obj );
          newdir->Append( hid.obj );
        }
      }
    }
    f->ReOpen( "READ" );
    p_fileMgr->close( f, name() );
    f = newfile;
  }

  return all_good ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

//*************************************************************************//

THistSvc::GlobalDirectoryRestore::GlobalDirectoryRestore( THistSvcMutex_t& mut ) : m_lock( mut ) {
  m_gDirectory        = gDirectory;
  m_gFile             = gFile;
  m_gErrorIgnoreLevel = gErrorIgnoreLevel;
}

THistSvc::GlobalDirectoryRestore::~GlobalDirectoryRestore() {
  gDirectory        = m_gDirectory;
  gFile             = m_gFile;
  gErrorIgnoreLevel = m_gErrorIgnoreLevel;
}

//*************************************************************************//

template <typename T>
T* THistSvc::readHist( const std::string& id ) const {
  return dynamic_cast<T*>( readHist_i<T>( id ) );
}

TTree* THistSvc::readTree( const std::string& id ) const { return dynamic_cast<TTree*>( readHist_i<TTree>( id ) ); }

void THistSvc::updateFiles() {
  // If TTrees grow beyond TTree::fgMaxTreeSize, a new file is
  // automatically created by root, and the old one closed. We
  // need to migrate all the UIDs over to show the correct file
  // pointer. This is ugly.

  if ( msgLevel( MSG::DEBUG ) ) debug() << "updateFiles()" << endmsg;

  for ( auto uitr = m_uids.begin(); uitr != m_uids.end(); ++uitr ) {
    for ( auto& hid : *( uitr->second ) ) {
#ifndef NDEBUG
      if ( msgLevel( MSG::VERBOSE ) )
        verbose() << " update: " << uitr->first << " " << hid.id << " " << hid.mode << endmsg;
#endif
      TObject* to      = hid.obj;
      TFile*   oldFile = hid.file;
      // A little sanity check.
      assert( m_ttreeClass != nullptr );
      if ( !to ) {
        warning() << uitr->first << ": TObject == 0" << endmsg;
      } else if ( hid.temp || hid.mode == READ ) {
// do nothing - no need to check how big the file is since we
// are just reading it.
#ifndef NDEBUG
        if ( msgLevel( MSG::VERBOSE ) ) verbose() << "     skipping" << endmsg;
#endif
      } else if ( to->IsA()->InheritsFrom( m_ttreeClass ) ) {
        TTree* tr      = dynamic_cast<TTree*>( to );
        TFile* newFile = tr->GetCurrentFile();

        if ( oldFile != newFile ) {
          std::string newFileName = newFile->GetName();
          std::string oldFileName, streamName, rem;
          TFile*      dummy = nullptr;
          findStream( hid.id, streamName, rem, dummy );

          for ( auto& itr : m_files ) {
            if ( itr.second.first == oldFile ) { itr.second.first = newFile; }
          }

          for ( auto uitr2 = uitr; uitr2 != m_uids.end(); ++uitr2 ) {
            for ( auto& hid2 : *( uitr2->second ) ) {
              if ( hid2.file == oldFile ) { hid2.file = newFile; }
            }
          }

          auto sitr = std::find_if( std::begin( m_fileStreams ), std::end( m_fileStreams ),
                                    [&]( streamMap::const_reference s ) { return s.second == streamName; } );
          if ( sitr != std::end( m_fileStreams ) ) oldFileName = sitr->first;

#ifndef NDEBUG
          if ( msgLevel( MSG::DEBUG ) ) {
            debug() << "migrating uid: " << hid.id << "   stream: " << streamName << "   oldFile: " << oldFileName
                    << "   newFile: " << newFileName << endmsg;
          }
#endif

          if ( !oldFileName.empty() ) {
            auto i = m_fileStreams.lower_bound( oldFileName );
            while ( i != std::end( m_fileStreams ) && i->first == oldFileName ) {
#ifndef NDEBUG
              if ( msgLevel( MSG::DEBUG ) ) {
                debug() << "changing filename \"" << i->first << "\" to \"" << newFileName << "\" for stream \""
                        << i->second << "\"" << endmsg;
              }
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
}

StatusCode THistSvc::writeObjectsToFile() {
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
    debug() << "THistSvc::writeObjectsToFile()::List of Files connected in ROOT " << endmsg;
    TSeqCollection* filelist = gROOT->GetListOfFiles();
    for ( int ii = 0; ii < filelist->GetEntries(); ii++ ) {
      debug() << "THistSvc::writeObjectsToFile()::List of Files connected in ROOT: \"" << filelist->At( ii )->GetName()
              << "\"" << endmsg;
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode THistSvc::connect( const std::string& ident ) {
  auto                                        loc    = ident.find( " " );
  std::string                                 stream = ident.substr( 0, loc );
  char                                        typ( 0 );
  typedef std::pair<std::string, std::string> Prop;
  std::vector<Prop>                           props;
  std::string                                 filename, db_typ( "ROOT" );
  int                                         cl( 1 );

  if ( loc != std::string::npos ) {
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

  const auto newMode = charToMode( typ );
  if ( newMode == THistSvc::INVALID ) {
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
      m_files[stream] = std::make_pair( f2, newMode );
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

  void*  vf = nullptr;
  TFile* f  = nullptr;

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
    // For SHARE files, all data will be stored in a temp file and will be
    // merged into the target file in writeObjectsToFile() when finalize(),
    // this help to solve some confliction. e.g. with storegate
    static int  ishared      = 0;
    std::string realfilename = filename;
    filename                 = "tmp_THistSvc_" + std::to_string( ishared++ ) + ".root";

    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << "Creating temp file \"" << filename << "\" and realfilename=" << realfilename << endmsg;
    }
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

  m_files[stream] = std::make_pair( f, newMode );
  m_fileStreams.insert( std::make_pair( filename, stream ) );

  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "Opening TFile \"" << filename << "\"  stream: \"" << stream << "\"  mode: \"" << typ << "\""
            << " comp level: " << cl << endmsg;
  }

  return StatusCode::SUCCESS;
}

TDirectory* THistSvc::changeDir( const THistSvc::THistID& hid ) const {
  std::string uid  = hid.id;
  TFile*      file = hid.file;
  std::string stream, fdir, bdir, dir, id;

  if ( file ) {
    file->cd( "/" );
  } else {
    gROOT->cd();
  }

  fdir = uid;
  bdir = stripDirectoryName( fdir );

  while ( ( dir = stripDirectoryName( fdir ) ) != "" ) {
    if ( !gDirectory->GetKey( dir.c_str() ) ) { gDirectory->mkdir( dir.c_str() ); }
    gDirectory->cd( dir.c_str() );
  }

  return gDirectory;
}

std::string THistSvc::stripDirectoryName( std::string& dir ) const {
  std::string::size_type i = dir.find( "/" );

  if ( i == std::string::npos ) return {};

  if ( i == 0 ) {
    dir.erase( 0, 1 );
    return stripDirectoryName( dir );
  }

  std::string root = dir.substr( 0, i );
  dir.erase( 0, i );

  return root;
}

void THistSvc::removeDoubleSlash( std::string& id ) const {
  while ( id.find( "//" ) != std::string::npos ) { id.replace( id.find( "//" ), 2, "/" ); }
}

void THistSvc::MergeRootFile( TDirectory* target, TDirectory* source ) {
  if ( msgLevel( MSG::DEBUG ) ) { debug() << "Target path: " << target->GetPath() << endmsg; }
  TString path( (char*)strstr( target->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  source->cd( path );
  TDirectory* current_sourcedir = gDirectory;

  // loop over all keys in this directory
  TList* lkeys = current_sourcedir->GetListOfKeys();
  int    nkeys = lkeys->GetEntries();
  TKey*  key   = nullptr;
  for ( int jj = 0; jj < nkeys; jj++ ) {
    key                          = (TKey*)lkeys->At( jj );
    std::string pathnameinsource = current_sourcedir->GetPath() + std::string( "/" ) + key->GetName();
    if ( msgLevel( MSG::DEBUG ) ) { debug() << "Reading Key:" << pathnameinsource << endmsg; }
    TObject* obj = source->Get( pathnameinsource.c_str() );

    if ( obj ) {
      if ( obj->IsA()->InheritsFrom( "TDirectory" ) ) {
        // it's a subdirectory
        if ( msgLevel( MSG::DEBUG ) ) { debug() << "Found subdirectory " << obj->GetName() << endmsg; }

        // create a new subdir of same name and title in the target file
        target->cd();
        TDirectory* newtargetdir = target->mkdir( obj->GetName(), obj->GetTitle() );

        MergeRootFile( newtargetdir, source );

      } else if ( obj->IsA()->InheritsFrom( "TTree" ) ) {
        if ( msgLevel( MSG::DEBUG ) ) { debug() << "Found TTree " << obj->GetName() << endmsg; }
        TTree* mytree   = dynamic_cast<TTree*>( obj );
        int    nentries = (int)mytree->GetEntries();
        mytree->SetBranchStatus( "*", 1 );

        if ( msgLevel( MSG::DEBUG ) ) { debug() << "Dumping TTree " << nentries << " entries" << endmsg; }
        target->cd();
        mytree->CloneTree();

      } else {
        target->cd();
        obj->Write( key->GetName() );
      }
    }
  }
}

bool THistSvc::findStream( const std::string& id, std::string& stream, std::string& rem, TFile*& file ) const {
  auto pos = id.find( "/" );

  if ( pos == std::string::npos ) {
    // no "/" in id
    stream = "temp";
    rem    = id;
  } else if ( pos != 0 ) {
    // id does not start with "/"
    stream = "temp";
    rem    = id;
  } else {
    // id starts with "/"

    auto pos2 = id.find( "/", pos + 1 );

    if ( pos2 == std::string::npos ) {
      // need at least 2 "/" in format "/STREAM/name" or "/STREAM/dir/name"
      error() << "badly formed Hist/Tree id: \"" << id << "\"" << endmsg;
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
  if ( !file ) { warning() << "no stream \"" << stream << "\" associated with id: \"" << id << "\"" << endmsg; }

  return true;
}

void THistSvc::parseString( const std::string& id, std::string& root, std::string& rem ) const {
  auto pos = id.find( "/" );

  if ( pos == std::string::npos ) {
    root.clear();
    rem = id;
  } else if ( pos == 0 ) {
    parseString( id.substr( 1 ), root, rem );
  } else {
    root = id.substr( 0, pos );
    rem  = id.substr( pos + 1 );
  }
}

void THistSvc::setupInputFile() {
  if ( FSMState() < Gaudi::StateMachine::CONFIGURED || !m_okToConnect ) {
    debug() << "Delaying connection of Input Files until Initialize"
            << ". now in " << FSMState() << endmsg;

    m_delayConnect = true;
  } else {
    debug() << "Now connecting of Input Files" << endmsg;

    StatusCode sc = StatusCode::SUCCESS;

    for ( const auto& itr : m_inputfile.value() ) {
      if ( m_alreadyConnectedInFiles.end() != m_alreadyConnectedInFiles.find( itr ) ) { continue; }
      if ( connect( itr ).isFailure() ) {
        sc = StatusCode::FAILURE;
      } else {
        m_alreadyConnectedInFiles.insert( itr );
      }
    }

    if ( !sc.isSuccess() ) { throw GaudiException( "Problem connecting inputfile !!", name(), StatusCode::FAILURE ); }
  }
}

void THistSvc::setupOutputFile() {
  if ( FSMState() < Gaudi::StateMachine::CONFIGURED || !m_okToConnect ) {
    debug() << "Delaying connection of Output Files until Initialize"
            << ". now in " << FSMState() << endmsg;
    m_delayConnect = true;
  } else {
    StatusCode sc = StatusCode::SUCCESS;
    for ( const auto& itr : m_outputfile.value() ) {
      if ( m_alreadyConnectedOutFiles.end() != m_alreadyConnectedOutFiles.find( itr ) ) { continue; }
      if ( connect( itr ).isFailure() ) {
        sc = StatusCode::FAILURE;
      } else {
        m_alreadyConnectedOutFiles.insert( itr );
      }
    }

    if ( !sc.isSuccess() ) { throw GaudiException( "Problem connecting outputfile !!", name(), StatusCode::FAILURE ); }
  }
}

void THistSvc::copyFileLayout( TDirectory* destination, TDirectory* source ) {
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "copyFileLayout() to destination path: " << destination->GetPath() << endmsg;
  }

  // strip out URLs
  TString path( (char*)strstr( destination->GetPath(), ":" ) );
  path.Remove( 0, 2 );

  source->cd( path );
  TDirectory* current_source_dir = gDirectory;

  // loop over all keys in this directory
  TList* key_list = current_source_dir->GetListOfKeys();
  int    n        = key_list->GetEntries();
  for ( int j = 0; j < n; ++j ) {
    TKey*             k               = (TKey*)key_list->At( j );
    const std::string source_pathname = current_source_dir->GetPath() + std::string( "/" ) + k->GetName();
    TObject*          o               = source->Get( source_pathname.c_str() );

    if ( o && o->IsA()->InheritsFrom( "TDirectory" ) ) {
      if ( msgLevel( MSG::VERBOSE ) ) { verbose() << " subdir [" << o->GetName() << "]..." << endmsg; }
      destination->cd();
      // Create directory if it does not exist yet
      TDirectory* destination_dir = destination->mkdir( o->GetName(), o->GetTitle() );
      if ( destination_dir == nullptr ) destination_dir = destination->GetDirectory( o->GetName() );
      copyFileLayout( destination_dir, source );
    }
  } // loop over keys
  return;
}

size_t THistSvc::findHistID( const std::string& id, const THistID*& hid, const size_t& index ) const {
  GlobalDirectoryRestore restore( m_svcMut );

  std::string idr( id );
  removeDoubleSlash( idr );

  hid = 0;

  if ( idr.find( "/" ) == 0 ) {
    // fully specified name, starts with "/"
    auto itr = m_uids.find( idr );
    if ( itr == m_uids.end() ) {
      // no matches found
      return 0;
    } else {
      // one or more matches found (clones).
      if ( index >= itr->second->size() ) {
        error() << "no index " << index << " found for Hist " << idr << endmsg;
        return 0;
      }
      hid = &( itr->second->at( index ) );
      return 1;
    }
  } else {
    // name not fully specified.
    auto mitr = m_ids.equal_range( idr );
    if ( mitr.first == mitr.second ) {
      // not found
      return 0;
    } else if ( distance( mitr.first, mitr.second ) == 1 ) {
      // one found
      if ( index >= mitr.first->second->size() ) {
        error() << "no index " << index << " found for Hist " << idr << endmsg;
        return 0;
      }
      hid = &( mitr.first->second->at( 0 ) );
      return 1;
    } else {
      // multiple matches
      hid = &( mitr.first->second->at( 0 ) );
      return distance( mitr.first, mitr.second );
    }
  }
}

void THistSvc::dump() const {
  std::ostringstream ost;

  // list< vector<THistID> >
  ost << "m_hlist:  size: " << m_hlist.size() << "\n";
  for ( auto& vh : m_hlist ) {
    ost << " - " << vh->at( 0 ) << " :: [" << vh << "] " << vh->size() << " {";
    for ( auto& e : *vh ) {
      TObject* o = e.obj;
      ost << "[" << o << "]";
    }
    ost << "}\n";
  }

  // map uid -> vector<THistID>*
  ost << "\n"
      << "m_uids: " << m_uids.size() << "\n";
  for ( auto& e : m_uids ) { ost << " - " << e.first << "  [" << e.second << "]" << std::endl; }

  // multimap id -> vector<THistID>*
  ost << "\n"
      << "m_ids: " << m_ids.size() << "\n";
  for ( auto& e : m_ids ) { ost << " - " << e.first << "  [" << e.second << "]" << std::endl; }

  // map TObject* -> THistID*
  ost << "\n"
      << "m_tobjs: " << m_tobjs.size() << "\n";
  for ( auto& e : m_tobjs ) {
    TObject* o = e.first;
    THistID& i = e.second.first->at( e.second.second );
    ost << " - " << o << " -> " << i << std::endl;
  }

  debug() << "dumping THistSvc contents\n" << ost.str() << endmsg;
}

StatusCode THistSvc::merge( const THistID& hid ) { return merge( hid.id ); }

StatusCode THistSvc::merge( vhid_t* vh ) {
  const std::string& name = vh->at( 0 ).id;
  if ( vh->size() == 1 ) {
    debug() << "merge: id: \"" << name << "\" is size 1. nothing to do" << endmsg;
    return StatusCode::SUCCESS;
  }

  if ( !vh->at( 0 ).obj->IsA()->InheritsFrom( "TH1" ) ) {
    error() << "merge: id \"" << name << "\" is not a THn. Cannot merge" << endmsg;
    return StatusCode::FAILURE;
  }

  TList* l = new TList();
  for ( size_t i = 1; i < vh->size(); ++i ) {
    debug() << "merge: id: \"" << name << "\" (" << vh->at( i ).obj << ") adding index " << i << endmsg;
    l->Add( vh->at( i ).obj );
  }

  TH1* t0 = dynamic_cast<TH1*>( vh->at( 0 ).obj );
  if ( t0 == 0 ) {
    error() << "merge: could not dcast " << name << "(" << t0 << ") index " << 0 << " to TH1" << endmsg;
    return StatusCode::FAILURE;
  }

  Long64_t n = t0->Merge( l );

  debug() << "merge: id: \"" << name << "\" merged " << n << " entries" << endmsg;

  for ( size_t i = 1; i < vh->size(); ++i ) {
    TH1* th = dynamic_cast<TH1*>( vh->at( i ).obj );
    if ( th != 0 ) {
      debug() << "clearing index " << i << "(" << th << ")" << endmsg;
      th->SetDirectory( nullptr );
      th->Reset();
    } else {
      error() << "merge: could not dcast " << name << " index " << i << " to TH1" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode THistSvc::rootOpenAction( const Io::FileAttr* fa, const std::string& caller ) {
  if ( fa->tech() != Io::ROOT ) {
    // This should never happen
    return StatusCode::SUCCESS;
  }

  if ( fa->desc() != "HIST" ) { return StatusCode::SUCCESS; }

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

StatusCode THistSvc::rootOpenErrAction( const Io::FileAttr* fa, const std::string& caller ) {
  if ( fa->tech() != Io::ROOT ) {
    // This should never happen
    return StatusCode::SUCCESS;
  }

  if ( fa->desc() != "HIST" ) { return StatusCode::SUCCESS; }

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
