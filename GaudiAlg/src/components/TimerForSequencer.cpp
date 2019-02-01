// local
#include "TimerForSequencer.h"

//-----------------------------------------------------------------------------
// Implementation file for class : TimerForSequencer
//
// 2013-04-21 : Chris Jones
//-----------------------------------------------------------------------------

uint64_t TimerForSequencer::stop() {
  uint64_t cpuTime  = System::cpuTime( System::microSec ) - m_startCpu;
  uint64_t lastTime = System::currentTime( System::microSec ) - m_startClock;

  //== Update the counter
  m_num += 1ULL;
  m_sum += lastTime;
  m_sum2 += lastTime * lastTime;
  m_sumCpu += cpuTime;

  // Branchless update, only cast
  bool numIsFirst = ( 1ULL == m_num );
  m_min += lastTime * numIsFirst;
  m_max += lastTime * numIsFirst;

  m_min = lastTime < m_min ? lastTime : m_min;
  m_max = lastTime > m_max ? lastTime : m_max;

  m_lastTime = lastTime;
  m_lastCpu  = cpuTime;

  return lastTime;
}

MsgStream& TimerForSequencer::fillStream( MsgStream& s ) const {
  float ave = 0.f;
  float cpu = 0.f;

  if ( 0ULL != m_num ) {
    ave = m_sum / m_num;
    cpu = m_sumCpu / m_num;
  }

  ave *= m_factor;
  cpu *= m_factor;
  float min = m_min * m_factor;
  float max = m_max * m_factor;
  float sum = m_sum * m_factor;

  // Calculate the sigma with 2 momenta. ROOT histos call this quantity
  // RMS but just to be consistent with paw.
  // The division is by N-1 since one degree of freedom is used to calculate
  // the average. See your favourite book for the proof!
  float sigma = m_num <= 1ULL ? 0.f : m_factor * sqrt( ( m_sum2 - m_sum * m_sum / m_num ) / ( m_num - 1 ) );

  return s << m_name.substr( 0, m_size )
           << format( "| %9.3f | %9.3f | %8.3f %9.1f %8.2f | %7d | %9.3f |", cpu, ave, min, max, sigma, m_num,
                      sum * 0.001f );
}

std::string TimerForSequencer::header( std::string::size_type size ) {
  return "Algorithm" + std::string( std::max( std::string::size_type( 21 ), size ) - 20, ' ' ) +
         "(millisec) |    <user> |   <clock> |" + "      min       max    sigma | entries | total (s) |";
}
