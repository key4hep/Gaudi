/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
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
#include <Gaudi/Auditor.h>
#include <Gaudi/IAuditor.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/INamedInterface.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/TypeNameString.h>

// Instantiation of a static factory class used by clients to create
//  instances of this service
DECLARE_COMPONENT( AuditorSvc )

std::unique_ptr<Gaudi::IAuditor> AuditorSvc::newAuditor( MsgStream& log, std::string_view name ) {
  // locate the auditor factory, instantiate a new auditor, initialize it
  Gaudi::Utils::TypeNameString     item( name );
  std::unique_ptr<Gaudi::IAuditor> aud{
      Gaudi::Auditor::Factory::create( item.type(), item.name(), serviceLocator().get() ) };
  if ( aud ) {
    // make sure we increase the internal reference counter used by SmartIF or any usage of it
    // somewhere else will delete the Auditor in our back !
    // note that release is never called, as we handle the ownership via unique_ptr and do not
    // rely on Gaudi's legacy black magic
    aud->addRef();
    if ( m_targetState >= Gaudi::StateMachine::INITIALIZED && aud->sysInitialize().isFailure() ) {
      log << MSG::WARNING << "Failed to initialize Auditor " << name << endmsg;
      aud.reset();
    }
  } else {
    log << MSG::WARNING << "Unable to retrieve factory for Auditor " << name << endmsg;
  }
  return aud;
}

Gaudi::IAuditor* AuditorSvc::getAuditor( const std::string& name ) const {
  // find an auditor by name, return 0 on error
  auto it = std::find_if(
      std::begin( m_pAudList ), std::end( m_pAudList ),
      [item_name = Gaudi::Utils::TypeNameString( name ).name()]( auto const& i ) { return i->name() == item_name; } );
  return it != std::end( m_pAudList ) ? it->get() : nullptr;
}

StatusCode AuditorSvc::addAuditor( std::string const& name ) {
  // this is clumsy, but the PropertyHolder won't tell us when my property changes right
  // under my nose, so I'll have to figure this out the hard way
  if ( !hasAuditor( name ) ) { // if auditor does not yet exist
    auto aud = newAuditor( msgStream(), name );
    if ( aud ) {
      m_pAudList.push_back( std::move( aud ) );
    } else {
      error() << "Error constructing Auditor " << name << endmsg;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

std::optional<StatusCode> AuditorSvc::removesAuditor( std::string const& name ) {
  auto it = std::find_if(
      std::begin( m_pAudList ), std::end( m_pAudList ),
      [item_name = Gaudi::Utils::TypeNameString( name ).name()]( auto const& i ) { return i->name() == item_name; } );
  if ( it != std::end( m_pAudList ) ) {
    auto sc = ( *it )->sysFinalize();
    if ( sc.isFailure() ) { error() << "Finalization of auditor " << name << " failed : " << sc << endmsg; }
    m_pAudList.erase( it );
    return sc;
  }
  return {};
}

StatusCode AuditorSvc::syncAuditors() {
  // drop all existing Auditors no more declared (and finalize them in case)
  auto pastEnd = std::remove_if( begin( m_pAudList ), end( m_pAudList ), [this]( auto& entry ) -> bool {
    return std::find( begin( m_audNameList ), end( m_audNameList ), entry->name() ) != m_audNameList.end();
  } );
  std::for_each( pastEnd, m_pAudList.end(), [this]( auto& entry ) {
    if ( entry->isEnabled() ) {
      auto sc = entry->sysFinalize();
      if ( sc.isFailure() ) { error() << "Finalization of auditor " << entry->name() << " failed : " << sc << endmsg; }
    }
  } );
  m_pAudList.erase( pastEnd, m_pAudList.end() );
  // create all newly declared Auditors
  StatusCode sc = StatusCode::SUCCESS;
  for ( auto& it : m_audNameList ) {
    if ( addAuditor( it ).isFailure() ) sc = StatusCode::FAILURE;
  }
  return sc;
}

StatusCode AuditorSvc::finalize() {
  for ( auto& it : m_pAudList ) {
    if ( it->isEnabled() ) it->sysFinalize().ignore();
  }
  m_pAudList.clear();
  return Service::finalize();
}

void AuditorSvc::before( std::string const& evt, const std::string& name, EventContext const& context ) {
  if ( !isEnabled() ) return;
  for ( auto& it : m_pAudList ) {
    if ( it->isEnabled() ) it->before( evt, name, context );
  }
}

void AuditorSvc::after( std::string const& evt, const std::string& name, EventContext const& context,
                        StatusCode const& sc ) {
  if ( !isEnabled() ) return;
  for ( auto& it : m_pAudList ) {
    if ( it->isEnabled() ) it->after( evt, name, context, sc );
  }
}
