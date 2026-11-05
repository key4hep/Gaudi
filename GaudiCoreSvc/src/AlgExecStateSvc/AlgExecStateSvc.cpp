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
#include <GaudiKernel/ConcurrencyFlags.h>
#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IAlgExecStateSvc.h>
#include <GaudiKernel/IAlgManager.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/ThreadLocalContext.h>

// STL includes
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <vector>

// FMT include
#include <fmt/format.h>

/**
 * @brief A service that keeps track of the execution state of Algorithm
 *
 */
class AlgExecStateSvc : public extends<Service, IAlgExecStateSvc> {

  /**
   *  internal state of an algorithm
   */
  class AlgExecStateInternal {
  public:
    bool                   filterPassed() const { return m_filterPassed; }
    void                   setFilterPassed( bool f = true ) { m_filterPassed = f; }
    AlgExecStateRef::State state() const { return m_state; }
    void                   setState( AlgExecStateRef::State s ) { m_state = s; }
    void                   setState( AlgExecStateRef::State s, const StatusCode& sc ) {
      m_state      = s;
      m_execStatus = sc;
    }
    const StatusCode& execStatus() const { return m_execStatus; }
    void              setExecStatus( const StatusCode& sc = StatusCode::SUCCESS ) { m_execStatus = sc; }
    void              reset() { *this = AlgExecStateInternal{}; }

    friend std::ostream& operator<<( std::ostream& ost, const AlgExecStateInternal& s ) {
      ost << "e: ";
      switch ( s.state() ) {
      case AlgExecStateRef::State::None:
        return ost << "n";
      case AlgExecStateRef::State::Executing:
        return ost << "e";
      default:
        return ost << "d f: " << s.filterPassed() << " sc: " << s.execStatus();
      }
    }

  private:
    bool                   m_filterPassed{ true };
    AlgExecStateRef::State m_state{ AlgExecStateRef::State::None };
    StatusCode             m_execStatus{ StatusCode::FAILURE };
  };

  using AlgStates            = std::vector<AlgExecStateInternal>;
  using AlgStatesWithSubSlot = std::unordered_map<AlgExecStateRef::AlgKey, AlgExecStateInternal>;

public:
  using extends::extends;

  AlgExecStateRef algExecState( const IAlgorithm* iAlg, const EventContext& ctx ) override {
    return { *this, ctx, algKey( iAlg->name() ) };
  }
  AlgExecStateRef algExecState( const std::string& algName, const EventContext& ctx ) override {
    return { *this, ctx, algKey( algName ) };
  }
  AlgExecStateRefConst algExecState( const IAlgorithm* iAlg, const EventContext& ctx ) const override {
    return { *this, ctx, algKey( iAlg->name() ) };
  }
  AlgExecStateRefConst algExecState( const std::string& algName, const EventContext& ctx ) const override {
    return { *this, ctx, algKey( algName ) };
  }
  void reset( const EventContext& ctx ) override;

  using IAlgExecStateSvc::addAlg;
  AlgExecStateRef::AlgKey addAlg( const Gaudi::StringKey& ) override;

  const EventStatus::Status& eventStatus( const EventContext& ctx ) const override;
  void                       setEventStatus( const EventStatus::Status& sc, const EventContext& ctx ) override;
  void                       updateEventStatus( const bool& b, const EventContext& ctx ) override;

  unsigned int algErrorCount( const IAlgorithm* iAlg ) const override;
  void         resetErrorCount() override;
  void         resetErrorCount( const IAlgorithm* iAlg ) override;
  unsigned int incrementErrorCount( const IAlgorithm* iAlg ) override;

