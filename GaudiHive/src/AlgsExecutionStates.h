#ifndef GAUDIHIVE_ALGSEXECUTIONSTATES_H
#define GAUDIHIVE_ALGSEXECUTIONSTATES_H

// Framework include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"

// C++ include files
#include <cstdint>
#include <functional>
#include <iterator>
#include <map>
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
class AlgsExecutionStates final
{
public:
  /// Execution states of the algorithms
  enum State : uint8_t {
    INITIAL      = 0,
    CONTROLREADY = 1,
    DATAREADY    = 2,
    SCHEDULED    = 3,
    EVTACCEPTED  = 4,
    EVTREJECTED  = 5,
    ERROR        = 6
  };

  static std::map<State, std::string> stateNames;

  AlgsExecutionStates( unsigned int algsNumber, SmartIF<IMessageSvc> MS )
      : m_states( algsNumber, INITIAL ), m_MS( std::move( MS ) ){};

  StatusCode updateState( unsigned int iAlgo, State newState );

  void reset() { std::fill( m_states.begin(), m_states.end(), INITIAL ); };

  bool algsPresent( State state ) const
  {
    return std::find( m_states.begin(), m_states.end(), state ) != m_states.end();
  }

  bool allAlgsExecuted()
  {
    return std::all_of( m_states.begin(), m_states.end(),
                        []( State s ) { return s == EVTACCEPTED || s == EVTREJECTED; } );
  };

  const State& operator[]( unsigned int i ) const { return m_states.at( i ); };

  size_t size() const { return m_states.size(); }

  size_t sizeOfSubset( State state ) const
  {
    return std::count_if( m_states.begin(), m_states.end(), [&]( State s ) { return s == state; } );
  }

private:
  std::vector<State> m_states;
  SmartIF<IMessageSvc> m_MS;

  MsgStream log() { return {m_MS, "AlgsExecutionStates"}; }

public:
  class Iterator final : public std::iterator<std::forward_iterator_tag, uint>
  {
    auto find_valid( std::vector<State>::const_iterator iter ) const { return std::find( iter, m_v->end(), m_s ); }

  public:
    Iterator( State s, const std::vector<State>& v, std::vector<State>::const_iterator pos )
        : m_s( s ), m_v( &v ), m_pos( find_valid( pos ) )
    {
    }

    friend bool operator==( const Iterator& lhs, const Iterator& rhs )
    {
      return lhs.m_s == rhs.m_s && lhs.m_v == rhs.m_v && lhs.m_pos == rhs.m_pos;
    }

    friend bool operator!=( const Iterator& lhs, const Iterator& rhs ) { return !( lhs == rhs ); }

    Iterator& operator++()
    {
      if ( m_pos != m_v->end() ) m_pos = find_valid( std::next( m_pos ) );
      return *this;
    }

    Iterator& operator++( int ) { return ++( *this ); }

    uint operator*() { return std::distance( m_v->begin(), m_pos ); }

  private:
    State m_s;
    const std::vector<State>* m_v;
    std::vector<State>::const_iterator m_pos;
  };

  Iterator begin( State kind ) { return {kind, m_states, m_states.begin()}; }
  Iterator end( State kind ) { return {kind, m_states, m_states.end()}; }
};

/// Streaming of State values (required by C++11 scoped enums).
inline std::ostream& operator<<( std::ostream& s, AlgsExecutionStates::State x )
{
  return s << static_cast<std::underlying_type_t<AlgsExecutionStates::State>>( x );
}

#endif // GAUDIHIVE_ALGSEXECUTIONSTATES_H
