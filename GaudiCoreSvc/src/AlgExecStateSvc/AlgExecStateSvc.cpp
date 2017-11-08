#include "AlgExecStateSvc.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include "GaudiKernel/SvcFactory.h"

DECLARE_SERVICE_FACTORY( AlgExecStateSvc )

//=============================================================================

AlgExecStateSvc::AlgExecStateSvc( const std::string& name, ISvcLocator* svcLoc )
    : base_class( name, svcLoc ), m_isInit( false )
{
}

//-----------------------------------------------------------------------------

AlgExecStateSvc::~AlgExecStateSvc() {}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::init()
{

  std::string wbn;

  // seriously? do we have no way of getting a Service by type???
  std::list<IService*> lst = serviceLocator()->getServices();
  for ( auto& is : lst ) {
    IHiveWhiteBoard* iwb = dynamic_cast<IHiveWhiteBoard*>( is );
    if ( iwb != 0 ) {
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
  for ( auto& alg : algos ) {
    addAlg( alg );
  }

  for ( auto& alg : m_preInitAlgs ) {
    addAlg( alg );
  }

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

StatusCode AlgExecStateSvc::initialize()
{

  // Initialise mother class (read properties, ...)
  StatusCode sc( Service::initialize() );
  if ( !sc.isSuccess() ) {
    warning() << "Base class could not be initialized" << endmsg;
    return StatusCode::FAILURE;
  }

  // moved all initialization to init(). hopefully it will get called in time....

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------

StatusCode AlgExecStateSvc::finalize() { return StatusCode::SUCCESS; }

//-----------------------------------------------------------------------------

void AlgExecStateSvc::dump( std::ostringstream& ost, const EventContext& ctx ) const
{
  size_t slotID = ctx.valid() ? ctx.slot() : 0;

  ost << "Event: " << trans( m_eventStatus.at( slotID ) ) << std::endl;
  ost << "Algs: " << m_algStates.at( slotID ).size() << std::endl;

  size_t ml( 0 );
  for ( auto& e : m_algStates.at( slotID ) ) {
    if ( e.first.str().length() > ml ) {
      ml = e.first.str().length();
    }
  }

  ost << " - Slot " << slotID << std::endl;
  const AlgStateMap_t& m = m_algStates.at( slotID );
  for ( auto& e : m ) {
    ost << "  + " << std::setw( ml ) << e.first.str() << "  " << e.second << std::endl;
  }
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::addAlg( IAlgorithm* iAlg ) { return addAlg( iAlg->nameKey() ); }

//-----------------------------------------------------------------------------

void AlgExecStateSvc::addAlg( const Gaudi::StringKey& alg )
{

  if ( !m_isInit ) {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "preInit: will add Alg " << alg.str() << " later" << endmsg;
    m_preInitAlgs.push_back( alg );
    return;
  }

  if ( m_algStates.size() > 0 ) {
    if ( m_algStates.at( 0 ).find( alg ) != m_algStates.at( 0 ).end() ) {
      // already added
      return;
    }
  }

  {
    // in theory, this should only get called during initialization (serial)
    // so shouldn't have to protect with a mutex...
    std::lock_guard<std::mutex> lock( m_mut );

    AlgExecState s;
    for ( size_t i = 0; i < m_algStates.size(); ++i ) {
      m_algStates.at( i )[alg] = s;
    }

    m_errorCount[alg] = 0;
  }

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "adding alg " << alg.str() << " to " << m_algStates.size() << " slots" << endmsg;
}

//-----------------------------------------------------------------------------

const AlgExecState& AlgExecStateSvc::algExecState( const Gaudi::StringKey& algName, const EventContext& ctx ) const
{

  checkInit();

  AlgStateMap_t::const_iterator itr = m_algStates.at( ctx.slot() ).find( algName );

  if ( UNLIKELY( itr == m_algStates.at( ctx.slot() ).end() ) ) {
    throw GaudiException{std::string{"cannot find Alg "} + algName.str() + " in AlgStateMap", name(),
                         StatusCode::FAILURE};
  }

  return itr->second;
}

//-----------------------------------------------------------------------------

const AlgExecState& AlgExecStateSvc::algExecState( IAlgorithm* iAlg, const EventContext& ctx ) const
{

  return algExecState( iAlg->nameKey(), ctx );
}

//-----------------------------------------------------------------------------

AlgExecState& AlgExecStateSvc::algExecState( IAlgorithm* iAlg, const EventContext& ctx )
{

  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );

  AlgStateMap_t::iterator itr = m_algStates.at( ctx.slot() ).find( iAlg->nameKey() );

  if ( UNLIKELY( itr == m_algStates.at( ctx.slot() ).end() ) ) {
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
  if ( m_eventStatus.at( ctx.slot() ) == EventStatus::Success ) {
    if ( fail ) m_eventStatus.at( ctx.slot() ) = EventStatus::AlgFail;
  } else if ( m_eventStatus.at( ctx.slot() ) == EventStatus::Invalid ) {
    if ( !fail ) {
      m_eventStatus.at( ctx.slot() ) = EventStatus::Success;
    } else {
      m_eventStatus.at( ctx.slot() ) = EventStatus::AlgFail;
    }
  }
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::reset( const EventContext& ctx )
{

  if ( msgLevel( MSG::DEBUG ) ) verbose() << "reset(" << ctx.slot() << ")" << endmsg;

  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  for ( auto& e : m_algStates.at( ctx.slot() ) ) {
    e.second.reset();
  }

  m_eventStatus.at( ctx.slot() ) = EventStatus::Invalid;
}

//-----------------------------------------------------------------------------

unsigned int AlgExecStateSvc::algErrorCount( const IAlgorithm* iAlg ) const
{
  AlgErrorMap_t::const_iterator itr = m_errorCount.find( iAlg->nameKey() );

  if ( itr != m_errorCount.end() ) {
    return itr->second;
  } else {
    error() << "Unable to find Algorithm \"" << iAlg->name() << "\" in map"
            << " of ErrorCounts" << endmsg;
    return 0;
  }
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::resetErrorCount( const IAlgorithm* iAlg )
{
  AlgErrorMap_t::iterator itr = m_errorCount.find( iAlg->nameKey() );
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
  AlgErrorMap_t::iterator itr = m_errorCount.find( iAlg->nameKey() );
  if ( itr != m_errorCount.end() ) {
    return ( ++( itr->second ) );
  } else {
    error() << "Unable to find Algorithm \"" << iAlg->name() << "\" in map"
            << " of ErrorCounts" << endmsg;
    return 0;
  }
}

//-----------------------------------------------------------------------------

std::string AlgExecStateSvc::trans( const EventStatus::Status& s ) const
{
  switch ( s ) {
  case EventStatus::Invalid:
    return "Invalid";
  case EventStatus::Success:
    return "Success";
  case EventStatus::AlgFail:
    return "AlgFail";
  case EventStatus::AlgStall:
    return "AlgStall";
  case EventStatus::Other:
    return "Other";
  default:
    return "Should not happen";
  }
}
