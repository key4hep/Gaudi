/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
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
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"

// C++ include files
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <string>
#include <vector>

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
  enum State : uint8_t {
    INITIAL      = 0,
    CONTROLREADY = 1,
    DATAREADY    = 2,
    RESOURCELESS = 3,
    SCHEDULED    = 4,
    EVTACCEPTED  = 5,
    EVTREJECTED  = 6,
    ERROR        = 7
  };

  AlgsExecutionStates( unsigned int algsNumber, SmartIF<IMessageSvc> MS )
      : m_states( algsNumber, INITIAL ), m_MS( std::move( MS ) ){};

  StatusCode set( unsigned int iAlgo, State newState );

  void reset() { std::fill( m_states.begin(), m_states.end(), INITIAL ); };

  /// check if the collection contains at least one state of requested type
  bool contains( State state ) const { return std::find( m_states.begin(), m_states.end(), state ) != m_states.end(); }

  /// check if the collection contains at least one state of any listed types
  bool containsAny( std::initializer_list<State> l ) const {
    return std::find_first_of( m_states.begin(), m_states.end(), l.begin(), l.end() ) != m_states.end();
  }

  /// check if the collection contains only states of listed types
  bool containsOnly( std::initializer_list<State> l ) const {
    return std::all_of( m_states.begin(), m_states.end(),
                        [l]( State s ) { return std::find( l.begin(), l.end(), s ) != l.end(); } );
  };

  const State& operator[]( unsigned int i ) const { return m_states.at( i ); };

  size_t size() const { return m_states.size(); }

  size_t sizeOfSubset( State state ) const {
    return std::count_if( m_states.begin(), m_states.end(), [&]( State s ) { return s == state; } );
  }

private:
  std::vector<State>   m_states;
  SmartIF<IMessageSvc> m_MS;

  MsgStream log() { return {m_MS, "AlgsExecutionStates"}; }

public:
  class Iterator final : public std::iterator<std::forward_iterator_tag, uint> {
    auto find_valid( std::vector<State>::const_iterator iter ) const { return std::find( iter, m_v->end(), m_s ); }

  public:
    Iterator( State s, const std::vector<State>& v, std::vector<State>::const_iterator pos )
        : m_s( s ), m_v( &v ), m_pos( find_valid( pos ) ) {}

    friend bool operator==( const Iterator& lhs, const Iterator& rhs ) {
      return lhs.m_s == rhs.m_s && lhs.m_v == rhs.m_v && lhs.m_pos == rhs.m_pos;
    }

    friend bool operator!=( const Iterator& lhs, const Iterator& rhs ) { return !( lhs == rhs ); }

    Iterator& operator++() {
      if ( m_pos != m_v->end() ) m_pos = find_valid( std::next( m_pos ) );
      return *this;
    }

    Iterator& operator++( int ) { return ++( *this ); }

    uint operator*() { return std::distance( m_v->begin(), m_pos ); }

  private:
    State                              m_s;
    const std::vector<State>*          m_v;
    std::vector<State>::const_iterator m_pos;
  };

  Iterator begin( State kind ) { return {kind, m_states, m_states.begin()}; }
  Iterator end( State kind ) { return {kind, m_states, m_states.end()}; }
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
