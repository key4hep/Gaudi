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
// Include Files
#include "AuditorSvc.h"
#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IAuditor.h"
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/TypeNameString.h"

// Instantiation of a static factory class used by clients to create
//  instances of this service
DECLARE_COMPONENT( AuditorSvc )

//
// ClassName:   AuditorSvc
//
// Description: This service manages Auditors.
//------------------------------------------------------------------

//- private helpers ---
SmartIF<IAuditor> AuditorSvc::newAuditor_( MsgStream& log, std::string_view name ) {
  // locate the auditor factory, instantiate a new auditor, initialize it
  StatusCode                   sc;
  Gaudi::Utils::TypeNameString item( name );
  SmartIF<IAuditor> aud{Auditor::Factory::create( item.type(), item.name(), serviceLocator().get() ).release()};
  if ( aud ) {
    if ( m_targetState >= Gaudi::StateMachine::INITIALIZED ) {
      sc = aud->sysInitialize();
      if ( sc.isFailure() ) {
        log << MSG::WARNING << "Failed to initialize Auditor " << name << endmsg;
        aud.reset();
      }
    }
  } else {
    log << MSG::WARNING << "Unable to retrieve factory for Auditor " << name << endmsg;
  }
  return aud;
}

SmartIF<IAuditor> AuditorSvc::findAuditor_( std::string_view name ) {
  // find an auditor by name, return 0 on error
  auto it = std::find_if( std::begin( m_pAudList ), std::end( m_pAudList ),
                          [item_name = Gaudi::Utils::TypeNameString( name ).name()]( const IAuditor* i ) {
                            return i->name() == item_name;
                          } );
  return SmartIF<IAuditor>{it != std::end( m_pAudList ) ? *it : nullptr};
}

StatusCode AuditorSvc::syncAuditors_() {
  if ( m_audNameList.size() == m_pAudList.size() ) return StatusCode::SUCCESS;

  StatusCode sc;

  //   if ( sc.isFailure() ) {
  //     error() << "Unable to locate ObjectManager Service" << endmsg;
  //     return sc;
  //   }

  // create all declared Auditors that do not yet exist
  for ( auto& it : m_audNameList ) {

    // this is clumsy, but the PropertyHolder won't tell us when my property changes right
    // under my nose, so I'll have to figure this out the hard way
    if ( !findAuditor_( it ) ) { // if auditor does not yet exist
      auto aud = newAuditor_( msgStream(), it );
      if ( aud ) {
        m_pAudList.push_back( std::move( aud ) );
      } else {
        error() << "Error constructing Auditor " << it << endmsg;
        sc = StatusCode::FAILURE;
      }
    }
  }
  return sc;
}

// Inherited Service overrides:
//
// Initialize the service.
StatusCode AuditorSvc::initialize() {
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) return sc;

  // create auditor objects for all named auditors
  sc = syncAuditors_();

  return sc;
}

// Finalise the service.
StatusCode AuditorSvc::finalize() {

  for ( auto& it : m_pAudList ) {
    if ( it->isEnabled() ) it->sysFinalize().ignore();
  }
  m_pAudList.clear();

  // Finalize this specific service
  return Service::finalize();
}

// --------- "Before" methods ---------
void AuditorSvc::before( StandardEventType evt, INamedInterface* obj ) {
  if ( !isEnabled() ) return;
  for ( auto& it : m_pAudList ) {
    if ( it->isEnabled() ) it->before( evt, obj );
  }
}

void AuditorSvc::before( StandardEventType evt, const std::string& name ) {
  if ( !isEnabled() ) return;
  for ( auto& it : m_pAudList ) {
    if ( it->isEnabled() ) it->before( evt, name );
  }
}

void AuditorSvc::before( CustomEventTypeRef evt, INamedInterface* obj ) {
  if ( !isEnabled() ) return;
  for ( auto& it : m_pAudList ) {
    if ( it->isEnabled() ) it->before( evt, obj );
  }
}

void AuditorSvc::before( CustomEventTypeRef evt, const std::string& name ) {
  if ( !isEnabled() ) return;
  for ( auto& it : m_pAudList ) {
    if ( it->isEnabled() ) it->before( evt, name );
  }
}

// --------- "After" methods ---------
void AuditorSvc::after( StandardEventType evt, INamedInterface* obj, const StatusCode& sc ) {
  if ( !isEnabled() ) return;
  for ( auto& it : m_pAudList ) {
    if ( it->isEnabled() ) it->after( evt, obj, sc );
  }
}

void AuditorSvc::after( StandardEventType evt, const std::string& name, const StatusCode& sc ) {
  if ( !isEnabled() ) return;
  for ( auto& it : m_pAudList ) {
    if ( it->isEnabled() ) it->after( evt, name, sc );
  }
}

void AuditorSvc::after( CustomEventTypeRef evt, INamedInterface* obj, const StatusCode& sc ) {
  if ( !isEnabled() ) return;
  for ( auto& it : m_pAudList ) {
    if ( it->isEnabled() ) it->after( evt, obj, sc );
  }
}

void AuditorSvc::after( CustomEventTypeRef evt, const std::string& name, const StatusCode& sc ) {
  if ( !isEnabled() ) return;
  for ( auto& it : m_pAudList ) {
    if ( it->isEnabled() ) it->after( evt, name, sc );
  }
}

// --------- obsolete methods ---------
#define OBSOLETION( name )                                                                                             \
  void AuditorSvc::name( INamedInterface* ) {                                                                          \
    throw GaudiException( "The method IAuditor::" #name " is obsolete do not call it.", "AuditorSvc::" #name,          \
                          StatusCode::FAILURE );                                                                       \
  }

OBSOLETION( beforeInitialize )
OBSOLETION( afterInitialize )

OBSOLETION( beforeReinitialize )
OBSOLETION( afterReinitialize )

OBSOLETION( beforeExecute )
void AuditorSvc::afterExecute( INamedInterface*, const StatusCode& ) {
  throw GaudiException( "The method afterExecute is obsolete do not call it.", "AuditorSvc::afterExecute",
                        StatusCode::FAILURE );
}

OBSOLETION( beforeFinalize )
OBSOLETION( afterFinalize )

bool AuditorSvc::isEnabled() const { return m_isEnabled; }

StatusCode AuditorSvc::sysInitialize() { return Service::sysInitialize(); }
StatusCode AuditorSvc::sysFinalize() { return Service::sysFinalize(); }

IAuditor* AuditorSvc::getAuditor( const std::string& name ) {
  // by interactively setting properties, auditors might be out of sync
  if ( !syncAuditors_().isSuccess() ) {
    // as we didn't manage to sync auditors, the safest bet is to assume the
    // worse...
    // So don't let clients play with an AuditorSvc in an inconsistent state
    return nullptr;
  }

  // search available auditors, returns 0 on error
  return findAuditor_( name );
}