  void dump( std::ostream& ost, const EventContext& ctx ) const override;
  void dump( std::ostream&, const EventContext&, AlgExecStateRef::AlgKey ) const override;

private:
  bool                   filterPassed( const EventContext&, AlgExecStateRef::AlgKey ) const override;
  void                   setFilterPassed( const EventContext&, AlgExecStateRef::AlgKey, bool ) override;
  AlgExecStateRef::State state( const EventContext&, AlgExecStateRef::AlgKey ) const override;
  void                   setState( const EventContext&, AlgExecStateRef::AlgKey, AlgExecStateRef::State ) override;
  void setState( const EventContext&, AlgExecStateRef::AlgKey, AlgExecStateRef::State, const StatusCode& ) override;
  const StatusCode&  execStatus( const EventContext&, AlgExecStateRef::AlgKey ) const override;
  void               setExecStatus( const EventContext&, AlgExecStateRef::AlgKey, const StatusCode& ) override;
  const std::string& algName( AlgExecStateRef::AlgKey ) const override;

  AlgExecStateInternal&       getInternalState( const EventContext& ctx, AlgExecStateRef::AlgKey k );
  AlgExecStateInternal const& getInternalState( const EventContext& ctx, AlgExecStateRef::AlgKey k ) const;
  AlgExecStateRef::AlgKey     algKey( const std::string& algName ) const { return m_algNameToIndex.at( algName ); }

  // one vector entry per event slot
  std::vector<AlgStates> m_algStates;
  // one map entry per event slot and subslot, with AlgKey as a key, for sparse storage
  std::vector<std::vector<AlgStatesWithSubSlot>> m_algSubSlotStates;
  // algorithm name to slot (with heterogeneous lookup), needed to regognize double addition
  std::unordered_map<Gaudi::StringKey, AlgExecStateRef::AlgKey, Gaudi::StringKeyHash, std::equal_to<>> m_algNameToIndex;

  std::vector<EventStatus::Status> m_eventStatus;
  std::vector<Gaudi::StringKey>    m_preInitAlgs;
  // keep the names of the algorithms by AlgKey for logging purposes
  std::vector<std::string> m_algNames;

  // Cannot be a vector as atomics are not copy/move constructible
  std::unordered_map<AlgExecStateRef::AlgKey, std::atomic<unsigned int>> m_errorCount;

  void           init();
  void           checkInit() const;
  std::once_flag m_initFlag;
  bool           m_isInit{ false };

  mutable std::mutex m_mut;
};

DECLARE_COMPONENT( AlgExecStateSvc )

void AlgExecStateSvc::init() {

  const std::size_t slots = Gaudi::Concurrency::ConcurrencyFlags::concurrent()
                                ? std::max( (size_t)1, Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents() )
                                : 1;
  m_algStates.resize( slots );
  m_eventStatus.resize( slots );

  if ( msgLevel( MSG::DEBUG ) ) debug() << "resizing state containers to : " << slots << endmsg;

  m_isInit = true;

  // call addAlg for algorithms which called it too early.
  for ( const auto& alg : m_preInitAlgs ) addAlg( alg );

  if ( msgLevel( MSG::VERBOSE ) ) {
    std::ostringstream ost;
    dump( ost, Gaudi::Hive::currentContext() );
    verbose() << "dumping state:\n" << ost.str() << endmsg;
  }
}

AlgExecStateRef::AlgKey AlgExecStateSvc::addAlg( const Gaudi::StringKey& alg ) {
  if ( !m_isInit ) {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "preInit: will add Alg " << alg.str() << " later" << endmsg;
    auto it = std::find( begin( m_preInitAlgs ), end( m_preInitAlgs ), alg );
    if ( it == m_preInitAlgs.end() ) {
      m_preInitAlgs.push_back( alg );
      return static_cast<AlgExecStateRef::AlgKey>( m_preInitAlgs.size() - 1 );
    } else {
      return static_cast<AlgExecStateRef::AlgKey>( std::distance( begin( m_preInitAlgs ), it ) );
    }
  }
  // try to create default state for first slot and retrieve the key
  auto [it, b] =
      m_algNameToIndex.try_emplace( alg, static_cast<AlgExecStateRef::AlgKey>( m_algStates.front().size() ) );
  if ( b ) {
    // create default state for all slots
    for ( auto& a : m_algStates ) a.emplace_back();
    m_errorCount.emplace( it->second, 0 );
    m_algNames.push_back( alg );
    if ( msgLevel( MSG::DEBUG ) )
      debug() << "adding alg " << alg.str() << " to " << m_algStates.size() << " slots" << endmsg;
  }
  return it->second;
}

