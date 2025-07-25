/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
//====================================================================
//	RootEvtSelector.cpp
//--------------------------------------------------------------------
//
//	Package    : RootCnv
//
//	Author     : M.Frank
//====================================================================
#ifndef GAUDIROOTCNV_ROOTEVTSELECTORCONTEXT_H
#  define GAUDIROOTCNV_ROOTEVTSELECTORCONTEXT_H

// Include files
#  include <RootCnv/RootEvtSelector.h>
#  include <vector>

// Forward declarations
class TBranch;

// FIXME - method below generates leak errors with sanitizer
//
// clang-format off
// Direct leak of 176 byte(s) in 1 object(s) allocated from:
//    #0 0x7f56e2dc2da8 in operator new(unsigned long) /afs/cern.ch/cms/CAF/CMSCOMM/COMM_ECAL/dkonst/GCC/build/contrib/gcc-8.2.0/src/gcc/8.2.0/libsanitizer/lsan/lsan_interceptors.cc:229
//    #1 0x7f56cb992ae4 in Gaudi::RootCnvSvc::createAddress(long, unsigned int const&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const*, unsigned long const*, IOpaqueAddress*&) ../RootCnv/src/RootCnvSvc.cpp:349
//    #2 0x7f56cb993530 in Gaudi::RootCnvSvc::connectDatabase(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&, int, Gaudi::RootDataConnection**) ../RootCnv/src/RootCnvSvc.cpp:242
//    #3 0x7f56cb9b5230 in Gaudi::RootEvtSelector::next(IEvtSelector::Context&) const ../RootCnv/src/RootEvtSelector.cpp:168
//    #4 0x7f56d2e748b3 in EventSelector::next(IEvtSelector::Context&, int) const ../GaudiCoreSvc/src/EventSelector/EventSelector.cpp:188
//    #5 0x7f56d2e737bf in EventSelector::next(IEvtSelector::Context&) const ../GaudiCoreSvc/src/EventSelector/EventSelector.cpp:177
//    #6 0x7f56d2e4c68f in EventLoopMgr::getEventRoot(IOpaqueAddress*&) ../GaudiCoreSvc/src/ApplicationMgr/EventLoopMgr.cpp:350
//    #7 0x7f56d2e4dab5 in EventLoopMgr::nextEvent(int) ../GaudiCoreSvc/src/ApplicationMgr/EventLoopMgr.cpp:307
//    #8 0x7f56d3fd5a5a in MinimalEventLoopMgr::executeRun(int) ../GaudiKernel/src/Lib/MinimalEventLoopMgr.cpp:296
//    #9 0x7f56d2e289ac in ApplicationMgr::executeRun(int) ../GaudiCoreSvc/src/ApplicationMgr/ApplicationMgr.cpp:824
//    #10 0x7f56d3f5a21c in Gaudi::Application::run() ../GaudiKernel/src/Lib/Application.cpp:81
// clang-format on
//
// These leaks are currently suppressed in Gaudi/job/Gaudi-LSan.supp - remove entry there to reactivate

/*
 *  Gaudi namespace declaration
 */
namespace Gaudi {

  /** @class RootEvtSelectorContext
   *
   *  ROOT specific event selector context.
   *  See the base class for a detailed description.
   *
   *  @author  M.Frank
   *  @version 1.0
   */
  class RootEvtSelectorContext : public IEvtSelector::Context {
  public:
    /// Definition of the file container
    typedef std::vector<std::string> Files;

  private:
    /// Reference to the hosting event selector instance
    const RootEvtSelector* m_sel;
    /// The file container managed by this context
    Files m_files;
    /// The iterator to the
    Files::const_iterator m_fiter;
    /// Current entry of current file
    long m_entry;
    /// Reference to the top level branch (typically /Event) used to iterate
    TBranch* m_branch;
    /// Connection fid
    std::string m_fid;

  public:
    /// Standard constructor with initialization
    RootEvtSelectorContext( const RootEvtSelector* s ) : m_sel( s ), m_entry( -1 ), m_branch( nullptr ) {}
    /// Access to the file container
    const Files& files() const { return m_files; }
    /// Set the file container
    void setFiles( const Files& f ) {
      m_files = f;
      m_fiter = m_files.begin();
    }
    /// Context identifier
    void* identifier() const override { return const_cast<RootEvtSelector*>( m_sel ); }
    /// Access to the file iterator
    Files::const_iterator fileIterator() const { return m_fiter; }
    /// Set file iterator
    void setFileIterator( Files::const_iterator i ) { m_fiter = i; }
    /// Access to the current event entry number
    long entry() const { return m_entry; }
    /// Set current event entry number
    void setEntry( long e ) { m_entry = e; }
    /// Set connection FID
    void setFID( const std::string& fid ) { m_fid = fid; }
    /// Access connection fid
    const std::string& fid() const { return m_fid; }
    /// Access to the top level branch (typically /Event) used to iterate
    TBranch* branch() const { return m_branch; }
    /// Set the top level branch (typically /Event) used to iterate
    void setBranch( TBranch* b ) { m_branch = b; }
  };
} // namespace Gaudi
#endif // GAUDIROOTCNV_ROOTEVTSELECTORCONTEXT_H

