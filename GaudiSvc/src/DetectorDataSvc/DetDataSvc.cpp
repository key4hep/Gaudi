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
#define DETECTORDATASVC_DETDATASVC_CPP

// Include files
#include "DetDataSvc.h"
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IAddressCreator.h>
#include <GaudiKernel/IConversionSvc.h>
#include <GaudiKernel/IOpaqueAddress.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/IValidity.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/System.h>
using System::getEnv;
using System::isEnvSet;

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( DetDataSvc )

// Service initialization
StatusCode DetDataSvc::initialize() {
  // Call base class initialization
  StatusCode sc = TsDataSvc::initialize();
  if ( sc.isFailure() ) return sc;

  // Set Data Loader
  auto cnv_svc = serviceLocator()->service<IConversionSvc>( m_persistencySvcName );
  if ( !cnv_svc ) {
    error() << "Unable to retrieve " << m_persistencySvcName << endmsg;
    return StatusCode::FAILURE;
  }

  sc = setDataLoader( cnv_svc );
  if ( sc.isFailure() ) {
    error() << "Unable to set DataLoader" << endmsg;
    return sc;
  }

  // Get address creator from the DetectorPersistencySvc
  m_addrCreator = cnv_svc;
  if ( !m_addrCreator ) {
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

  if ( m_usePersistency ) {

    IOpaqueAddress* rootAddr = nullptr;
    if ( m_detDbLocation.empty() || m_detDbLocation == "empty" ) {

      // if the name of DBlocation is not given - construct it!
      // by retrieving the value of XMLDDBROOT

      /// @todo: remove references to obsolete package XMLDDDBROOT
      if ( isEnvSet( "XMLDDDBROOT" ) ) {
        const std::string loc = getEnv( "XMLDDDBROOT" );
        m_detDbLocation       = loc + "/DDDB/lhcb.xml";
      }
    }
    if ( m_detDbLocation.empty() || m_detDbLocation == "empty" ) {
      error() << "Detector data location name not set. Detector data will "
                 "not be found."
              << endmsg;
      return StatusCode::FAILURE;
    } else {
      // Create address
      unsigned long     iargs[] = { 0, 0 };
      const std::string args[]  = { m_detDbLocation, m_detDbRootName };
      StatusCode        sc      = m_addrCreator->createAddress( m_detStorageType, CLID_Catalog, args, iargs, rootAddr );
      if ( sc.isSuccess() ) {
        sc = i_setRoot( rootAddr );
        if ( sc.isFailure() ) {
          error() << "Unable to set detector data store root" << endmsg;
          return sc;
        }
      } else {
        error() << "Unable to create address for  /dd" << endmsg;
        return sc;
      }
    }
    // Writing the description file in the output log file [bugs #2854]
    always() << "Detector description database: " << m_detDbLocation.value() << endmsg;
  } else {
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
  // StatusCode sc  = DataSvc::reinitialize();
  // if( sc.isFailure() ) return sc;

  // Delete the associated event time
  // if( 0 != m_eventTime ) delete m_eventTime;
  // m_eventTimeDefined = false;

  // return setupDetectorDescription();
  return StatusCode::SUCCESS;
}

/// Finalize the service.
StatusCode DetDataSvc::finalize() {
  DEBMSG << "Finalizing" << endmsg;

  // clears the store
  m_usePersistency = false; // avoid creation of an empty store when clearing
  clearStore().ignore();

  // Releases the address creator
  m_addrCreator = nullptr;

  // Releases the DataLoader
  setDataLoader( nullptr ).ignore();

  // Finalize the base class
  return TsDataSvc::finalize();
}

/// Remove all data objects in the data store.
StatusCode DetDataSvc::clearStore() {

  TsDataSvc::clearStore().ignore();

  if ( m_usePersistency ) {
    // Create root address
    unsigned long     iargs[] = { 0, 0 };
    const std::string args[]  = { m_detDbLocation, m_detDbRootName };
    IOpaqueAddress*   rootAddr;
    StatusCode        sc = m_addrCreator->createAddress( m_detStorageType, CLID_Catalog, args, iargs, rootAddr );
    // Set detector data store root
    if ( sc.isSuccess() ) {
      sc = i_setRoot( rootAddr );
      if ( sc.isFailure() ) { error() << "Unable to set detector data store root" << endmsg; }
    } else {
      error() << "Unable to create address for  /dd" << endmsg;
    }
    return sc;
  }
  return StatusCode::SUCCESS;
}

/// Standard Constructor
DetDataSvc::DetDataSvc( const std::string& name, ISvcLocator* svc ) : extends( name, svc ) {
  setProperty( "RootCLID", CLID_Catalog ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
}

/// Set the new event time
void DetDataSvc::setEventTime( const Gaudi::Time& time ) {
  m_eventTime = time;
  DEBMSG << "Event Time set to " << eventTime() << endmsg;
}

/// Check if the event time has been set
bool DetDataSvc::validEventTime() const { return true; }

/// Get the event time
const Gaudi::Time& DetDataSvc::eventTime() const { return m_eventTime; }

/// Inform that a new incident has occurred
void DetDataSvc::handle( const Incident& inc ) {
  ON_DEBUG {
    debug() << "New incident received" << endmsg;
    debug() << "Incident source: " << inc.source() << endmsg;
    debug() << "Incident type: " << inc.type() << endmsg;
  }
}

/// Update object
/// @todo update also its ancestors in the data store if necessary
StatusCode DetDataSvc::updateObject( DataObject* toUpdate ) {

  DEBMSG << "Method updateObject starting" << endmsg;

  // Check that object to update exists
  if ( !toUpdate ) {
    error() << "There is no DataObject to update" << endmsg;
    return Status::INVALID_OBJECT;
  }

  // Retrieve IValidity interface of object to update
  IValidity* condition = dynamic_cast<IValidity*>( toUpdate );
  if ( !condition ) {
    warning() << "Cannot update DataObject: DataObject does not implement IValidity" << endmsg;
    return StatusCode::SUCCESS;
  }

  // Check that the event time has been defined
  if ( !validEventTime() ) {
    warning() << "Cannot update DataObject: event time undefined" << endmsg;
    return StatusCode::SUCCESS;
  }

  // No need to update if condition is valid
  if ( condition->isValid( eventTime() ) ) {
    DEBMSG << "DataObject is valid: no need to update" << endmsg;
    return StatusCode::SUCCESS;
  }

  DEBMSG << "DataObject is invalid: update it" << endmsg;
  // TODO: before loading updated object, update HERE its parent in data store

  // Now delegate update to the conversion service by calling the base class
  DEBMSG << "Delegate update to relevant conversion service" << endmsg;
  StatusCode status = TsDataSvc::updateObject( toUpdate );
  if ( !status.isSuccess() ) {
    error() << "Could not update DataObject" << endmsg;
    if ( status == Status::NO_DATA_LOADER ) error() << "There is no data loader" << endmsg;
    return status;
  }

  // Now cross-check that the new condition is valid
  condition = dynamic_cast<IValidity*>( toUpdate );
  if ( !condition ) {
    error() << "Updated DataObject does not implement IValidity" << endmsg;
    return StatusCode::FAILURE;
  }
  if ( FSMState() == Gaudi::StateMachine::RUNNING && !condition->isValid( eventTime() ) ) {
    error() << "Updated DataObject is not valid" << endmsg;
    error() << "Are you sure the conversion service has updated it?" << endmsg;
    return StatusCode::FAILURE;
  }

  // DataObject was successfully updated
  DEBMSG << "Method updateObject exiting successfully" << endmsg;
  return StatusCode::SUCCESS;
}
