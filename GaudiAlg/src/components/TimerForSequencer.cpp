// local
#include "TimerForSequencer.h"

//-----------------------------------------------------------------------------
// Implementation file for class : TimerForSequencer
//
// 2013-04-21 : Chris Jones
//-----------------------------------------------------------------------------

double TimerForSequencer::stop()
{
  double cpuTime  =  double(System::cpuTime( System::microSec ) - m_startCpu );
  double lastTime =  double(System::currentTime( System::microSec ) - m_startClock );

  //== Change to normalized millisecond
  cpuTime  *= m_factor;
  lastTime *= m_factor;

  //== Update the counter
  m_num    += 1;
  m_sum    += lastTime;
  m_sumCpu += cpuTime;

  if ( 1 == m_num )
  {
    m_min = lastTime;
    m_max = lastTime;
  }
  else
  {
    if ( lastTime < m_min ) m_min = lastTime;
    if ( lastTime > m_max ) m_max = lastTime;
  }
  m_lastTime = lastTime;
  m_lastCpu  = cpuTime;
  return lastTime;
}

MsgStream & TimerForSequencer::fillStream(MsgStream & s) const
{
  double ave = 0.;
  double cpu = 0.;

  if ( 0 != m_num )
  {
    ave = m_sum    / m_num;
    cpu = m_sumCpu / m_num;
  }

  return s << m_name.substr(0,m_size)
           << format( "| %9.3f | %9.3f | %8.3f %9.1f | %7d | %9.3f |",
                      cpu, ave, m_min, m_max, m_num, m_sum * 0.001 );
}

std::string TimerForSequencer::header( std::string::size_type size )
{
  return  "Algorithm" + std::string( std::max(std::string::size_type(21),size) - 20, ' ' )
        + "(millisec) |    <user> |   <clock> |" 
        + "      min       max | entries | total (s) |";
}
