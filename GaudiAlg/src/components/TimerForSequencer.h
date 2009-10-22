// $Id: TimerForSequencer.h,v 1.5 2004/11/25 13:26:26 mato Exp $
#ifndef TIMERFORSEQUENCER_H 

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Timing.h"

/** Auxilliary class. Measure the time between start and stop, and compute
 *  average, min and max. In fact, measure the cpu time, and the elapsed time
 *  but givesmin/max only for elapsed.
 *
 * @author O.Callot
 */

class TimerForSequencer {

public:
  /** Constructor. Specify the name, for later printing. */

  TimerForSequencer( std::string name, double factor ) {
    m_name   = name;
    m_num    = 0;
    m_min    = 0;
    m_max    = 0;
    m_sum    = 0;
    m_sumCpu = 0;
    m_factor = factor;
    m_lastTime = 0.;
    m_lastCpu  = 0.;
  }

  ~TimerForSequencer() {};

  /** Start a time measurement */
  void start () {
    m_startClock = System::currentTime( System::microSec );
    m_startCpu   = System::cpuTime( System::microSec );
  }
  
  /** Stop time measurement and return the last elapsed time.
      @return Measured time in ms
  */
  double stop() {
    double cpuTime  =  double(System::cpuTime( System::microSec ) - m_startCpu );
    double lastTime =  double(System::currentTime( System::microSec ) - m_startClock );

    //== Change to normalized millisecond
    cpuTime  *= m_factor;
    lastTime *= m_factor;
    
    //== Update the counter
    m_num    += 1;
    m_sum    += lastTime;
    m_sumCpu += cpuTime;
    
    if ( 1 == m_num ) {
      m_min = lastTime;
      m_max = lastTime;
    } else {
      if ( lastTime < m_min ) m_min = lastTime;
      if ( lastTime > m_max ) m_max = lastTime;
    }
    m_lastTime = lastTime;
    m_lastCpu  = cpuTime;
    return lastTime;
  }

  /** returns the name **/
  std::string name() const { return m_name; }

  /** returns the last measured time **/
  double lastTime()   const { return m_lastTime; }

  /** returns the last measured time **/
  double lastCpu()   const { return m_lastCpu; }

  /** Write measured time into the message stream. */
  MsgStream & fillStream(MsgStream & s) const {
    double ave = 0.;
    double cpu = 0.;
    
    if ( 0 != m_num ) {
      ave = m_sum    / m_num;
      cpu = m_sumCpu / m_num;
    }
    
    return s << m_name 
             << format( "| %9.3f | %9.3f | %8.3f %9.1f | %7d | %9.3f |",
                        cpu, ave, m_min, m_max, m_num, m_sum * 0.001 );
  }

  /** header matching the previous format **/
  std::string header( ) const {
    std::string s = "Algorithm          (millisec) |    <user> |   <clock> |";
    s += "      min       max | entries | total (s) |";
    return s;
  }

private:
  std::string m_name;
  double m_factor;
  longlong m_startClock;
  longlong m_startCpu;

  long m_num;
  double m_lastTime;
  double m_lastCpu;
  double m_min;
  double m_max;
  double m_sum;
  double m_sumCpu;
};

inline MsgStream& operator<<(MsgStream& ms, const TimerForSequencer& count) {
  return count.fillStream( ms );
}

#endif // TIMERFORSEQUENCER_H
