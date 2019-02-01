#ifdef _WIN32
// ============================================================================
// Avoid conflicts between windows and the message service.
// ============================================================================
#  define NOMSG
#  define NOGDI
#endif
// ============================================================================
// Include Files
// ============================================================================
#include "GaudiKernel/AppReturnCode.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/LockedChrono.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"
#include <functional>
// ============================================================================
// Local
// ============================================================================
#include "IncidentSvc.h"
// ============================================================================
// Instantiation of a static factory class used by clients to create
//  instances of this service
DECLARE_COMPONENT( IncidentSvc )
// ============================================================================
namespace {
  // ==========================================================================
  static const std::string s_unknown = "<unknown>";
  // Helper to get the name of the listener
  inline const std::string& getListenerName( IIncidentListener* lis ) {
    SmartIF<INamedInterface> iNamed( lis );
    return iNamed ? iNamed->name() : s_unknown;
  }
  // ==========================================================================
} // namespace

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

// ============================================================================
// Constructors and Destructors
// ============================================================================
IncidentSvc::IncidentSvc( const std::string& name, ISvcLocator* svc ) : base_class( name, svc ) {}
// ============================================================================
IncidentSvc::~IncidentSvc() { std::unique_lock<std::recursive_mutex> lock( m_listenerMapMutex ); }
// ============================================================================
// Inherited Service overrides:
// ============================================================================
StatusCode IncidentSvc::initialize() {
  // initialize the Service Base class
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) return sc;

  m_currentIncidentType = nullptr;

  // set my own (IncidentSvc) properties via the jobOptionService
  sc = setProperties();
  if ( UNLIKELY( sc.isFailure() ) ) {
    error() << "Could not set my properties" << endmsg;
    return sc;
  }
  return sc;
}
// ============================================================================
StatusCode IncidentSvc::finalize() {
  DEBMSG << m_timer.outputUserTime( "Incident  timing: Mean(+-rms)/Min/Max:%3%(+-%4%)/%6%/%7%[ms] ", System::milliSec )
         << m_timer.outputUserTime( "Total:%2%[s]", System::Sec ) << endmsg;

  {
    // clear the local storage of allocated Incident objects.
    std::unique_lock<std::recursive_mutex> lock( m_listenerMapMutex );
    for ( auto& fi : m_firedIncidents ) {
      std::for_each( fi.second.unsafe_begin(), fi.second.unsafe_end(), []( auto i ) { delete i; } );
      fi.second.clear();
    }
    m_firedIncidents.clear();
  }

  // Finalize this specific service
  return Service::finalize();
}
// ============================================================================
// Inherited IIncidentSvc overrides:
// ============================================================================
void IncidentSvc::addListener( IIncidentListener* lis, const std::string& type, long prio, bool rethrow,
                               bool singleShot ) {
  static const std::string               all{"ALL"};
  std::unique_lock<std::recursive_mutex> lock( m_listenerMapMutex );

  const std::string& ltype = ( !type.empty() ? type : all );

  // find if the type already exists
  auto itMap = m_listenerMap.find( ltype );
  if ( itMap == m_listenerMap.end() ) {
    // if not found, create and insert now a list of listeners
    auto p = m_listenerMap.insert( {ltype, std::make_unique<ListenerList>()} );
    if ( !p.second ) { /* OOPS */
    }
    itMap = p.first;
  }
  auto& llist = *itMap->second;
  // add Listener ordered by priority -- higher priority first,
  // and then add behind listeneres with the same priority
  // -- so we skip over all items with higher or same priority
  auto i = std::partition_point( std::begin( llist ), std::end( llist ),
                                 [&]( const Listener& j ) { return j.priority >= prio; } );
  // We insert before the current position
  DEBMSG << "Adding [" << type << "] listener '" << getListenerName( lis ) << "' with priority " << prio << endmsg;
  llist.emplace( i, lis, prio, rethrow, singleShot );
}
// ============================================================================
IncidentSvc::ListenerMap::iterator
IncidentSvc::removeListenerFromList( ListenerMap::iterator i, IIncidentListener* item, bool scheduleRemoval ) {
  auto match = [&]( ListenerList::const_reference j ) { return !item || item == j.iListener; };

  auto& c = *( i->second );
  if ( !scheduleRemoval ) {
    ON_DEBUG std::for_each( std::begin( c ), std::end( c ), [&]( ListenerList::const_reference j ) {
      if ( match( j ) )
        debug() << "Removing [" << i->first << "] listener '" << getListenerName( j.iListener ) << "'" << endmsg;
    } );
    c.erase( std::remove_if( std::begin( c ), std::end( c ), match ), std::end( c ) );
  } else {
    std::for_each( std::begin( c ), std::end( c ), [&]( Listener& i ) {
      if ( match( i ) ) i.singleShot = true; // will trigger removal as soon as it is safe
    } );
  }
  return c.empty() ? m_listenerMap.erase( i ) : std::next( i );
}
// ============================================================================
void IncidentSvc::removeListener( IIncidentListener* lis, const std::string& type ) {
  std::unique_lock<std::recursive_mutex> lock( m_listenerMapMutex );

  bool scheduleForRemoval = ( m_currentIncidentType && type == *m_currentIncidentType );
  if ( type.empty() ) {
    auto i = std::begin( m_listenerMap );
    while ( i != std::end( m_listenerMap ) ) { i = removeListenerFromList( i, lis, scheduleForRemoval ); }
  } else {
    auto i = m_listenerMap.find( type );
    if ( i != m_listenerMap.end() ) removeListenerFromList( i, lis, scheduleForRemoval );
  }
}
// ============================================================================
namespace {
  /// Helper class to identify a singleShot Listener
  constexpr struct isSingleShot_t {
    bool operator()( const IncidentSvc::Listener& l ) const { return l.singleShot; }
  } isSingleShot{};
} // namespace
// ============================================================================
void IncidentSvc::i_fireIncident( const Incident& incident, const std::string& listenerType ) {

  std::unique_lock<std::recursive_mutex> lock( m_listenerMapMutex );

  // Wouldn't it be better to write a small 'ReturnCode' service which
  // looks for these 'special' incidents and does whatever needs to
  // be done instead of making a special case here?

  // Special case: FailInputFile incident must set the application return code
  if ( incident.type() == IncidentType::FailInputFile || incident.type() == IncidentType::CorruptedInputFile ) {
    auto appmgr = serviceLocator()->as<IProperty>();
    Gaudi::setAppReturnCode( appmgr, incident.type() == IncidentType::FailInputFile
                                         ? Gaudi::ReturnCode::FailInput
                                         : Gaudi::ReturnCode::CorruptedInput )
        .ignore();
  }

  auto ilisteners = m_listenerMap.find( listenerType );
  if ( m_listenerMap.end() == ilisteners ) return;

  // setting this pointer will avoid that a call to removeListener() during
  // the loop triggers a segfault
  m_currentIncidentType = &incident.type();

  bool firedSingleShot = false;

  auto& listeners = *ilisteners->second;

  for ( auto& listener : listeners ) {

    VERMSG << "Calling '" << getListenerName( listener.iListener ) << "' for incident [" << incident.type() << "]"
           << endmsg;

    // handle exceptions if they occur
    try {
      listener.iListener->handle( incident );
    } catch ( const GaudiException& exc ) {
      error() << "Exception with tag=" << exc.tag()
              << " is caught"
                 " handling incident "
              << *m_currentIncidentType << endmsg;
      error() << exc << endmsg;
      if ( listener.rethrow ) { throw exc; }
    } catch ( const std::exception& exc ) {
      error() << "Standard std::exception is caught"
                 " handling incident "
              << *m_currentIncidentType << endmsg;
      error() << exc.what() << endmsg;
      if ( listener.rethrow ) { throw exc; }
    } catch ( ... ) {
      error() << "UNKNOWN Exception is caught"
                 " handling incident "
              << *m_currentIncidentType << endmsg;
      if ( listener.rethrow ) { throw; }
    }
    // check wheter one of the listeners is singleShot
    firedSingleShot |= listener.singleShot;
  }
  if ( firedSingleShot ) {
    // remove all the singleshot listeners that got there shot...
    listeners.erase( std::remove_if( std::begin( listeners ), std::end( listeners ), isSingleShot ),
                     std::end( listeners ) );
    if ( listeners.empty() ) m_listenerMap.erase( ilisteners );
  }

  m_currentIncidentType = nullptr;
}
// ============================================================================
void IncidentSvc::fireIncident( const Incident& incident ) {

  Gaudi::Utils::LockedChrono timer( m_timer, m_timerLock );

  // Call specific listeners
  i_fireIncident( incident, incident.type() );
  // Try listeners registered for ALL incidents
  if ( incident.type() != "ALL" ) { // avoid double calls if somebody fires the incident "ALL"
    i_fireIncident( incident, "ALL" );
  }
}
// ============================================================================
void IncidentSvc::fireIncident( std::unique_ptr<Incident> incident ) {

  DEBMSG << "Async incident '" << incident->type() << "' fired on context " << incident->context() << endmsg;
  auto ctx = incident->context();
  auto res = m_firedIncidents.insert( std::make_pair( ctx, IncQueue_t() ) );
  res.first->second.push( incident.release() );
}
// ============================================================================

