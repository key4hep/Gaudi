/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

// Framework includes
#include <GaudiKernel/AppReturnCode.h>
#include <GaudiKernel/ChronoEntity.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/HashMap.h>
#include <GaudiKernel/IIncidentListener.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/Incident.h>
#include <GaudiKernel/LockedChrono.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/StringKey.h>

// STL includes
#include <list>

// TBB includes
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>

/**
 * @class IncidentSvc
 * @brief Default implementation of the IIncidentSvc interface.
 *
 * This implementation is thread-safe with the following features:
 *  - Calls to fireIncident(), addListener() and removeListener() are
 *    synchronized across threads.
 *  - Calls to IIncidentListener::handle() are serialized, i.e. at any time
 *    there is at most one incident handler being executed across all threads.
 */

struct isSingleShot_t;

class IncidentSvc : public extends<Service, IIncidentSvc> {
private:
  // Typedefs
  // ListenerList should be a list rather than a vector because handing
  // a fired incident may result in a call to addListener.
  typedef std::list<IIncidentSvc::Listener>                                    ListenerList;
  typedef GaudiUtils::HashMap<Gaudi::StringKey, std::unique_ptr<ListenerList>> ListenerMap;

public:
  // Inherited Service overrides:
  StatusCode finalize() override;

  void addListener( IIncidentListener* lis, const std::string& type = "", long priority = 0, bool rethrow = false,
                    bool singleShot = false ) override;

  void removeListener( IIncidentListener* l, const std::string& type = "" ) override;
  void fireIncident( const Incident& incident ) override;
  void fireIncident( std::unique_ptr<Incident> incident ) override;
  // TODO: return by value instead...
  void getListeners( std::vector<IIncidentListener*>& lis, const std::string& type = "" ) const override;

  IncidentSvc( const std::string& name, ISvcLocator* svc );
  ~IncidentSvc() override;
  IIncidentSvc::IncidentPack getIncidents( const EventContext* ctx ) override;

private:
  ListenerMap::iterator removeListenerFromList( ListenerMap::iterator, IIncidentListener* item, bool scheduleRemoval );
  /// Internal function to allow incidents listening to all events
  void i_fireIncident( const Incident& incident, const std::string& type );

  /// List of auditor names
  ListenerMap m_listenerMap;

  /// Incident being fired. It is used to know if we can safely remove a listener or
  /// we have to schedule its removal for later.
  const std::string* m_currentIncidentType = nullptr;

  /// Mutex to synchronize access to m_listenerMap
  mutable std::recursive_mutex m_listenerMapMutex;

  /// timer & it's lock
  mutable ChronoEntity m_timer;
  mutable bool         m_timerLock = false;
  // When TBB supports unique_ptrs in concurrent queue typedef should be changed
  // typedef tbb::concurrent_queue<std::unique_ptr<Incident>> IncQueue_t;
  typedef tbb::concurrent_queue<std::unique_ptr<Incident>>             IncQueue_t;
  tbb::concurrent_unordered_map<EventContext::ContextID_t, IncQueue_t> m_firedIncidents;

  /// Event ID for each slot
  tbb::concurrent_unordered_map<EventContext::ContextID_t, EventContext::ContextEvt_t> m_slotEvent;
};

DECLARE_COMPONENT( IncidentSvc )

namespace {
  static const std::string s_unknown = "<unknown>";
  // Helper to get the name of the listener
  inline const std::string& getListenerName( IIncidentListener* lis ) {
    SmartIF<INamedInterface> iNamed( lis );
    return iNamed ? iNamed->name() : s_unknown;
  }
} // namespace

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