// Include files
#include <GaudiKernel/AttribStringParser.h>
#include <GaudiKernel/ClassID.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IPersistencySvc.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/TypeNameString.h>
#include <RootCnv/RootCnvSvc.h>
#include <RootCnv/RootDataConnection.h>
#include <TBranch.h>

using namespace Gaudi;
using namespace std;

// Helper method to issue error messages
StatusCode RootEvtSelector::error( const string& msg ) const {
  MsgStream log( msgSvc(), name() );
  log << MSG::ERROR << msg << endmsg;
  return StatusCode::FAILURE;
}

// IService implementation: Db event selector override
StatusCode RootEvtSelector::initialize() {
  // Initialize base class
  StatusCode status = Service::initialize();
  if ( !status.isSuccess() ) { return error( "Error initializing base class Service!" ); }

  auto ipers = serviceLocator()->service<IPersistencySvc>( m_persName );
  if ( !ipers ) { return error( "Unable to locate IPersistencySvc interface of " + m_persName ); }
  IConversionSvc*              cnvSvc = nullptr;
  Gaudi::Utils::TypeNameString itm( m_cnvSvcName );
  status = ipers->getService( itm.name(), cnvSvc );
  if ( !status.isSuccess() ) {
    status = ipers->getService( itm.type(), cnvSvc );
    if ( !status.isSuccess() ) {
      return error( "Unable to locate IConversionSvc interface of database type " + m_cnvSvcName );
    }
  }
  m_dbMgr = dynamic_cast<RootCnvSvc*>( cnvSvc );
  if ( !m_dbMgr ) {
    cnvSvc->release();
    return error( "Unable to localize service:" + m_cnvSvcName );
  }
  m_dbMgr->addRef();

  // Get DataSvc
  auto eds = serviceLocator()->service<IDataManagerSvc>( "EventDataSvc" );
  if ( !eds ) { return error( "Unable to localize service EventDataSvc" ); }
  m_rootCLID = eds->rootCLID();
  m_rootName = eds->rootName();
  MsgStream log( msgSvc(), name() );
  log << MSG::DEBUG << "Selection root:" << m_rootName << " CLID:" << m_rootCLID << endmsg;
  return status;
}

// IService implementation: Service finalization
StatusCode RootEvtSelector::finalize() {
  // Initialize base class
  if ( m_dbMgr ) m_dbMgr->release();
  m_dbMgr = nullptr; // release
  return Service::finalize();
}

// Create a new event loop context
StatusCode RootEvtSelector::createContext( Context*& refpCtxt ) const {
  refpCtxt = new RootEvtSelectorContext( this );
  return StatusCode::SUCCESS;
}

// Access last item in the iteration
StatusCode RootEvtSelector::last( Context& /*refContext*/ ) const { return StatusCode::FAILURE; }

// Get next iteration item from the event loop context
StatusCode RootEvtSelector::next( Context& ctxt ) const {
  RootEvtSelectorContext* pCtxt = dynamic_cast<RootEvtSelectorContext*>( &ctxt );
  if ( pCtxt ) {
    TBranch* b = pCtxt->branch();
    if ( !b ) {
      auto fileit = pCtxt->fileIterator();
      pCtxt->setBranch( nullptr );
      pCtxt->setEntry( -1 );
      if ( fileit != pCtxt->files().end() ) {
        RootDataConnection* con = nullptr;
        string              in  = *fileit;
        StatusCode          sc  = m_dbMgr->connectDatabase( in, IDataConnection::READ, &con );
        if ( sc.isSuccess() ) {
          string section = m_rootName[0] == '/' ? m_rootName.substr( 1 ) : m_rootName;
          b              = con->getBranch( section, m_rootName );
          if ( b ) {
            pCtxt->setFID( con->fid() );
            pCtxt->setBranch( b );
            return next( ctxt );
          }
        }
        m_dbMgr->disconnect( in ).ignore();
        pCtxt->setFileIterator( ++fileit );
        return next( ctxt );
      }
      return StatusCode::FAILURE;
    }
    long     ent  = pCtxt->entry();
    Long64_t nent = b->GetEntries();
    if ( nent > ( ent + 1 ) ) {
      pCtxt->setEntry( ++ent );
      return StatusCode::SUCCESS;
    }
    auto fit = pCtxt->fileIterator();
    pCtxt->setFileIterator( ++fit );
    pCtxt->setEntry( -1 );
    pCtxt->setBranch( nullptr );
    pCtxt->setFID( "" );
    return next( ctxt );
  }
  return StatusCode::FAILURE;
}

