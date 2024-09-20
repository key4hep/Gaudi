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
// Include files
#include <GaudiKernel/IIncidentListener.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/Service.h>
#include <climits>
#include <iostream>

// local
#include "JemallocProfileSvc.h"

// including jemmalloc.h is difficult as the malloc signature is not exactly identical
// to the system one (issue with throw).
// We therefore declare mallctl here.
extern "C" {
int mallctl( const char* name, void* oldp, size_t* oldlenp, void* newp, size_t newlen );
}

//-----------------------------------------------------------------------------
// Implementation file for class : JemallocProfileSvc
//
// 2016-01-11 : Ben Couturier
//-----------------------------------------------------------------------------

//=============================================================================
// Initializer
//=============================================================================
StatusCode JemallocProfileSvc::initialize() {
  StatusCode sc = base_class::initialize();
  if ( sc.isFailure() ) return sc;

  // register to the incident service
  static const std::string serviceName = "IncidentSvc";
  m_incidentSvc                        = serviceLocator()->service( serviceName );
  if ( !m_incidentSvc ) {
    error() << "Cannot retrieve " << serviceName << endmsg;
    return StatusCode::FAILURE;
  }

  debug() << "Register to the IncidentSvc" << endmsg;
  m_incidentSvc->addListener( this, IncidentType::BeginEvent );
  m_incidentSvc->addListener( this, IncidentType::EndEvent );
  for ( std::string incident : m_startFromIncidents ) { m_incidentSvc->addListener( this, incident ); }
  for ( std::string incident : m_stopAtIncidents ) { m_incidentSvc->addListener( this, incident ); }

  // Resetting the event counter
  m_eventNumber = 0;
  m_profiling   = false;

  // Cache whether we have start/stop incidents
  m_hasStartIncident = ( m_startFromIncidents.size() > 0 );
  m_hasStopIncident  = ( m_stopAtIncidents.size() > 0 );

  // Checking the consistency of the start/stop events
  if ( m_nStartFromEvent == 0 && !m_hasStartIncident && ( m_hasStopIncident || m_nStopAtEvent > 0 ) ) {
    info() << "Stop profiling trigger was specified but no start. Defaulting to first events" << endmsg;
    m_nStartFromEvent = 1;
  }

  return StatusCode::SUCCESS;
}

// Finalization of the service.
StatusCode JemallocProfileSvc::finalize() {
  if ( m_profiling ) { stopProfiling(); }
  // unregistering from the IncidentSvc
  m_incidentSvc->removeListener( this, IncidentType::BeginEvent );
  m_incidentSvc->removeListener( this, IncidentType::EndEvent );
  m_incidentSvc.reset();
  return base_class::finalize();
}

//=============================================================================
// Event handling methods

//=============================================================================

// Handler for incidents
void JemallocProfileSvc::handle( const Incident& incident ) {
  if ( IncidentType::BeginEvent == incident.type() ) {
    handleBegin();
  } else if ( IncidentType::EndEvent == incident.type() ) {
    handleEnd();
  }

  // If already processing we can ignore the incidents for start
  if ( !m_profiling && m_hasStartIncident ) {
    for ( std::string startincident : m_startFromIncidents ) {
      if ( startincident == incident.type() ) {
        info() << "Starting Jemalloc profile at incident " << incident.type() << endmsg;
        startProfiling();
        break;
      }
    } // Loop on incidents
  }   // If checking incidents to start

  // If already processing we can ignore the incidents for start
  if ( m_profiling && m_hasStopIncident ) {
    for ( std::string stopincident : m_stopAtIncidents ) {
      if ( stopincident == incident.type() ) {
        info() << "Stopping Jemalloc profile at incident " << incident.type() << endmsg;
        stopProfiling();
        break;
      }
    } // Loop on incidents
  }   // If checking incidents to stop
}

// Handler for incidents
// Called on at begin events
inline void JemallocProfileSvc::handleBegin() {
  m_eventNumber += 1;

  if ( m_eventNumber == m_nStartFromEvent ) { startProfiling(); }
}

// Handler for incidents
// Called on at End events
inline void JemallocProfileSvc::handleEnd() {
  if ( m_profiling && m_eventNumber != m_nStartFromEvent &&
       ( ( m_eventNumber - m_nStartFromEvent ) % m_dumpPeriod == 0 ) ) {
    dumpProfile();
  }

  if ( m_eventNumber == m_nStopAtEvent ) { stopProfiling(); }
}

//=============================================================================
// Utilities calling mallctl

//=============================================================================

/**
 * Utility method to start profiling with jemalloc
 */
inline void JemallocProfileSvc::startProfiling() {
  m_profiling = true;
  info() << "Starting Jemalloc profile at event " << m_eventNumber << endmsg;
  mallctl( "prof.dump", NULL, NULL, NULL, 0 );
}

/**
 * Utility method to stop profiling with jemalloc
 */
inline void JemallocProfileSvc::stopProfiling() {
  m_profiling = false;
  dumpProfile();
}

/**
 * Utility method to dump profile with jemalloc
 */
inline void JemallocProfileSvc::dumpProfile() {
  info() << "Dumping Jemalloc profile at event " << m_eventNumber << endmsg;
  mallctl( "prof.dump", NULL, NULL, NULL, 0 );
}

//=============================================================================
// Declaration of the factory
DECLARE_COMPONENT( JemallocProfileSvc )
