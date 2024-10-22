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
//	RecordDataSvc.cpp
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: implementation of the Transient event data service.
//
//  Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 10/12/08| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define DATASVC_RECORDDATASVC_CPP

#include <GaudiKernel/DataIncident.h>
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IConversionSvc.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/IOpaqueAddress.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/RegistryEntry.h>
#include <GaudiKernel/SmartIF.h>

#include "RecordDataSvc.h"
using namespace std;

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT( RecordDataSvc )

/// Service initialisation
StatusCode RecordDataSvc::initialize() {
  // Nothing to do: just call base class initialisation
  StatusCode sc = DataSvc::initialize();

  if ( !sc.isSuccess() ) { // Base class failure
    return sc;
  }
  // Attach data loader facility
  m_cnvSvc = service( m_persSvcName, true );
  if ( !m_cnvSvc ) {
    error() << "Failed to access RecordPersistencySvc." << endmsg;
    return StatusCode::FAILURE;
  }
  auto prp = m_cnvSvc.as<IProperty>();
  if ( prp ) {
    // prp->setProperty();
  }
  sc = setDataLoader( m_cnvSvc.get() );
  if ( !sc.isSuccess() ) {
    error() << "Failed to attach dataloader RecordPersistencySvc." << endmsg;
    return sc;
  }

  sc = setRoot( m_rootName, new DataObject() );
  if ( !sc.isSuccess() ) {
    warning() << "Error declaring Record root DataObject" << endmsg;
    return sc;
  }

  if ( !m_incidentSvc ) {
    fatal() << "IncidentSvc is invalid--base class failed." << endmsg;
    return sc;
  }

  /// We need to subscribe to 2 incident:
  /// 1) FILE_OPEN_READ:            fired by conversion service on open file
  /// 2) IncidentType::BeginEvent   fired by event loop BEFORE the event processing starts.
  ///                               Do everything to bootstract access to the old event record.
  m_incidentSvc->addListener( this, "FILE_OPEN_READ" );
  m_incidentSvc->addListener( this, m_saveIncidentName );
  return sc;
}

/// Service finalization
StatusCode RecordDataSvc::finalize() {
  if ( m_incidentSvc ) m_incidentSvc->removeListener( this );
  m_cnvSvc.reset();
  DataSvc::finalize().ignore();
  return StatusCode::SUCCESS;
}

/// Inform that a new incident has occured
void RecordDataSvc::handle( const Incident& incident ) {
  if ( incident.type() == "FILE_OPEN_READ" ) {
    typedef ContextIncident<IOpaqueAddress*> Ctxt;
    auto                                     inc = dynamic_cast<const Ctxt*>( &incident );
    if ( !inc ) {
      always() << "Received invalid incident of type:" << incident.type() << endmsg;
    } else {
      registerRecord( inc->source(), inc->tag() );
      if ( !m_incidentName.empty() ) {
        auto incidents = m_incidents;
        m_incidents.clear();
        for ( const auto& i : incidents ) m_incidentSvc->fireIncident( Incident{ i, m_incidentName } );
      }
    }
  } else if ( m_saveIncidentName == incident.type() ) {
    always() << "Saving records not implemented." << endmsg;
  }
}

/// Load dependent records into memory
void RecordDataSvc::loadRecords( IRegistry* pObj ) {
  if ( !pObj ) {
    error() << "Failed to load records object" << endmsg;
  } else {
    vector<IRegistry*> leaves;
    DataObject*        p   = nullptr;
    const string&      id0 = pObj->identifier();
    StatusCode         sc  = retrieveObject( id0, p );
    if ( sc.isSuccess() ) {
      debug() << "Loaded records object: " << id0 << endmsg;
      sc = objectLeaves( pObj, leaves );
      if ( sc.isSuccess() ) {
        for ( const auto& i : leaves ) loadRecords( i );
      }
    }
  }
}

/// Load new run record into the data store if necessary
void RecordDataSvc::registerRecord( const string& data, IOpaqueAddress* pAddr ) {
  if ( !data.empty() && pAddr ) {
    string fid = data;
    debug() << "Request to load record for file " << fid << endmsg;
    StatusCode sc = registerAddress( m_root.get(), fid, pAddr );
    if ( !sc.isSuccess() ) {
      warning() << "Failed to register record for:" << fid << endmsg;
      pAddr->release();
      return;
    }
    if ( m_autoLoad ) { loadRecords( pAddr->registry() ); }
    m_incidents.push_back( pAddr->registry()->identifier() );
  } else if ( !data.empty() && !pAddr ) {
    info() << "Failed to register record for:" << data << " [Invalid Address]" << endmsg;
  }
}

/// Standard Constructor
RecordDataSvc::RecordDataSvc( const string& name, ISvcLocator* svc ) : base_class( name, svc ) {
  m_rootName = "/Records";
}
