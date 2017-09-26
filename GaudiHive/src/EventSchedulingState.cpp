#include "GaudiHive/EventSchedulingState.h"

EventSchedulingState::EventSchedulingState( const unsigned int& n_algos, const unsigned int& n_products )
    : m_numberOfAlgos( n_algos ), m_eventState( n_products )
{
  m_algosInFlight = 0;
  m_algosFinished = 0;
  m_algosStarted.resize( n_algos );
  std::fill( m_algosStarted.begin(), m_algosStarted.end(), false );
}

void EventSchedulingState::algoFinished()
{
  --m_algosInFlight;
  ++m_algosFinished;
}

bool EventSchedulingState::hasStarted( unsigned int& index ) const { return m_algosStarted[index]; }

void EventSchedulingState::algoStarts( unsigned int& index )
{
  ++m_algosInFlight;
  m_algosStarted[index] = true;
}

void EventSchedulingState::update_state( unsigned int& product_index ) { m_eventState[product_index] = true; }
