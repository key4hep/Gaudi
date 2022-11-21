/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
//====================================================================
//        RootDataConnection.cpp
//--------------------------------------------------------------------
//
//        Author     : M.Frank
//====================================================================

// Framework include files
#include "RootCnv/RootDataConnection.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/strcasecmp.h"
#include "RootUtils.h"
// ROOT include files
#include "TBranch.h"
#include "TClass.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TMemFile.h"
#include "TROOT.h"
#include "TTree.h"
#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 33, 0 )
#  include "Compression.h"
static int s_compressionLevel = ROOT::CompressionSettings( ROOT::kLZMA, 4 );
#else
static int s_compressionLevel = 1;
#endif

// C/C++ include files
#include <limits>
#include <numeric>
#include <stdexcept>

using namespace Gaudi;
using namespace std;
typedef const string& CSTR;

static const string s_empty;
static const string s_local = "<localDB>";

#ifdef __POOL_COMPATIBILITY
#  include "PoolTool.h"
#endif
#include "RootTool.h"

namespace {
  std::array<char, 256> init_table() {
    std::array<char, 256> table;
    std::iota( std::begin( table ), std::end( table ), 0 );
    return table;
  }

  struct RootDataConnectionCategory : StatusCode::Category {
    const char* name() const override { return "RootDataConnection"; }

    bool isRecoverable( StatusCode::code_t ) const override { return false; }

    std::string message( StatusCode::code_t code ) const override {
      switch ( static_cast<RootDataConnection::Status>( code ) ) {
      case RootDataConnection::Status::ROOT_READ_ERROR:
        return "ROOT_READ_ERROR";
      case RootDataConnection::Status::ROOT_OPEN_ERROR:
        return "ROOT_OPEN_ERROR";
      default:
        return StatusCode::default_category().message( code );
      }
    }
  };

  static bool match_wild( const char* str, const char* pat ) {
    //
    // Credits: Code from Alessandro Felice Cantatore.
    //
    static const auto table = init_table();
    const char *      s, *p;
    bool              star = false;
  loopStart:
    for ( s = str, p = pat; *s; ++s, ++p ) {
      switch ( *p ) {
      case '?':
        if ( *s == '.' ) goto starCheck;
        break;
      case '*':
        star = true;
        str = s, pat = p;
        do { ++pat; } while ( *pat == '*' );
        if ( !*pat ) return true;
        goto loopStart;
      default:
        if ( table[*s] != table[*p] ) goto starCheck;
        break;
      } /* endswitch */
    }   /* endfor */
    while ( *p == '*' ) ++p;
    return ( !*p );

  starCheck:
    if ( !star ) return false;
    str++;
    goto loopStart;
  }
} // namespace

STATUSCODE_ENUM_IMPL( Gaudi::RootDataConnection::Status, RootDataConnectionCategory )

/// Set the global compression level
StatusCode RootConnectionSetup::setCompression( std::string_view compression ) {
#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 33, 0 )
  int  res = 0, level = ROOT::CompressionSettings( ROOT::kLZMA, 6 );
  auto idx = compression.find( ':' );
  if ( idx != string::npos ) {
    auto                        alg      = compression.substr( 0, idx );
    ROOT::ECompressionAlgorithm alg_code = ROOT::kUseGlobalSetting;
    if ( alg.size() == 4 && strncasecmp( alg.data(), "ZLIB", 4 ) == 0 )
      alg_code = ROOT::kZLIB;
    else if ( alg.size() == 4 && strncasecmp( alg.data(), "LZMA", 4 ) == 0 )
      alg_code = ROOT::kLZMA;
    else if ( alg.size() == 3 && strncasecmp( alg.data(), "LZ4", 3 ) == 0 )
      alg_code = ROOT::kLZ4;
#  if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 20, 0 )
    else if ( alg.size() == 4 && strncasecmp( alg.data(), "ZSTD", 4 ) == 0 )
      alg_code = ROOT::kZSTD;
#  endif
    else
      throw runtime_error( "ERROR: request to set unknown ROOT compression algorithm:" + std::string{ alg } );
    res = ::sscanf( std::string{ compression.substr( idx + 1 ) }.c_str(), "%d",
                    &level ); // TODO: use C++17 std::from_chars instead...
    if ( res == 1 ) {
      s_compressionLevel = ROOT::CompressionSettings( alg_code, level );
      return StatusCode::SUCCESS;
    }
    throw runtime_error( "ERROR: request to set unknown ROOT compression level:" +
                         std::string{ compression.substr( idx + 1 ) } );
  } else if ( 1 == ::sscanf( std::string{ compression }.c_str(), "%d", &level ) ) { // TODO: use C++17 std::from_chars
                                                                                    // instead
    s_compressionLevel = level;
    return StatusCode::SUCCESS;
  }
  throw runtime_error( "ERROR: request to set unknown ROOT compression mechanism:" + std::string{ compression } );
#else
  if ( !compression.empty() ) {}
  return StatusCode::SUCCESS;
#endif
}