IncidentSvc::IncidentSvc( const std::string& name, ISvcLocator* svc ) : base_class( name, svc ) {}
IncidentSvc::~IncidentSvc() { auto lock = std::scoped_lock{ m_listenerMapMutex }; }
StatusCode IncidentSvc::finalize() {
  DEBMSG << m_timer.outputUserTime( "Incident  timing: Mean(+-rms)/Min/Max:%3%(+-%4%)/%6%/%7%[ms] ", System::milliSec )
         << m_timer.outputUserTime( "Total:%2%[s]", System::Sec ) << endmsg;

  m_firedIncidents.clear();

  // Finalize this specific service
  return Service::finalize();
}
void IncidentSvc::addListener( IIncidentListener* lis, const std::string& type, long prio, bool rethrow,
                               bool singleShot ) {
  static const std::string all{ "ALL" };
  auto                     lock = std::scoped_lock{ m_listenerMapMutex };

  const std::string& ltype = ( !type.empty() ? type : all );

  // find if the type already exists
  auto itMap = m_listenerMap.find( ltype );
  if ( itMap == m_listenerMap.end() ) {
    // if not found, create and insert now a list of listeners
    auto p = m_listenerMap.insert( { ltype, std::make_unique<ListenerList>() } );
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
  llist.emplace( i, IIncidentSvc::Listener{ lis, prio, rethrow, singleShot } );
}
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
void IncidentSvc::removeListener( IIncidentListener* lis, const std::string& type ) {
  auto lock = std::scoped_lock{ m_listenerMapMutex };

  bool scheduleForRemoval = ( m_currentIncidentType && type == *m_currentIncidentType );
  if ( type.empty() ) {
    auto i = std::begin( m_listenerMap );
    while ( i != std::end( m_listenerMap ) ) { i = removeListenerFromList( i, lis, scheduleForRemoval ); }
  } else {
    auto i = m_listenerMap.find( type );
    if ( i != m_listenerMap.end() ) removeListenerFromList( i, lis, scheduleForRemoval );
  }
}
namespace {
  /// Helper class to identify a singleShot Listener
  constexpr struct isSingleShot_t {
    bool operator()( const IncidentSvc::Listener& l ) const { return l.singleShot; }
  } isSingleShot{};
} // namespace
void IncidentSvc::i_fireIncident( const Incident& incident, const std::string& listenerType ) {

  auto lock = std::scoped_lock{ m_listenerMapMutex };

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
  std::string curIncTyp;
  if ( m_currentIncidentType != nullptr ) {
    curIncTyp = *m_currentIncidentType;
  } else {
    curIncTyp = "UNKNOWN";
  }

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
              << curIncTyp << " in listener " << getListenerName( listener.iListener ) << endmsg;
      error() << exc << endmsg;
      if ( listener.rethrow ) { throw exc; }
    } catch ( const std::exception& exc ) {
      error() << "Standard std::exception is caught"
                 " handling incident "
              << curIncTyp << " in listener " << getListenerName( listener.iListener ) << endmsg;
      error() << exc.what() << endmsg;
      if ( listener.rethrow ) { throw exc; }
    } catch ( ... ) {
      error() << "UNKNOWN Exception is caught"
                 " handling incident "
              << curIncTyp << " in listener " << getListenerName( listener.iListener ) << endmsg;
      if ( listener.rethrow ) { throw; }
    }
    // check wheter one of the listeners is singleShot
    firedSingleShot |= listener.singleShot;
  }
  if ( firedSingleShot ) {
    // remove all the singleshot listeners that got their shot...
    listeners.erase( std::remove_if( std::begin( listeners ), std::end( listeners ), isSingleShot ),
                     std::end( listeners ) );
    if ( listeners.empty() ) m_listenerMap.erase( ilisteners );
  }

  m_currentIncidentType = nullptr;
}
void IncidentSvc::fireIncident( const Incident& incident ) {

  Gaudi::Utils::LockedChrono timer( m_timer, m_timerLock );

  // Call specific listeners
  i_fireIncident( incident, incident.type() );
  // Try listeners registered for ALL incidents
  if ( incident.type() != "ALL" ) { // avoid double calls if somebody fires the incident "ALL"
    i_fireIncident( incident, "ALL" );
  }
}
void IncidentSvc::fireIncident( std::unique_ptr<Incident> incident ) {

  const EventContext& ctx = incident->context();
  DEBMSG << "Async incident '" << incident->type() << "' fired on context " << ctx << endmsg;

  // create or get incident queue for slot
  auto [incItr, inserted1] = m_firedIncidents.insert( { ctx.slot(), IncQueue_t() } );
  // save or get current event for slot
  auto [slotItr, inserted2] = m_slotEvent.insert( { ctx.slot(), ctx.evt() } );

  // if new event in slot, clear all remaining old incidents
  if ( slotItr->second != ctx.evt() ) {
    slotItr->second = ctx.evt();

    if ( msgLevel( MSG::DEBUG ) and !incItr->second.empty() ) {
      debug() << "Clearing remaining obsolete incidents from slot " << ctx.slot() << ":";
      std::unique_ptr<Incident> inc;
      while ( incItr->second.try_pop( inc ) ) { debug() << " " << inc->type() << "(" << inc->context() << ")"; }
      debug() << endmsg;
    }
    incItr->second.clear();
  }
  incItr->second.push( std::move( incident ) );
}

void IncidentSvc::getListeners( std::vector<IIncidentListener*>& l, const std::string& type ) const {
  static const std::string ALL{ "ALL" };
  auto                     lock = std::scoped_lock{ m_listenerMapMutex };

  const std::string& ltype = ( !type.empty() ? type : ALL );

  l.clear();
  auto i = m_listenerMap.find( ltype );
  if ( i != m_listenerMap.end() ) {
    l.reserve( i->second->size() );
    std::transform( std::begin( *i->second ), std::end( *i->second ), std::back_inserter( l ),
                    []( const Listener& j ) { return j.iListener; } );
  }
}

IIncidentSvc::IncidentPack IncidentSvc::getIncidents( const EventContext* ctx ) {
  IIncidentSvc::IncidentPack p;
  if ( ctx ) {
    auto incs = m_firedIncidents.find( ctx->slot() );
    if ( incs != m_firedIncidents.end() ) {
      std::unique_ptr<Incident> inc;

      DEBMSG << "Collecting listeners fired on context " << *ctx << endmsg;
      while ( incs->second.try_pop( inc ) ) {
        // ensure incident is for this event (should not be necessary)
        if ( inc->context().evt() == ctx->evt() ) {
          std::scoped_lock lock( m_listenerMapMutex );
          auto             i = m_listenerMap.find( inc->type() );
          if ( i != m_listenerMap.end() ) {
            p.emplace_back( std::move( inc ), std::vector<Listener>{ i->second->begin(), i->second->end() } );
          }
        }
      }
    }
  }
  return p;
}