void IncidentSvc::getListeners( std::vector<IIncidentListener*>& l, const std::string& type ) const {
  static const std::string               ALL{"ALL"};
  std::unique_lock<std::recursive_mutex> lock( m_listenerMapMutex );

  const std::string& ltype = ( !type.empty() ? type : ALL );

  l.clear();
  auto i = m_listenerMap.find( ltype );
  if ( i != m_listenerMap.end() ) {
    l.reserve( i->second->size() );
    std::transform( std::begin( *i->second ), std::end( *i->second ), std::back_inserter( l ),
                    []( const Listener& j ) { return j.iListener; } );
  }
}

// ============================================================================

IIncidentSvc::IncidentPack IncidentSvc::getIncidents( const EventContext* ctx ) {
  IIncidentSvc::IncidentPack p;
  if ( ctx ) {
    auto incs = m_firedIncidents.find( *ctx );
    if ( incs != m_firedIncidents.end() ) {
      Incident* inc( 0 );

      DEBMSG << "Collecting listeners fired on context " << *ctx << endmsg;
      while ( incs->second.try_pop( inc ) ) {
        std::vector<IIncidentListener*> ls;
        getListeners( ls, inc->type() );
        p.incidents.emplace_back( std::move( inc ) );
        p.listeners.emplace_back( std::move( ls ) );
      }
    }
  }
  return p;
}
// ============================================================================
// The END
// ============================================================================