/// Global compression level
int RootConnectionSetup::compression() { return s_compressionLevel; }

/// Set message service reference
void RootConnectionSetup::setMessageSvc( MsgStream* m ) { m_msgSvc.reset( m ); }

/// Set incident service reference
void RootConnectionSetup::setIncidentSvc( IIncidentSvc* s ) { m_incidentSvc.reset( s ); }

/// Standard constructor
RootDataConnection::RootDataConnection( const IInterface* owner, std::string_view fname,
                                        std::shared_ptr<RootConnectionSetup> setup )
    : IDataConnection( owner, std::string{ fname } )
    , m_setup( std::move( setup ) ) { //               01234567890123456789012345678901234567890
  // Check if FID: A82A3BD8-7ECB-DC11-8DC0-000423D950B0
  if ( fname.size() == 36 && fname[8] == '-' && fname[13] == '-' && fname[18] == '-' && fname[23] == '-' ) {
    m_name = "FID:";
    m_name.append( fname.data(), fname.size() );
  }
  m_age = 0;
  m_file.reset();
  addClient( owner );
}

/// Add new client to this data source
void RootDataConnection::addClient( const IInterface* client ) { m_clients.insert( client ); }

/// Remove client from this data source
size_t RootDataConnection::removeClient( const IInterface* client ) {
  auto i = m_clients.find( client );
  if ( i != m_clients.end() ) m_clients.erase( i );
  return m_clients.size();
}

/// Lookup client for this data source
bool RootDataConnection::lookupClient( const IInterface* client ) const {
  auto i = m_clients.find( client );
  return i != m_clients.end();
}

/// Error handler when bad write statements occur
void RootDataConnection::badWriteError( std::string_view msg ) const {
  msgSvc() << MSG::ERROR << "File:" << fid() << "Failed action:" << msg << endmsg;
}

/// Save TTree access statistics if required
void RootDataConnection::saveStatistics( std::string_view statisticsFile ) {
  if ( m_statistics ) {
    m_statistics->Print();
    if ( !statisticsFile.empty() ) m_statistics->SaveAs( std::string{ statisticsFile }.c_str() );
    m_statistics.reset();
  }
}

/// Enable TTreePerStats
void RootDataConnection::enableStatistics( std::string_view section ) {
  if ( m_statistics ) {
    TTree* t = getSection( section, false );
    if ( t ) {
      m_statistics.reset( new TTreePerfStats( ( std::string{ section } + "_ioperf" ).c_str(), t ) );
      return;
    }
    msgSvc() << MSG::WARNING << "Failed to enable perfstats for tree:" << section << endmsg;
    return;
  }
  msgSvc() << MSG::INFO << "Perfstats are ALREADY ENABLED." << endmsg;
}

