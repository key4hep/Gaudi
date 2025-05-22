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
#include "IncidentProcAlg.h"
#include <GaudiKernel/IIncidentListener.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/Incident.h>
#include <GaudiKernel/ThreadLocalContext.h>

DECLARE_COMPONENT( IncidentProcAlg )

namespace {
  static const std::string s_unknown = "<unknown>";
  // Helper to get the name of the listener
  inline const std::string& getListenerName( IIncidentListener* lis ) {
    SmartIF<INamedInterface> iNamed( lis );
    return iNamed ? iNamed->name() : s_unknown;
  }
} // namespace

IncidentProcAlg::IncidentProcAlg( const std::string& name, ISvcLocator* pSvcLocator )
    : Algorithm( name, pSvcLocator ) {}

StatusCode IncidentProcAlg::initialize() {
  StatusCode sc = Algorithm::initialize();
  if ( sc.isFailure() ) return sc;
  m_incSvc = service( "IncidentSvc", true );
  return StatusCode::SUCCESS;
}

StatusCode IncidentProcAlg::execute( const EventContext& ctx ) const {
  auto      incPack = m_incSvc->getIncidents( &ctx );
  MsgStream log( msgSvc(), name() );
  if ( msgLevel( MSG::DEBUG ) ) {
    log << MSG::DEBUG << " Number of Incidents to process = " << incPack.size() << " Context= " << ctx << endmsg;
  }
  while ( !incPack.empty() ) {
    for ( const auto& [inc, listeners] : incPack ) {
      for ( const auto& l : listeners ) {
        if ( msgLevel( MSG::DEBUG ) ) {
          log << MSG::DEBUG << "Calling '" << getListenerName( l.iListener ) << "' for incident [" << inc->type() << "]"
              << endmsg;
        }

        // handle exceptions if they occur
        try {
          l.iListener->handle( *inc );
        } catch ( const GaudiException& exc ) {
          error() << "Exception with tag=" << exc.tag()
                  << " is caught"
                     " handling incident"
                  << inc->type() << endmsg;
          error() << exc << endmsg;
          if ( l.rethrow ) { throw exc; }
        } catch ( const std::exception& exc ) {
          error() << "Standard std::exception is caught"
                     " handling incident"
                  << inc->type() << endmsg;
          error() << exc.what() << endmsg;
          if ( l.rethrow ) { throw exc; }
        } catch ( ... ) {
          error() << "UNKNOWN Exception is caught handling incident" << inc->type() << endmsg;
          if ( l.rethrow ) { throw; }
        }
      }
    }
    incPack = m_incSvc->getIncidents( &ctx );
  }

  return StatusCode::SUCCESS;
}
