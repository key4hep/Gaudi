#include "IncidentAsyncTestSvc.h"

#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"

#include "GaudiKernel/MsgStream.h"

#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ThreadLocalContext.h"

DECLARE_COMPONENT( IncidentAsyncTestSvc )

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

StatusCode IncidentAsyncTestSvc::initialize()
{
  auto sc = Service::initialize();
  if ( sc.isFailure() ) return sc;
  m_incSvc = service( "IncidentSvc", true );
  if ( !m_incSvc ) throw GaudiException( "Cannot find IncidentSvc", name(), StatusCode::FAILURE );
  m_msgSvc = msgSvc();
  if ( m_incidentNames.empty() ) {
    std::vector<std::string> incNames;
    incNames.push_back( IncidentType::BeginEvent );
    incNames.push_back( IncidentType::EndEvent );
    m_incidentNames.setValue( incNames );
  }
  auto& incNames = m_incidentNames.value();
  for ( auto& i : incNames ) {
    m_incSvc->addListener( this, i, m_prio );
  }
  return sc;
}

StatusCode IncidentAsyncTestSvc::finalize() { return Service::finalize(); }

//=============================================================================
void IncidentAsyncTestSvc::handle( const Incident& incident )
{
  if ( incident.type() == IncidentType::BeginEvent ) {
    auto res = m_ctxData.insert(
        std::make_pair( incident.context(), incident.context().evt() * m_eventMultiplier + m_fileOffset ) );
    if ( !res.second ) {
      warning() << " Context already exists for '" << incident.type() << "' event=" << incident.context().evt()
                << endmsg;
    }
  } else if ( incident.type() == IncidentType::EndEvent ) {
    {
      std::unique_lock<decltype( m_eraseMutex )>( m_eraseMutex );
      auto res = m_ctxData.unsafe_erase( incident.context() );
      if ( res == 0 ) {
        warning() << " Context is missing for '" << incident.type() << "' event=" << incident.context().evt() << endmsg;
      }
    }
    info() << " Cleaned up context store for event =" << incident.context().evt() << " for incident='"
           << incident.type() << "'" << endmsg;
  }
  info() << " Handling incident '" << incident.type() << "' at ctx=" << incident.context() << endmsg;
}

void IncidentAsyncTestSvc::getData( uint64_t* data, EventContext* ctx ) const
{
  debug() << "Asked for data with context " << *ctx << endmsg;
  if ( ctx ) {
    auto cit = m_ctxData.find( *ctx );
    if ( cit == m_ctxData.end() ) {
      fatal() << " data for event " << ctx->evt() << " is not initialized yet!. This shouldn't happen!" << endmsg;
      return;
    }
    *data = cit->second;
  } else {
    const auto& ct = Gaudi::Hive::currentContext();
    auto cit       = m_ctxData.find( ct );
    if ( cit == m_ctxData.end() ) {
      fatal() << " data for event " << ct.evt() << " is not initialized yet!. This shouldn't happen!" << endmsg;
      return;
    }
    *data = cit->second;
  }
}
