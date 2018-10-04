#include "AlgExecStateSvc.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/ThreadLocalContext.h"

DECLARE_COMPONENT( AlgExecStateSvc )

//=============================================================================

void AlgExecStateSvc::init()
{

  // seriously? do we have no way of getting a Service by type???
  std::string wbn;
  for ( auto& is : serviceLocator()->getServices() ) {
    IHiveWhiteBoard* iwb = dynamic_cast<IHiveWhiteBoard*>( is );
    if ( iwb ) {
      wbn = is->name();
      if ( msgLevel( MSG::VERBOSE ) ) verbose() << "HiveWhiteBoard service name is " << wbn << endmsg;
      break;
    }
  }

  SmartIF<IHiveWhiteBoard> wbs;
  wbs = serviceLocator()->service( wbn, false );

  if ( wbs.isValid() ) {
    m_algStates.resize( wbs->getNumberOfStores() );
    m_eventStatus.resize( wbs->getNumberOfStores() );
  } else {
    m_algStates.resize( 1 );
    m_eventStatus.resize( 1 );
  }

  if ( msgLevel( MSG::DEBUG ) ) debug() << "resizing state containers to : " << m_algStates.size() << endmsg;

  SmartIF<IAlgManager> algMan( serviceLocator() );
  if ( !algMan.isValid() ) {
    fatal() << "could not get the AlgManager" << endmsg;
    throw GaudiException( "In AlgExecStateSvc, unable to get the AlgManager!", "AlgExecStateSvc", StatusCode::FAILURE );
  }

  m_isInit = true;

  auto algos = algMan->getAlgorithms();
  for ( auto& alg : algos ) addAlg( alg );
  for ( auto& alg : m_preInitAlgs ) addAlg( alg );

  if ( msgLevel( MSG::VERBOSE ) ) {
    std::ostringstream ost;
    dump( ost, Gaudi::Hive::currentContext() );
    verbose() << "dumping state:\n" << ost.str() << endmsg;
  }
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::checkInit() const
{

  if ( !m_isInit ) {
    fatal() << "AlgExecStateSvc not initialized before first use" << endmsg;
    throw GaudiException( "AlgExecStateSvc not initialized before first use!", "AlgExecStateSvc", StatusCode::FAILURE );
  }
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::dump( std::ostringstream& ost, const EventContext& ctx ) const
{
  size_t slotID = ctx.valid() ? ctx.slot() : 0;

  ost << "  [slot: " << slotID << ", incident: " << m_eventStatus.at( slotID ) << "]:\n\n";

  auto& algState = m_algStates.at( slotID );
  auto  ml       = std::accumulate( begin( algState ), end( algState ), size_t{0},
                             []( size_t m, const auto& as ) { return std::max( m, as.first.str().length() ); } );

  for ( auto& e : algState ) ost << "  + " << std::setw( ml ) << e.first.str() << "  " << e.second << '\n';
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::addAlg( const Gaudi::StringKey& alg )
{
  if ( !m_isInit ) {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "preInit: will add Alg " << alg.str() << " later" << endmsg;
    m_preInitAlgs.push_back( alg );
    return;
  }

  if ( !m_algStates.empty() && m_algStates.front().find( alg ) != m_algStates.front().end() ) {
    // already added
    return;
  }

  {
    // in theory, this should only get called during initialization (serial)
    // so shouldn't have to protect with a mutex...
    std::lock_guard<std::mutex> lock( m_mut );

    AlgExecState s;
    for ( auto& a : m_algStates ) a[alg] = s;
    m_errorCount[alg]                    = 0;
  }

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "adding alg " << alg.str() << " to " << m_algStates.size() << " slots" << endmsg;
}

//-----------------------------------------------------------------------------

const AlgExecState& AlgExecStateSvc::algExecState( const Gaudi::StringKey& algName, const EventContext& ctx ) const
{
  checkInit();

  auto& algState = m_algStates.at( ctx.slot() );
  auto  itr      = algState.find( algName );
  if ( UNLIKELY( itr == algState.end() ) ) {
    throw GaudiException{"cannot find Alg " + algName.str() + " in AlgStateMap", name(), StatusCode::FAILURE};
  }
  return itr->second;
}

//-----------------------------------------------------------------------------

AlgExecState& AlgExecStateSvc::algExecState( IAlgorithm* iAlg, const EventContext& ctx )
{
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );

  auto& algState = m_algStates.at( ctx.slot() );
  auto  itr      = algState.find( iAlg->nameKey() );

  if ( UNLIKELY( itr == algState.end() ) ) {
    throw GaudiException{std::string{"cannot find Alg "} + iAlg->name() + " in AlgStateMap", name(),
                         StatusCode::FAILURE};
  }

  return itr->second;
}

//-----------------------------------------------------------------------------

const IAlgExecStateSvc::AlgStateMap_t& AlgExecStateSvc::algExecStates( const EventContext& ctx ) const
{
  checkInit();
  return m_algStates.at( ctx.slot() );
}

//-----------------------------------------------------------------------------

const EventStatus::Status& AlgExecStateSvc::eventStatus( const EventContext& ctx ) const
{
  checkInit();
  return m_eventStatus.at( ctx.slot() );
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::setEventStatus( const EventStatus::Status& sc, const EventContext& ctx )
{
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  m_eventStatus.at( ctx.slot() ) = sc;
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::updateEventStatus( const bool& fail, const EventContext& ctx )
{
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  auto& status = m_eventStatus.at( ctx.slot() );
  if ( status == EventStatus::Success ) {
    if ( fail ) status = EventStatus::AlgFail;
  } else if ( status == EventStatus::Invalid ) {
    status = ( fail ? EventStatus::AlgFail : EventStatus::Success );
  }
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::reset( const EventContext& ctx )
{
  if ( msgLevel( MSG::DEBUG ) ) verbose() << "reset(" << ctx.slot() << ")" << endmsg;

  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  for ( auto& e : m_algStates.at( ctx.slot() ) ) e.second.reset();

  m_eventStatus.at( ctx.slot() ) = EventStatus::Invalid;
}

//-----------------------------------------------------------------------------

unsigned int AlgExecStateSvc::algErrorCount( const IAlgorithm* iAlg ) const
{
  auto itr = m_errorCount.find( iAlg->nameKey() );
  if ( itr == m_errorCount.end() ) {
    error() << "Unable to find Algorithm \"" << iAlg->name() << "\" in map"
            << " of ErrorCounts" << endmsg;
    return 0;
  }

  return itr->second;
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::resetErrorCount( const IAlgorithm* iAlg )
{
  auto itr = m_errorCount.find( iAlg->nameKey() );
  if ( itr != m_errorCount.end() ) {
    itr->second = 0;
  } else {
    error() << "Unable to find Algorithm \"" << iAlg->name() << "\" in map"
            << " of ErrorCounts" << endmsg;
  }
}

//-----------------------------------------------------------------------------

unsigned int AlgExecStateSvc::incrementErrorCount( const IAlgorithm* iAlg )
{
  auto itr = m_errorCount.find( iAlg->nameKey() );
  if ( itr == m_errorCount.end() ) {
    error() << "Unable to find Algorithm \"" << iAlg->name() << "\" in map"
            << " of ErrorCounts" << endmsg;
    return 0;
  }
  return ++( itr->second );
}
