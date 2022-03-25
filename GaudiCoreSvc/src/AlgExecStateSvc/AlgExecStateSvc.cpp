/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "AlgExecStateSvc.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/ThreadLocalContext.h"

DECLARE_COMPONENT( AlgExecStateSvc )

//=============================================================================

void AlgExecStateSvc::init() {

  const std::size_t slots = Gaudi::Concurrency::ConcurrencyFlags::concurrent()
                                ? std::max( (size_t)1, Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents() )
                                : 1;

  m_algStates.resize( slots );
  m_eventStatus.resize( slots );

  if ( msgLevel( MSG::DEBUG ) ) debug() << "resizing state containers to : " << slots << endmsg;

  SmartIF<IAlgManager> algMan( serviceLocator() );
  if ( !algMan.isValid() ) {
    fatal() << "could not get the AlgManager" << endmsg;
    throw GaudiException( "In AlgExecStateSvc, unable to get the AlgManager!", "AlgExecStateSvc", StatusCode::FAILURE );
  }

  m_isInit = true;

  for ( const auto& alg : algMan->getAlgorithms() ) addAlg( alg );
  for ( const auto& alg : m_preInitAlgs ) addAlg( alg );

  if ( msgLevel( MSG::VERBOSE ) ) {
    std::ostringstream ost;
    dump( ost, Gaudi::Hive::currentContext() );
    verbose() << "dumping state:\n" << ost.str() << endmsg;
  }
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::checkInit() const {

  if ( !m_isInit ) {
    throw GaudiException( "AlgExecStateSvc not initialized before first use!", "AlgExecStateSvc", StatusCode::FAILURE );
  }
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::dump( std::ostringstream& ost, const EventContext& ctx ) const {
  const size_t slotID = ctx.valid() ? ctx.slot() : 0;

  ost << "  [slot: " << slotID << ", incident: " << m_eventStatus.at( slotID ) << "]:\n\n";

  auto& algState = m_algStates.at( slotID );
  auto  ml       = std::accumulate( begin( algState ), end( algState ), size_t{ 0 },
                                    []( size_t m, const auto& as ) { return std::max( m, as.first.str().length() ); } );

  for ( const auto& e : algState ) ost << "  + " << std::setw( ml ) << e.first.str() << "  " << e.second << '\n';
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::addAlg( const Gaudi::StringKey& alg ) {
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
    std::scoped_lock lock( m_mut );

    AlgExecState s;
    for ( auto& a : m_algStates ) a[alg] = s;
    m_errorCount[alg] = 0;
  }

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "adding alg " << alg.str() << " to " << m_algStates.size() << " slots" << endmsg;
}

//-----------------------------------------------------------------------------

const AlgExecState& AlgExecStateSvc::algExecState( const Gaudi::StringKey& algName, const EventContext& ctx ) const {
  checkInit();

  auto& algState = m_algStates.at( ctx.slot() );
  auto  itr      = algState.find( algName );
  if ( itr == algState.end() ) {
    throw GaudiException{ "cannot find Alg " + algName.str() + " in AlgStateMap", name(), StatusCode::FAILURE };
  }

  // Assuming the alg is known, look for its state in the sub-slot
  if ( ctx.usesSubSlot() ) {
    auto& subSlots    = m_algSubSlotStates[ctx.slot()];
    auto& thisSubSlot = subSlots[ctx.subSlot()];
    auto  subitr      = thisSubSlot.find( algName );
    if ( subitr == thisSubSlot.end() ) {
      throw GaudiException{ "cannot find Alg " + algName.str() + " in AlgStateMap", name(), StatusCode::FAILURE };
    } else {
      return subitr->second;
    }
  }

  return itr->second;
}

//-----------------------------------------------------------------------------

AlgExecState& AlgExecStateSvc::algExecState( IAlgorithm* iAlg, const EventContext& ctx ) {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );

  auto& algState = m_algStates.at( ctx.slot() );
  auto  itr      = algState.find( iAlg->nameKey() );
  if ( itr == algState.end() ) {
    throw GaudiException{ std::string{ "cannot find Alg " } + iAlg->name() + " in AlgStateMap", name(),
                          StatusCode::FAILURE };
  }

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

    // Find (or create) the state of the algorithm in this sub-slot
    auto& thisSubSlot = subSlots[ctx.subSlot()];
    auto  subitr      = thisSubSlot.find( iAlg->name() );
    if ( subitr == thisSubSlot.end() ) {
      thisSubSlot[iAlg->name()] = AlgExecState();
      return thisSubSlot[iAlg->name()];
    } else {
      return subitr->second;
    }
  }

  return itr->second;
}

//-----------------------------------------------------------------------------

const IAlgExecStateSvc::AlgStateMap_t& AlgExecStateSvc::algExecStates( const EventContext& ctx ) const {
  checkInit();
  return m_algStates.at( ctx.slot() );
}

//-----------------------------------------------------------------------------

const EventStatus::Status& AlgExecStateSvc::eventStatus( const EventContext& ctx ) const {
  checkInit();
  return m_eventStatus.at( ctx.slot() );
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::setEventStatus( const EventStatus::Status& sc, const EventContext& ctx ) {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  m_eventStatus.at( ctx.slot() ) = sc;
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::updateEventStatus( const bool& fail, const EventContext& ctx ) {
  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  auto& status = m_eventStatus.at( ctx.slot() );
  if ( status == EventStatus::Success ) {
    if ( fail ) status = EventStatus::AlgFail;
  } else if ( status == EventStatus::Invalid ) {
    status = ( fail ? EventStatus::AlgFail : EventStatus::Success );
  }
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::reset( const EventContext& ctx ) {
  if ( msgLevel( MSG::VERBOSE ) ) verbose() << "reset(" << ctx.slot() << ")" << endmsg;

  std::call_once( m_initFlag, &AlgExecStateSvc::init, this );
  for ( auto& e : m_algStates.at( ctx.slot() ) ) e.second.reset();

  // Also clear sub slots
  if ( ctx.slot() < m_algSubSlotStates.size() ) {
    for ( auto& subSlot : m_algSubSlotStates[ctx.slot()] ) {
      for ( auto& e : subSlot ) e.second.reset();
    }
  }

  m_eventStatus.at( ctx.slot() ) = EventStatus::Invalid;
}

//-----------------------------------------------------------------------------

unsigned int AlgExecStateSvc::algErrorCount( const IAlgorithm* iAlg ) const {
  auto itr = m_errorCount.find( iAlg->nameKey() );
  if ( itr == m_errorCount.end() ) {
    error() << "Unable to find Algorithm \"" << iAlg->name() << "\" in map"
            << " of ErrorCounts" << endmsg;
    return 0;
  }

  return itr->second;
}

//-----------------------------------------------------------------------------

void AlgExecStateSvc::resetErrorCount( const IAlgorithm* iAlg ) {
  auto itr = m_errorCount.find( iAlg->nameKey() );
  if ( itr != m_errorCount.end() ) {
    itr->second = 0;
  } else {
    error() << "Unable to find Algorithm \"" << iAlg->name() << "\" in map"
            << " of ErrorCounts" << endmsg;
  }
}

//-----------------------------------------------------------------------------

unsigned int AlgExecStateSvc::incrementErrorCount( const IAlgorithm* iAlg ) {
  auto itr = m_errorCount.find( iAlg->nameKey() );
  if ( itr == m_errorCount.end() ) {
    error() << "Unable to find Algorithm \"" << iAlg->name() << "\" in map"
            << " of ErrorCounts" << endmsg;
    return 0;
  }
  return ++( itr->second );
}
