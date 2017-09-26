#ifndef GAUDIHIVE_ALGSEXECUTIONSTATES_H
#define GAUDIHIVE_ALGSEXECUTIONSTATES_H

// Framework include files
#include "GaudiKernel/Service.h"

// C++ include files
#include <functional>
#include <iterator>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <boost/dynamic_bitset.hpp>

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
class AlgsExecutionStates
{
public:
  /// Execution states of the algorithms
  enum State : unsigned short {
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
      : m_states( algsNumber, INITIAL ), m_MS( MS ){};

  ~AlgsExecutionStates(){};

  StatusCode updateState( unsigned int iAlgo, State newState );

  void reset() { m_states.assign( m_states.size(), INITIAL ); };

  bool algsPresent( State state ) const
  {
    return std::find( m_states.begin(), m_states.end(), state ) != m_states.end();
  }

  bool allAlgsExecuted()
  {
    int execAlgos = std::count_if( m_states.begin(), m_states.end(),
                                   []( State s ) { return ( s == EVTACCEPTED || s == EVTREJECTED ); } );
    return m_states.size() == (unsigned int)execAlgos;
  };

  const State& operator[]( unsigned int i ) const { return m_states.at( i ); };

  size_t size() const { return m_states.size(); }

  size_t sizeOfSubset( State state ) const
  {
    return std::count_if( m_states.begin(), m_states.end(), [&]( State s ) { return ( s == state ); } );
  }

private:
  std::vector<State> m_states;
  SmartIF<IMessageSvc> m_MS;

public:
  class Iterator : public std::iterator<std::forward_iterator_tag, uint>
  {

  public:
    enum POS { BEGIN, END };

    Iterator( POS pos, State s, const std::vector<State>& v ) : s_( s ), v_( &v )
    {
      if ( pos == POS::BEGIN ) pos_ = std::find( v_->begin(), v_->end(), s_ );
      if ( pos == POS::END ) pos_   = v_->end();
      // std::cout << "initialized iterator at " << pos_ << std::endl;
    }

    ~Iterator() {}

    Iterator& operator=( const Iterator& other )
    {
      pos_ = other.pos_;
      v_   = other.v_;
      s_   = other.s_;
      return ( *this );
    }

    bool operator==( const Iterator& other ) { return pos_ == other.pos_ && s_ == other.s_ && v_ == other.v_; }

    bool operator!=( const Iterator& other ) { return pos_ != other.pos_ || s_ != other.s_ || v_ != other.v_; }

    Iterator& operator++()
    {
      if ( pos_ != v_->end() ) {
        pos_ = std::find( ++pos_, v_->end(), s_ );
        // std::cout << "advanced iterator to " << pos_ << std::endl;
      }
      return ( *this );
    }

    Iterator& operator++( int ) { return ( ++( *this ) ); }

    uint operator*() { return std::distance( v_->begin(), pos_ ); }

  private:
    std::vector<State>::const_iterator pos_;
    State s_;
    const std::vector<State>* v_;
  };

  Iterator begin( State kind ) { return ( Iterator( Iterator::POS::BEGIN, kind, m_states ) ); }

  Iterator end( State kind ) { return ( Iterator( Iterator::POS::END, kind, m_states ) ); }
};

/// Streaming of State values (required by C++11 scoped enums).
inline std::ostream& operator<<( std::ostream& s, AlgsExecutionStates::State x )
{
  return s << static_cast<unsigned short>( x );
}

#endif // GAUDIHIVE_ALGSEXECUTIONSTATES_H