AlgExecStateSvc::AlgExecStateInternal& AlgExecStateSvc::getInternalState( const EventContext&     ctx,
                                                                          AlgExecStateRef::AlgKey k ) {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  // Sub-slots are dynamic
  // Assuming the alg is known, look for a subslot
  // Return the existing state, or create a new one
  if ( ctx.usesSubSlot() ) {
    // Use mutex since dynamic
    std::scoped_lock lock( m_mut );

    // Check that there is any sub-slot information for this slot
    if ( ctx.slot() >= m_algSubSlotStates.size() ) m_algSubSlotStates.resize( ctx.slot() + 1 );

    // Check that there is information for this sub-slot
    auto& subSlots = m_algSubSlotStates[ctx.slot()];
    if ( ctx.subSlot() >= subSlots.size() ) subSlots.resize( ctx.subSlot() + 1 );

    // Find (and potentially extend) the AlgStates
    return subSlots[ctx.subSlot()].try_emplace( k ).first->second;
  }
  // regular case with no subslot
  assert( ctx.slot() < m_algStates.size() );
  assert( static_cast<size_t>( k ) < m_algStates.at( ctx.slot() ).size() );
  return m_algStates.at( ctx.slot() )[static_cast<size_t>( k )];
}

AlgExecStateSvc::AlgExecStateInternal const& AlgExecStateSvc::getInternalState( const EventContext&     ctx,
                                                                                AlgExecStateRef::AlgKey k ) const {
  assert( m_isInit );

  // Sub-slots are dynamic
  // Assuming the alg is known, look for a subslot
  // Return the existing state, or create a new one
  if ( ctx.usesSubSlot() ) {
    // Use mutex since dynamic
    std::scoped_lock lock( m_mut );
    // Check that there is any sub-slot information for this slot
    if ( ctx.slot() >= m_algSubSlotStates.size() ) {
      throw GaudiException( "Could not find slot in m_algSubSlotStates", "AlgExecStateSvc", StatusCode::FAILURE );
    }
    // Check that there is information for this sub-slot
    auto& subSlots = m_algSubSlotStates[ctx.slot()];
    if ( ctx.subSlot() >= subSlots.size() ) {
      throw GaudiException( "Could not find subslot in m_algSubSlotStates", "AlgExecStateSvc", StatusCode::FAILURE );
    }
    // Find the AlgStates
    return subSlots[ctx.subSlot()].at( k );
  }
  // regular case with no subslot
  return m_algStates.at( ctx.slot() )[static_cast<size_t>( k )];
}

const EventStatus::Status& AlgExecStateSvc::eventStatus( const EventContext& ctx ) const {
  assert( m_isInit );
  return m_eventStatus.at( ctx.slot() );
}

void AlgExecStateSvc::setEventStatus( const EventStatus::Status& sc, const EventContext& ctx ) {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  std::atomic_ref{ m_eventStatus.at( ctx.slot() ) }.store( sc, std::memory_order_relaxed );
}

void AlgExecStateSvc::updateEventStatus( const bool& fail, const EventContext& ctx ) {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  // Called at the end of every algorithm, and algorithms of one event slot finish concurrently:
  // update atomically so that a failure cannot be overwritten by a concurrent success.
  std::atomic_ref status{ m_eventStatus.at( ctx.slot() ) };
  auto            current = status.load( std::memory_order_relaxed );
  while ( true ) {
    auto next = current;
    if ( current == EventStatus::Success ) {
      if ( fail ) next = EventStatus::AlgFail;
    } else if ( current == EventStatus::Invalid ) {
      next = ( fail ? EventStatus::AlgFail : EventStatus::Success );
    }
    if ( next == current || status.compare_exchange_weak( current, next, std::memory_order_relaxed ) ) { break; }
  }
}