// Get next iteration item from the event loop context
StatusCode RootEvtSelector::next( Context& ctxt, int jump ) const {
  if ( jump > 0 ) {
    for ( int i = 0; i < jump; ++i ) {
      StatusCode status = next( ctxt );
      if ( !status.isSuccess() ) { return status; }
    }
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

// Get previous iteration item from the event loop context
StatusCode RootEvtSelector::previous( Context& /* ctxt */ ) const {
  return error( "EventSelector Iterator, operator -- not supported " );
}

// Get previous iteration item from the event loop context
StatusCode RootEvtSelector::previous( Context& ctxt, int jump ) const {
  if ( jump > 0 ) {
    for ( int i = 0; i < jump; ++i ) {
      StatusCode status = previous( ctxt );
      if ( !status.isSuccess() ) { return status; }
    }
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

// Rewind the dataset
StatusCode RootEvtSelector::rewind( Context& ctxt ) const {
  RootEvtSelectorContext* pCtxt = dynamic_cast<RootEvtSelectorContext*>( &ctxt );
  if ( pCtxt ) {
    auto fileit = pCtxt->fileIterator();
    if ( fileit != pCtxt->files().end() ) {
      string input = *fileit;
      m_dbMgr->disconnect( input ).ignore();
    }
    pCtxt->setFID( "" );
    pCtxt->setEntry( -1 );
    pCtxt->setBranch( nullptr );
    pCtxt->setFileIterator( pCtxt->files().begin() );
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

// Create new Opaque address corresponding to the current record
StatusCode RootEvtSelector::createAddress( const Context& ctxt, IOpaqueAddress*& pAddr ) const {
  const RootEvtSelectorContext* pctxt = dynamic_cast<const RootEvtSelectorContext*>( &ctxt );
  if ( pctxt ) {
    long ent = pctxt->entry();
    if ( ent >= 0 ) {
      auto fileit = pctxt->fileIterator();
      if ( fileit != pctxt->files().end() ) {
        const string        par[2]  = { pctxt->fid(), m_rootName };
        const unsigned long ipar[2] = { 0, (unsigned long)ent };
        return m_dbMgr->createAddress( m_dbMgr->repSvcType(), m_rootCLID, &par[0], &ipar[0], pAddr );
      }
    }
  }
  pAddr = nullptr;
  return StatusCode::FAILURE;
}

// Release existing event iteration context
StatusCode RootEvtSelector::releaseContext( Context*& ctxt ) const {
  RootEvtSelectorContext* pCtxt = dynamic_cast<RootEvtSelectorContext*>( ctxt );
  if ( pCtxt ) {
    delete pCtxt;
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

// Will set a new criteria for the selection of the next list of events and will change
// the state of the context in a way to point to the new list.
StatusCode RootEvtSelector::resetCriteria( const string& criteria, Context& context ) const {
  MsgStream               log( msgSvc(), name() );
  RootEvtSelectorContext* ctxt = dynamic_cast<RootEvtSelectorContext*>( &context );
  string                  db, typ, item, sel, stmt, aut, addr;
  if ( ctxt ) {
    if ( criteria.compare( 0, 5, "FILE " ) == 0 ) {
      // The format for the criteria is:
      //        FILE  filename1, filename2 ...
      db = criteria.substr( 5 );
    } else {
      using Parser = Gaudi::Utils::AttribStringParser;
      for ( auto attrib : Parser( criteria ) ) {
        string tmp = attrib.tag.substr( 0, 3 );
        if ( tmp == "DAT" ) {
          db = std::move( attrib.value );
        } else if ( tmp == "OPT" ) {
          if ( attrib.value.compare( 0, 3, "REA" ) != 0 ) {
            log << MSG::ERROR << "Option:\"" << attrib.value << "\" not valid" << endmsg;
            return StatusCode::FAILURE;
          }
        } else if ( tmp == "TYP" ) {
          typ = std::move( attrib.value );
        } else if ( tmp == "ADD" ) {
          item = std::move( attrib.value );
        } else if ( tmp == "SEL" ) {
          sel = std::move( attrib.value );
        } else if ( tmp == "FUN" ) {
          stmt = std::move( attrib.value );
        } else if ( tmp == "AUT" ) {
          aut = std::move( attrib.value );
        } else if ( tmp == "COL" ) {
          addr = std::move( attrib.value );
        }
      }
    }
    // It's now time to parse the criteria for the event selection
    // The format for the criteria is:
    //        FILE  filename1, filename2 ...
    //        JOBID number1-number2, number3, ...
    RootEvtSelectorContext::Files files;
    string                        rest = db;
    files.clear();
    while ( true ) {
      int ipos = rest.find_first_not_of( " ," );
      if ( ipos == -1 ) break;
      rest     = rest.substr( ipos, string::npos ); // remove blanks before
      int lpos = rest.find_first_of( " ," );        // locate next blank

      files.push_back( rest.substr( 0, lpos == -1 ? string::npos : lpos ) ); // insert in list
      if ( lpos == -1 ) break;
      rest = rest.substr( lpos, string::npos ); // get the rest
    }
    ctxt->setFiles( files );
    ctxt->setFileIterator( ctxt->files().begin() );
    return StatusCode::SUCCESS;
  }
  return error( "Invalid iteration context." );
}