/// Create file access tool to encapsulate POOL compatibiliy
RootDataConnection::Tool* RootDataConnection::makeTool() {
  if ( !m_refs ) m_refs = (TTree*)m_file->Get( "Refs" );
  if ( m_refs ) m_tool.reset( new RootTool( this ) );
#ifdef __POOL_COMPATIBILITY
  else if ( m_file->Get( "##Links" ) != nullptr )
    m_tool.reset( new PoolTool( this ) );
#endif
  else
    m_tool.reset();
  return m_tool.get();
}

/// Connect the file in READ mode
StatusCode RootDataConnection::connectRead() {
  m_file.reset( TFile::Open( m_pfn.c_str() ) );
  if ( !m_file || m_file->IsZombie() ) {
    m_file.reset();
    return StatusCode::FAILURE;
  }
  StatusCode sc = StatusCode::FAILURE;
  msgSvc() << MSG::DEBUG << "Opened file " << m_pfn << " in mode READ. [" << m_fid << "]" << endmsg << MSG::DEBUG;
  if ( msgSvc().isActive() ) m_file->ls();
  msgSvc() << MSG::VERBOSE;
  if ( msgSvc().isActive() ) m_file->Print();
  if ( makeTool() ) {
    sc = m_tool->readRefs();
    sc.ignore();
#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 33, 0 )
    if ( sc == Status::ROOT_READ_ERROR ) {
      IIncidentSvc* inc = m_setup->incidentSvc();
      if ( inc ) { inc->fireIncident( Incident( pfn(), IncidentType::CorruptedInputFile ) ); }
    }
#endif
  }
  if ( !sc.isSuccess() ) return sc;
  bool   need_fid = m_fid == m_pfn;
  string fid      = m_fid;
  m_mergeFIDs.clear();
  for ( auto& elem : m_params ) {
    if ( elem.first == "FID" ) {
      m_mergeFIDs.push_back( elem.second );
      if ( elem.second != m_fid ) {
        msgSvc() << MSG::DEBUG << "Check FID param:" << elem.second << endmsg;
        // if ( m_fid == m_pfn ) {
        m_fid = elem.second;
        //}
      }
    }
  }
  if ( !need_fid && fid != m_fid ) {
    msgSvc() << MSG::ERROR << "FID mismatch:" << fid << "(Catalog) != " << m_fid << "(file)" << endmsg
             << "for PFN:" << m_pfn << endmsg;
    return StatusCode::FAILURE;
  }
  msgSvc() << MSG::DEBUG << "Using FID " << m_fid << " from params table...." << endmsg << "for PFN:" << m_pfn
           << endmsg;
  return sc;
}

