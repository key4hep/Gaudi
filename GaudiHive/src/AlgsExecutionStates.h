/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIHIVE_ALGSEXECUTIONSTATES_H
#define GAUDIHIVE_ALGSEXECUTIONSTATES_H

// Framework include files
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/Service.h>

// C++ include files
#include <algorithm>
#include <initializer_list>
#include <string>
#include <vector>

// Boost include files
#include <boost/container/flat_set.hpp>

//---------------------------------------------------------------------------

/**@class AlgsExecutionStates AlgsExecutionStates.h GaudiKernel/AlgsExecutionStates.h
 *
 *  The AlgsExecutionStates encodes the state machine for the execution of
 *  algorithms within a single event. It is used by the concurrent schedulers
 *
    @author  Benedikt Hegner
 *  @author  Danilo Piparo
 *  @version 1.0
 */
class AlgsExecutionStates final {
public:
  /// Execution states of the algorithms
  /// Must have contiguous integer values 0, 1... N
  enum State : uint8_t {
    INITIAL      = 0,
    CONTROLREADY = 1,
    DATAREADY    = 2,
    RESOURCELESS = 3,
    SCHEDULED    = 4,
    EVTACCEPTED  = 5,
    EVTREJECTED  = 6,
    ERROR        = 7,
    MAXVALUE     = 8 // Allows loop over all states
  };

  AlgsExecutionStates( unsigned int algsNumber, SmartIF<IMessageSvc> MS )
      : m_states( algsNumber, INITIAL ), m_algsInState( MAXVALUE ), m_MS( std::move( MS ) ) {

    m_algsInState[INITIAL].reserve( algsNumber );
    for ( unsigned int i = 0; i < algsNumber; ++i ) m_algsInState[INITIAL].insert( i );
  }

  StatusCode set( unsigned int iAlgo, State newState );

  void reset() {
    std::fill( m_states.begin(), m_states.end(), INITIAL );

    for ( auto& algs : m_algsInState ) algs.clear();
    m_algsInState[INITIAL].reserve( m_states.size() );
    for ( unsigned int i = 0; i < m_states.size(); ++i ) m_algsInState[INITIAL].insert( i );
  }

  /// check if the collection contains at least one state of requested type
  bool contains( State state ) const { return m_algsInState[state].size() > 0; }

  /// check if the collection contains at least one state of any listed types
  bool containsAny( std::initializer_list<State> l ) const {
    for ( auto state : l )
      if ( m_algsInState[state].size() > 0 ) return true;
    return false;
  }

  // copy the current set of algs in a particular state
  // states change during scheduler loop over set, so cannot return reference
  const boost::container::flat_set<int> algsInState( State state ) const { return m_algsInState[state]; }

  const State& operator[]( unsigned int i ) const { return m_states.at( i ); }

  size_t size() const { return m_states.size(); }

  size_t sizeOfSubset( State state ) const { return m_algsInState[state].size(); }

private:
  std::vector<State>                           m_states;
  std::vector<boost::container::flat_set<int>> m_algsInState;
  SmartIF<IMessageSvc>                         m_MS;

  MsgStream log() { return { m_MS, "AlgsExecutionStates" }; }
};

/// Streaming of State values
inline std::ostream& operator<<( std::ostream& s, AlgsExecutionStates::State x ) {
  using State = AlgsExecutionStates::State;
  switch ( x ) {
  case State::INITIAL:
    s << "INITIAL";
    break;
  case State::CONTROLREADY:
    s << "CONTROLREADY";
    break;
  case State::DATAREADY:
    s << "DATAREADY";
    break;
  case State::RESOURCELESS:
    s << "RESOURCELESS";
    break;
  case State::SCHEDULED:
    s << "SCHEDULED";
    break;
  case State::EVTACCEPTED:
    s << "EVTACCEPTED";
    break;
  case State::EVTREJECTED:
    s << "EVTREJECTED";
    break;
  case State::ERROR:
    s << "ERROR";
    break;
  default:
    s << "UNKNOWN";
  }
  return s;
}

#endif // GAUDIHIVE_ALGSEXECUTIONSTATES_H
