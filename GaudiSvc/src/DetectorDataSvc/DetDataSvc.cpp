// $Id: DetDataSvc.cpp,v 1.24 2008/10/27 19:22:21 marcocle Exp $
#define  DETECTORDATASVC_DETDATASVC_CPP

// Include files
#include "DetDataSvc.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IValidity.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/System.h"
using System::isEnvSet;
using System::getEnv;

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_SERVICE_FACTORY(DetDataSvc)

// Service initialization
StatusCode DetDataSvc::initialize()   {
  MsgStream log(msgSvc(), name());

  // Call base class initialization
  StatusCode sc  = DataSvc::initialize();
  if( sc.isFailure() ) return sc;

  // Set Data Loader
  SmartIF<IConversionSvc> cnv_svc(serviceLocator()->service(m_persistencySvcName));
  if( !cnv_svc.isValid() ) {
    log << MSG::ERROR << "Unable to retrieve " << m_persistencySvcName << endmsg;
    return StatusCode::FAILURE;
  }

  sc = setDataLoader(cnv_svc);
  if( sc.isFailure() ) {
    log << MSG::ERROR << "Unable to set DataLoader" << endmsg;
    return sc;
  }

  // Get address creator from the DetectorPersistencySvc
  m_addrCreator = cnv_svc;
  if (!m_addrCreator.isValid()) {
    log << MSG::ERROR << "Unable to get AddressCreator." << endmsg;
    return StatusCode::FAILURE;
  }

  return setupDetectorDescription();
}

StatusCode DetDataSvc::setupDetectorDescription() {
  // Now you can use the MsgSvc
  MsgStream log(msgSvc(), name());

  // Initialize the detector data transient store
  log << MSG::DEBUG << "Storage type used is: " << m_detStorageType << endmsg;
  log << MSG::DEBUG << "Setting DetectorDataSvc root node... " << endmsg;

  if( m_usePersistency ) {

    IOpaqueAddress* rootAddr;
    if( m_detDbLocation.empty() || "empty" == m_detDbLocation ) {

      // if the name of DBlocation is not given - construct it!
	  // by retrieving the value of XMLDDBROOT

      /// @todo: remove references to obsolete package XMLDDDBROOT
      if ( isEnvSet("XMLDDDBROOT") ) {
        m_detDbLocation  = getEnv("XMLDDDBROOT");
        m_detDbLocation += "/DDDB/lhcb.xml";
      }
    }
    if( m_detDbLocation.empty() || "empty" == m_detDbLocation ) {
      log << MSG::ERROR
          << "Detector data location name not set. Detector data will "
          << "not be found." << endmsg;
      return StatusCode::FAILURE;
    }
    else {
      // Create address
      unsigned long iargs[]={0,0};
      const std::string args[] = {m_detDbLocation, m_detDbRootName};
      StatusCode sc = m_addrCreator->createAddress (m_detStorageType,
                                                    CLID_Catalog,
                                                    args,
                                                    iargs,
                                                    rootAddr);
      if( sc.isSuccess() ) {
        std::string dbrName = "/" + m_detDbRootName;
        sc = i_setRoot( dbrName, rootAddr );
        if( sc.isFailure() ) {
          log << MSG::ERROR << "Unable to set detector data store root"
              << endmsg;
          return sc;
        }
      }
      else {
        log << MSG::ERROR << "Unable to create address for  /dd" << endmsg;
        return sc;
      }
    }
    // Writing the description file in the output log file [bugs #2854]
    log << MSG::ALWAYS << "Detector description database: " << m_detDbLocation << endmsg;
  }
  else {
    log << MSG::INFO << "Detector description not requested to be loaded"
        << endmsg;
  }

  return StatusCode::SUCCESS;
}

// Service initialisation
StatusCode DetDataSvc::reinitialize()   {
  MsgStream log(msgSvc(), name());

  // The DetectorDataSvc does not need to be re-initialized. If it is done
  // all the Algorithms having references to DetectorElements will become
  // invalid and crash the program.  (Pere Mato)

  // Call base class reinitialization
  //StatusCode sc  = DataSvc::reinitialize();
  //if( sc.isFailure() ) return sc;

  // Delete the associated event time
  //if( 0 != m_eventTime ) delete m_eventTime;
  //m_eventTimeDefined = false;

  //return setupDetectorDescription();
  return StatusCode::SUCCESS;
}

/// Finalize the service.
StatusCode DetDataSvc::finalize()
{
  MsgStream log(msgSvc(), name());
  log << MSG::DEBUG << "Finalizing" << endmsg;

  // clears the store
  m_usePersistency = false; // avoid creation of an empty store when clearing
  clearStore().ignore();

  // Releases the address creator
  m_addrCreator = 0;

  // Releases the DataLoader
  setDataLoader(0).ignore();

  // Finalize the base class
  return DataSvc::finalize();
}