/// Open data stream in write mode
StatusCode RootDataConnection::connectWrite( IoType typ ) {
  int compress = RootConnectionSetup::compression();
  msgSvc() << MSG::DEBUG;
  std::string spec = m_pfn;
  if ( m_setup->produceReproducibleFiles ) spec += "?reproducible"; // https://root.cern.ch/doc/master/classTFile.html
  switch ( typ ) {
  case CREATE:
    resetAge();
    m_file.reset( TFile::Open( spec.c_str(), "CREATE", "Root event data", compress ) );
    m_refs = new TTree( "Refs", "Root reference data" );
    msgSvc() << "Opened file " << m_pfn << " in mode CREATE. [" << m_fid << "]" << endmsg;
    m_params.emplace_back( "PFN", m_pfn );
    if ( m_fid != m_pfn ) { m_params.emplace_back( "FID", m_fid ); }
    makeTool();
    break;
  case RECREATE:
    resetAge();
    m_file.reset( TFile::Open( spec.c_str(), "RECREATE", "Root event data", compress ) );
    msgSvc() << "Opened file " << m_pfn << " in mode RECREATE. [" << m_fid << "]" << endmsg;
    m_refs = new TTree( "Refs", "Root reference data" );
    m_params.emplace_back( "PFN", m_pfn );
    if ( m_fid != m_pfn ) { m_params.emplace_back( "FID", m_fid ); }
    makeTool();
    break;
  case UPDATE:
    resetAge();
    m_file.reset( TFile::Open( spec.c_str(), "UPDATE", "Root event data", compress ) );
    msgSvc() << "Opened file " << m_pfn << " in mode UPDATE. [" << m_fid << "]" << endmsg;
    if ( m_file && !m_file->IsZombie() ) {
      if ( makeTool() ) {
        StatusCode sc = m_tool->readRefs();
        sc.ignore();
        if ( sc == Status::ROOT_READ_ERROR ) {
#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 33, 0 )
          IIncidentSvc* inc = m_setup->incidentSvc();
          if ( inc ) { inc->fireIncident( Incident( pfn(), IncidentType::CorruptedInputFile ) ); }
#endif
        }
        return sc;
      }
      TDirectory::TContext ctxt( m_file.get() );
      m_refs = new TTree( "Refs", "Root reference data" );
      makeTool();
      return StatusCode::SUCCESS;
    }
    break;
  default:
    m_refs = nullptr;
    m_file.reset();
    return StatusCode::FAILURE;
  }
  return m_file ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

/// Release data stream and release implementation dependent resources
StatusCode RootDataConnection::disconnect() {
  if ( m_file ) {
    if ( !m_file->IsZombie() ) {
      if ( m_file->IsWritable() ) {
        msgSvc() << MSG::DEBUG;
        TDirectory::TContext ctxt( m_file.get() );
        if ( m_refs ) {
          if ( !m_tool->saveRefs().isSuccess() ) badWriteError( "Saving References" );
          if ( m_refs->Write() < 0 ) badWriteError( "Write Reference branch" );
        }
        for ( auto& i : m_sections ) {
          if ( i.second ) {
            if ( i.second->Write() < 0 ) badWriteError( "Write section:" + i.first );
            msgSvc() << "Disconnect section " << i.first << " " << i.second->GetName() << endmsg;
          }
        }
        m_sections.clear();
      }
      msgSvc() << MSG::DEBUG;
      if ( msgSvc().isActive() ) m_file->ls();
      msgSvc() << MSG::VERBOSE;
      if ( msgSvc().isActive() ) m_file->Print();
      m_file->Close();
    }
    msgSvc() << MSG::DEBUG << "Disconnected file " << m_pfn << " " << m_file->GetName() << endmsg;
    m_file.reset();
    m_tool.reset();
  }
  return StatusCode::SUCCESS;
}

/// Access TTree section from section name. The section is created if required.
TTree* RootDataConnection::getSection( std::string_view section, bool create ) {
  auto   it = m_sections.find( section );
  TTree* t  = ( it != m_sections.end() ? it->second : nullptr );
  if ( !t ) {
    t = (TTree*)m_file->Get( std::string{ section }.c_str() );
    if ( !t && create ) {
      TDirectory::TContext ctxt( m_file.get() );
      t = new TTree( std::string{ section }.c_str(), "Root data for Gaudi" );
    }
    if ( t ) {
      int cacheSize = m_setup->cacheSize;
      if ( create ) {
        // t->SetAutoFlush(100);
      }
      if ( section == m_setup->loadSection && cacheSize > -2 ) {
        MsgStream& msg          = msgSvc();
        int        learnEntries = m_setup->learnEntries;
        t->SetCacheSize( cacheSize );
        t->SetCacheLearnEntries( learnEntries );
        msg << MSG::DEBUG;
        if ( create ) {
          msg << "Tree:" << section << "Setting up tree cache:" << cacheSize << endmsg;
        } else {
          const StringVec& vB = m_setup->vetoBranches;
          const StringVec& cB = m_setup->cacheBranches;
          msg << "Tree:" << section << " Setting up tree cache:" << cacheSize << " Add all branches." << endmsg;
          msg << "Tree:" << section << " Learn for " << learnEntries << " entries." << endmsg;

          if ( cB.empty() && vB.empty() ) {
            msg << "Adding (default) all branches to tree cache." << endmsg;
            t->AddBranchToCache( "*", kTRUE );
          }
          if ( cB.size() == 1 && cB[0] == "*" ) {
            msg << "Adding all branches to tree cache according to option \"CacheBranches\"." << endmsg;
            t->AddBranchToCache( "*", kTRUE );
          } else {
            for ( TIter it( t->GetListOfBranches() ); it.Next(); ) {
              const char* n   = ( (TNamed*)( *it ) )->GetName();
              bool        add = false, veto = false;
              for ( const auto& i : cB ) {
                if ( !match_wild( n, ( i ).c_str() ) ) continue;
                add = true;
                break;
              }
              for ( auto i = vB.cbegin(); !add && i != vB.cend(); ++i ) {
                if ( !match_wild( n, ( *i ).c_str() ) ) continue;
                veto = true;
                break;
              }
              if ( add && !veto ) {
                msg << "Add " << n << " to branch cache." << endmsg;
                t->AddBranchToCache( n, kTRUE );
              } else {
                msg << "Do not cache branch " << n << endmsg;
              }
            }
          }
        }
      }
      m_sections[std::string{ section }] = t;
    }
  }
  return t;
}

/// Access data branch by name: Get existing branch in write mode
TBranch* RootDataConnection::getBranch( std::string_view section, std::string_view branch_name, TClass* cl, void* ptr,
                                        int buff_siz, int split_lvl ) {
  string n = std::string{ branch_name };
  std::replace_if(
      begin( n ), end( n ), []( const char c ) { return !isalnum( c ); }, '_' );
  n += ".";
  TTree*   t = getSection( section, true );
  TBranch* b = t->GetBranch( n.c_str() );
  if ( !b && cl && m_file->IsWritable() ) {
    b = t->Branch( n.c_str(), cl->GetName(), (void*)( ptr ? &ptr : nullptr ), buff_siz, split_lvl );
  }
  if ( !b ) b = t->GetBranch( std::string{ branch_name }.c_str() );
  if ( b ) b->SetAutoDelete( kFALSE );
  return b;
}

/// Convert path string to path index
int RootDataConnection::makeLink( std::string_view p ) {
  auto ip = std::find( std::begin( m_links ), std::end( m_links ), p );
  if ( ip != std::end( m_links ) ) return std::distance( std::begin( m_links ), ip );
  m_links.push_back( std::string{ p } );
  return m_links.size() - 1;
}

/// Access database/file name from saved index
CSTR RootDataConnection::getDb( int which ) const {
  if ( ( which >= 0 ) && ( size_t( which ) < m_dbs.size() ) ) {
    if ( *( m_dbs.begin() + which ) == s_local ) return m_fid;
    return *( m_dbs.begin() + which );
  }
  return s_empty;
}

/// Empty string reference
CSTR RootDataConnection::empty() const { return s_empty; }

/// Save object of a given class to section and container
pair<int, unsigned long> RootDataConnection::saveObj( std::string_view section, std::string_view cnt, TClass* cl,
                                                      DataObject* pObj, int minBufferSize, int maxBufferSize,
                                                      int approxEventsPerBasket, int split_lvl, bool fill ) {
  DataObjectPush push( pObj );
  return save( section, cnt, cl, pObj, minBufferSize, maxBufferSize, approxEventsPerBasket, split_lvl, fill );
}

/// Save object of a given class to section and container
pair<int, unsigned long> RootDataConnection::save( std::string_view section, std::string_view cnt, TClass* cl,
                                                   void* pObj, int minBufferSize, int maxBufferSize,
                                                   int approxEventsPerBasket, int split_lvl, bool fill_missing ) {
  split_lvl  = 0;
  TBranch* b = getBranch( section, cnt, cl, pObj ? &pObj : nullptr, minBufferSize, split_lvl );
  if ( b ) {
    Long64_t evt = b->GetEntries();
    // msgSvc() << MSG::DEBUG << cnt.c_str() << " Obj:" << (void*)pObj
    //         << " Split:" << split_lvl << " Buffer size:" << minBufferSize << endl;
    bool set_buffer_size = ( evt == 0 );
    if ( fill_missing ) {
      Long64_t num, nevt = b->GetTree()->GetEntries();
      if ( nevt > evt ) {
        set_buffer_size = true;
        b->SetAddress( nullptr );
        num = nevt - evt;
        while ( num > 0 ) {
          b->Fill();
          --num;
        }
        msgSvc() << MSG::DEBUG << "Added " << long( nevt - evt ) << " / Tree: " << nevt
                 << " / Branch: " << b->GetEntries() + 1 << " NULL entries to:" << cnt << endmsg;
        evt = b->GetEntries();
      }
    }
    if ( set_buffer_size ) {
      auto     dummy_file = make_unique<TMemFile>( "dummy.root", "CREATE" );
      auto     dummy_tree = make_unique<TTree>( "DummyTree", "DummyTree", split_lvl, dummy_file->GetDirectory( "/" ) );
      TBranch* dummy_branch = dummy_tree->Branch( "DummyBranch", cl->GetName(), &pObj, minBufferSize, split_lvl );
      Int_t    nWritten     = dummy_branch->Fill();
      if ( nWritten < 0 ) return { nWritten, evt };
      Int_t newBasketSize = nWritten * approxEventsPerBasket;
      // Ensure that newBasketSize doesn't wrap around
      if ( std::numeric_limits<Int_t>::max() / approxEventsPerBasket < nWritten ) {
        newBasketSize = std::numeric_limits<Int_t>::max();
      }
      b->SetBasketSize( std::min( maxBufferSize, std::max( minBufferSize, newBasketSize ) ) );
      msgSvc() << MSG::DEBUG << "Setting basket size to " << newBasketSize << " for " << cnt << endmsg;
    }
    b->SetAddress( &pObj );
    return { b->Fill(), evt };
  }
  if ( pObj ) { msgSvc() << MSG::ERROR << "Failed to access branch " << m_name << "/" << cnt << endmsg; }
  return { -1, ~0 };
}

/// Load object
int RootDataConnection::loadObj( std::string_view section, std::string_view cnt, unsigned long entry,
                                 DataObject*& pObj ) {
  TBranch* b = getBranch( section, cnt );
  if ( b ) {
    TClass* cl = gROOT->GetClass( b->GetClassName(), kTRUE );
    if ( cl ) {
      int nb = -1;
      pObj   = (DataObject*)cl->New();
      {
        DataObjectPush push( pObj );
        b->SetAddress( &pObj );
        if ( section == m_setup->loadSection ) {
          TTree* t = b->GetTree();
          if ( Long64_t( entry ) != t->GetReadEntry() ) { t->LoadTree( Long64_t( entry ) ); }
        }
        nb = b->GetEntry( entry );
        msgSvc() << MSG::VERBOSE;
        if ( msgSvc().isActive() ) {
          msgSvc() << "Load [" << entry << "] --> " << section << ":" << cnt << "  " << nb << " bytes." << endmsg;
        }
        if ( nb < 0 ) { // This is definitely an error...ROOT says if reads fail, -1 is issued.
#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 33, 0 )
          IIncidentSvc* inc = m_setup->incidentSvc();
          if ( inc ) { inc->fireIncident( Incident( pfn(), IncidentType::CorruptedInputFile ) ); }
#endif
        } else if ( nb == 0 && pObj->clID() == CLID_DataObject ) {
          TFile* f   = b->GetFile();
          int    vsn = f->GetVersion();
          if ( vsn < 52400 ) {
            // For Gaudi v21r5 (ROOT 5.24.00b) DataObject::m_version was not written!
            // Still this call be well be successful.
            nb = 1;
          } else if ( vsn > 1000000 && ( vsn % 1000000 ) < 52400 ) {
            // dto. Some POOL files have for unknown reasons a version
            // not according to ROOT standards. Hack this explicitly.
            nb = 1;
          }
        }
        if ( nb < 0 ) {
          delete pObj;
          pObj = nullptr;
        }
      }
      return nb;
    }
  }
  return -1;
}

/// Load references object
int RootDataConnection::loadRefs( std::string_view section, std::string_view cnt, unsigned long entry,
                                  RootObjectRefs& refs ) {
  int nbytes = m_tool->loadRefs( section, cnt, entry, refs );
#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 33, 0 )
  if ( nbytes < 0 ) {
    // This is definitely an error:
    // -- Either branch not preesent at all or
    // -- ROOT I/O error, which issues -1
    IIncidentSvc* inc = m_setup->incidentSvc();
    if ( inc ) { inc->fireIncident( Incident( pfn(), IncidentType::CorruptedInputFile ) ); }
  }
#endif
  return nbytes;
}

/// Access link section for single container and entry
pair<const RootRef*, const RootDataConnection::ContainerSection*>
RootDataConnection::getMergeSection( std::string_view container, int entry ) const {
  // size_t idx = cont.find('/',1);
  // string container = cont[0]=='/' ? cont.substr(1,idx==string::npos?idx:idx-1) : cont;
  auto i = m_mergeSects.find( container );
  if ( i != m_mergeSects.end() ) {
    size_t                   cnt = 0;
    const ContainerSections& s   = ( *i ).second;
    for ( auto j = s.cbegin(); j != s.cend(); ++j, ++cnt ) {
      const ContainerSection& c = *j;
      if ( entry >= c.start && entry < ( c.start + c.length ) ) {
        if ( m_linkSects.size() > cnt ) {
          if ( msgSvc().isActive() ) {
            msgSvc() << MSG::VERBOSE << "MergeSection for:" << container << "  [" << entry << "]" << endmsg
                     << "FID:" << m_fid << " -> PFN:" << m_pfn << endmsg;
          }
          return { &( m_linkSects[cnt] ), &c };
        }
      }
    }
  }
  msgSvc() << MSG::DEBUG << "Return INVALID MergeSection for:" << container << "  [" << entry << "]" << endmsg
           << "FID:" << m_fid << " -> PFN:" << m_pfn << endmsg;
  return { nullptr, nullptr };
}

/// Create reference object from registry entry
void RootDataConnection::makeRef( const IRegistry& pR, RootRef& ref ) {
  IOpaqueAddress* pA = pR.address();
  makeRef( pR.name(), pA->clID(), pA->svcType(), pA->par()[0], pA->par()[1], -1, ref );
}

/// Create reference object from values
void RootDataConnection::makeRef( std::string_view name, long clid, int tech, std::string_view dbase,
                                  std::string_view cnt, int entry, RootRef& ref ) {
  auto db   = ( dbase == m_fid ? std::string_view{ s_local } : dbase );
  ref.entry = entry;

  int cdb = -1;
  if ( !db.empty() ) {
    auto idb = std::find_if( m_dbs.begin(), m_dbs.end(), [&]( const std::string& i ) { return i == db; } );
    cdb      = std::distance( m_dbs.begin(), idb );
    if ( idb == m_dbs.end() ) m_dbs.push_back( std::string{ db } );
  }

  int ccnt = -1;
  if ( !cnt.empty() ) {
    auto icnt = std::find_if( m_conts.begin(), m_conts.end(), [&]( const std::string& i ) { return i == cnt; } );
    ccnt      = std::distance( m_conts.begin(), icnt );
    if ( icnt == m_conts.end() ) m_conts.push_back( std::string{ cnt } );
  }

  int clnk = -1;
  if ( !name.empty() ) {
    auto ilnk = std::find_if( m_links.begin(), m_links.end(), [&]( const std::string& i ) { return i == name; } );
    clnk      = std::distance( m_links.begin(), ilnk );
    if ( ilnk == m_links.end() ) m_links.push_back( std::string{ name } );
  }

  ref.dbase     = cdb;
  ref.container = ccnt;
  ref.link      = clnk;
  ref.clid      = clid;
  ref.svc       = tech;
  if ( ref.svc == POOL_ROOT_StorageType || ref.svc == POOL_ROOTKEY_StorageType ||
       ref.svc == POOL_ROOTTREE_StorageType ) {
    ref.svc = ROOT_StorageType;
  }
}
