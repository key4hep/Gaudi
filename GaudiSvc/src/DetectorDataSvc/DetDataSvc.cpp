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
#include "GaudiKernel/System.h"
using System::isEnvSet;
using System::getEnv;

#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT(DetDataSvc)

// Service initialization
StatusCode DetDataSvc::initialize()   {
  // Call base class initialization
  StatusCode sc  = DataSvc::initialize();
  if( UNLIKELY(sc.isFailure()) ) return sc;

  // Set Data Loader
  SmartIF<IConversionSvc> cnv_svc(serviceLocator()->service(m_persistencySvcName));
  if( UNLIKELY(!cnv_svc.isValid()) ) {
    error() << "Unable to retrieve " << m_persistencySvcName << endmsg;
    return StatusCode::FAILURE;
  }

  sc = setDataLoader(cnv_svc);
  if( UNLIKELY(sc.isFailure()) ) {
    error() << "Unable to set DataLoader" << endmsg;
    return sc;
  }

  // Get address creator from the DetectorPersistencySvc
  m_addrCreator = cnv_svc;
  if ( UNLIKELY(!m_addrCreator.isValid()) ) {
    error() << "Unable to get AddressCreator." << endmsg;
    return StatusCode::FAILURE;
  }

  return setupDetectorDescription();
}

StatusCode DetDataSvc::setupDetectorDescription() {
  // Initialize the detector data transient store
  ON_DEBUG {
    debug() << "Storage type used is: " << m_detStorageType << endmsg;
    debug() << "Setting DetectorDataSvc root node... " << endmsg;
  }

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
      error() << "Detector data location name not set. Detector data will "
                 "not be found." << endmsg;
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
          error() << "Unable to set detector data store root" << endmsg;
          return sc;
        }
      }
      else {
        error() << "Unable to create address for  /dd" << endmsg;
        return sc;
      }
    }
    // Writing the description file in the output log file [bugs #2854]
    always() << "Detector description database: " << m_detDbLocation << endmsg;
  }
  else {
    info() << "Detector description not requested to be loaded" << endmsg;
  }

  return StatusCode::SUCCESS;
}

// Service initialisation
StatusCode DetDataSvc::reinitialize() {
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
  DEBMSG << "Finalizing" << endmsg;

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
        error() << "Unable to set detector data store root" << endmsg;
      }
    } else {
      error() << "Unable to create address for  /dd" << endmsg;
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
  DEBMSG << "Event Time set to " << eventTime() << endmsg;
}

/// Check if the event time has been set
bool DetDataSvc::validEventTime ( ) const {
  return true;
}

/// Get the event time
const Gaudi::Time& DetDataSvc::eventTime ( ) const {
  return m_eventTime;
}

/// Inform that a new incident has occurred
void DetDataSvc::handle ( const Incident& inc ) {
  ON_DEBUG {
    debug() << "New incident received" << endmsg;
    debug() << "Incident source: " << inc.source() << endmsg;
    debug() << "Incident type: " << inc.type() << endmsg;
  }
  return;
}

/// Update object
/// @todo update also its ancestors in the data store if necessary
StatusCode DetDataSvc::updateObject( DataObject* toUpdate ) {

  DEBMSG << "Method updateObject starting" << endmsg;

  // Check that object to update exists
  if ( 0 == toUpdate ) {
    error() << "There is no DataObject to update" << endmsg;
    return INVALID_OBJECT;
  }

  // Retrieve IValidity interface of object to update
  IValidity* condition = dynamic_cast<IValidity*>( toUpdate );
  if ( 0 == condition ) {
    warning()
	<< "Cannot update DataObject: DataObject does not implement IValidity"
	<< endmsg;
    return StatusCode::SUCCESS;
  }

  // Check that the event time has been defined
  if ( !validEventTime() ) {
    warning()
	<< "Cannot update DataObject: event time undefined"
	<< endmsg;
    return StatusCode::SUCCESS;
  }

  // No need to update if condition is valid
  if ( condition->isValid( eventTime() ) ) {
    DEBMSG << "DataObject is valid: no need to update" << endmsg;
    return StatusCode::SUCCESS;
  } else {
    DEBMSG << "DataObject is invalid: update it" << endmsg;
  }

  // TODO: before loading updated object, update HERE its parent in data store

  // Now delegate update to the conversion service by calling the base class
  DEBMSG << "Delegate update to relevant conversion service" << endmsg;
  StatusCode status = DataSvc::updateObject(toUpdate);
  if ( !status.isSuccess() ) {
    error() << "Could not update DataObject" << endmsg;
    if ( status == NO_DATA_LOADER )
      error() << "There is no data loader" << endmsg;
    return status;
  }

  // Now cross-check that the new condition is valid
  condition = dynamic_cast<IValidity*>(toUpdate);
  if ( 0 == condition ) {
    error() << "Updated DataObject does not implement IValidity" << endmsg;
    return StatusCode::FAILURE;
  }
  if ( FSMState() == Gaudi::StateMachine::RUNNING &&
       !condition->isValid( eventTime() ) ) {
    error() << "Updated DataObject is not valid" << endmsg;
    error() << "Are you sure the conversion service has updated it?" << endmsg;
    return StatusCode::FAILURE;
  }

  // DataObject was successfully updated
  DEBMSG << "Method updateObject exiting successfully" << endmsg;
  return StatusCode::SUCCESS;

}