unsigned int AlgExecStateSvc::algErrorCount( const IAlgorithm* iAlg ) const {
  assert( m_isInit );
  return m_errorCount.at( algKey( iAlg->name() ) );
}
void AlgExecStateSvc::resetErrorCount() {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  for ( unsigned int k = 0; k < m_errorCount.size(); k++ ) m_errorCount[static_cast<AlgExecStateRef::AlgKey>( k )] = 0;
}
void AlgExecStateSvc::resetErrorCount( const IAlgorithm* iAlg ) {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  m_errorCount[algKey( iAlg->name() )] = 0;
}
unsigned int AlgExecStateSvc::incrementErrorCount( const IAlgorithm* iAlg ) {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  return ++m_errorCount[algKey( iAlg->name() )];
}
bool AlgExecStateSvc::filterPassed( const EventContext& ctx, AlgExecStateRef::AlgKey k ) const {
  assert( m_isInit );
  return getInternalState( ctx, k ).filterPassed();
}
void AlgExecStateSvc::setFilterPassed( const EventContext& ctx, AlgExecStateRef::AlgKey k, bool b ) {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  getInternalState( ctx, k ).setFilterPassed( b );
}
AlgExecStateRef::State AlgExecStateSvc::state( const EventContext& ctx, AlgExecStateRef::AlgKey k ) const {
  assert( m_isInit );
  return getInternalState( ctx, k ).state();
}
void AlgExecStateSvc::setState( const EventContext& ctx, AlgExecStateRef::AlgKey k, AlgExecStateRef::State s ) {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  getInternalState( ctx, k ).setState( s );
}
void AlgExecStateSvc::setState( const EventContext& ctx, AlgExecStateRef::AlgKey k, AlgExecStateRef::State s,
                                const StatusCode& sc ) {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  getInternalState( ctx, k ).setState( s, sc );
}
const StatusCode& AlgExecStateSvc::execStatus( const EventContext& ctx, AlgExecStateRef::AlgKey k ) const {
  assert( m_isInit );
  return getInternalState( ctx, k ).execStatus();
}
void AlgExecStateSvc::setExecStatus( const EventContext& ctx, AlgExecStateRef::AlgKey k, const StatusCode& s ) {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  getInternalState( ctx, k ).setExecStatus( s );
}
const std::string& AlgExecStateSvc::algName( AlgExecStateRef::AlgKey k ) const {
  assert( m_isInit );
  return m_algNames[static_cast<size_t>( k )];
}

void AlgExecStateSvc::dump( std::ostream& ost, const EventContext& ctx ) const {
  const size_t slotID = ctx.valid() ? ctx.slot() : 0;
  ost << "  [slot: " << slotID << ", incident: " << m_eventStatus.at( slotID ) << "]:\n\n";
  auto& algState = m_algStates.at( slotID );
  auto  ml       = std::accumulate( begin( m_algNames ), end( m_algNames ), size_t{ 0 },
                                    []( size_t m, const auto& as ) { return std::max( m, as.length() ); } );
  for ( size_t k = 0; const auto& e : algState )
    ost << "  + " << std::setw( ml ) << algName( static_cast<AlgExecStateRef::AlgKey>( k++ ) ) << "  " << e << '\n';
}

void AlgExecStateSvc::dump( std::ostream& ost, const EventContext& ctx, AlgExecStateRef::AlgKey k ) const {
  ost << getInternalState( ctx, k );
}

void AlgExecStateSvc::reset( const EventContext& ctx ) {
  if ( msgLevel( MSG::VERBOSE ) ) verbose() << "reset(" << ctx.slot() << ")" << endmsg;

  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  for ( auto& it : m_algStates.at( ctx.slot() ) ) it.reset();

  // Also clear sub slots
  if ( ctx.slot() < m_algSubSlotStates.size() ) {
    for ( auto& subSlot : m_algSubSlotStates[ctx.slot()] ) {
      for ( auto& it : subSlot ) it.second.reset();
    }
  }

  std::atomic_ref{ m_eventStatus.at( ctx.slot() ) }.store( EventStatus::Invalid, std::memory_order_relaxed );
}