/// Remove all data objects in the data store.
StatusCode DetDataSvc::clearStore()   {

  MsgStream log(msgSvc(), name());

  DataSvc::clearStore().ignore();

  if( m_usePersistency ) {
    // Create root address
    unsigned long iargs[]    = {0,0};
    const std::string args[] = {m_detDbLocation, m_detDbRootName};
    IOpaqueAddress*   rootAddr;
    StatusCode sc = m_addrCreator->createAddress (m_detStorageType,
                                                  CLID_Catalog,
                                                  args,
                                                  iargs,
                                                  rootAddr);
    // Set detector data store root
    if( sc.isSuccess() ) {
      std::string dbrName = "/" + m_detDbRootName;
      sc = i_setRoot( dbrName, rootAddr );
      if( sc.isFailure() ) {
        log << MSG::ERROR
	    << "Unable to set detector data store root" << endmsg;
      }
    } else {
      log << MSG::ERROR << "Unable to create address for  /dd" << endmsg;
    }
    return sc;

  }
  return StatusCode::SUCCESS;

}

/// Standard Constructor
DetDataSvc::DetDataSvc(const std::string& name,ISvcLocator* svc) :
  base_class(name,svc), m_eventTime(0)  {
  declareProperty("DetStorageType",  m_detStorageType = XML_StorageType );
  declareProperty("DetDbLocation",   m_detDbLocation  = "empty" );
  declareProperty("DetDbRootName",   m_detDbRootName  = "dd" );
  declareProperty("UsePersistency",  m_usePersistency = false );
  declareProperty("PersistencySvc",  m_persistencySvcName = "DetectorPersistencySvc" );
  m_rootName = "/dd";
  m_rootCLID = CLID_Catalog;
  m_addrCreator = 0;
}

/// Standard Destructor
DetDataSvc::~DetDataSvc()  {
}

/// Set the new event time
void DetDataSvc::setEventTime ( const Gaudi::Time& time ) {
  m_eventTime = time;
  if ( msgSvc()->outputLevel() <= MSG::DEBUG) {
    MsgStream log( msgSvc(), name() );
    log << MSG::DEBUG << "Event Time set to " << eventTime() << endmsg;
  }
}

/// Check if the event time has been set
bool DetDataSvc::validEventTime ( ) const {
  return true;
}

/// Get the event time
const Gaudi::Time& DetDataSvc::eventTime ( ) const {
  return m_eventTime;
}

/// Inform that a new incident has occured
void DetDataSvc::handle ( const Incident& inc ) {
  MsgStream log( msgSvc(), name() );
  log << MSG::DEBUG << "New incident received" << endmsg;
  log << MSG::DEBUG << "Incident source: " << inc.source() << endmsg;
  log << MSG::DEBUG << "Incident type: " << inc.type() << endmsg;
  return;
}

/// Update object
/// @todo update also its ancestors in the data store if necessary
StatusCode DetDataSvc::updateObject( DataObject* toUpdate ) {

  MsgStream log( msgSvc(), name() );
  log << MSG::DEBUG << "Method updateObject starting" << endmsg;

  // Check that object to update exists
  if ( 0 == toUpdate ) {
    log << MSG::ERROR
	<< "There is no DataObject to update" << endmsg;
    return INVALID_OBJECT;
  }

  // Retrieve IValidity interface of object to update
  IValidity* condition = dynamic_cast<IValidity*>( toUpdate );
  if ( 0 == condition ) {
    log << MSG::WARNING
	<< "Cannot update DataObject: DataObject does not implement IValidity"
	<< endmsg;
    return StatusCode::SUCCESS;
  }

  // Check that the event time has been defined
  if ( !validEventTime() ) {
    log << MSG::WARNING
	<< "Cannot update DataObject: event time undefined"
	<< endmsg;
    return StatusCode::SUCCESS;
  }

  // No need to update if condition is valid
  if ( condition->isValid( eventTime() ) ) {
    log << MSG::DEBUG
	<< "DataObject is valid: no need to update" << endmsg;
    return StatusCode::SUCCESS;
  } else {
    log << MSG::DEBUG
	<< "DataObject is invalid: update it" << endmsg;
  }

  // TODO: before loading updated object, update HERE its parent in data store

  // Now delegate update to the conversion service by calling the base class
  log << MSG::DEBUG
      << "Delegate update to relevant conversion service" << endmsg;
  StatusCode status = DataSvc::updateObject(toUpdate);
  if ( !status.isSuccess() ) {
    log << MSG::ERROR
	<< "Could not update DataObject" << endmsg;
    if ( status == NO_DATA_LOADER )
      log << MSG::ERROR << "There is no data loader" << endmsg;
    return status;
  }

  // Now cross-check that the new condition is valid
  condition = dynamic_cast<IValidity*>(toUpdate);
  if ( 0 == condition ) {
    log << MSG::ERROR
	<< "Updated DataObject does not implement IValidity" << endmsg;
    return StatusCode::FAILURE;
  }
  if ( FSMState() == Gaudi::StateMachine::RUNNING &&
       !condition->isValid( eventTime() ) ) {
    log << MSG::ERROR
	<< "Updated DataObject is not valid" << endmsg;
    log << MSG::ERROR
	<< "Are you sure the conversion service has updated it?" << endmsg;
    return StatusCode::FAILURE;
  }

  // DataObject was successfully updated
  log << MSG::DEBUG << "Method updateObject exiting successfully" << endmsg;
  return StatusCode::SUCCESS;

}
