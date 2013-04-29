// $Id: TimerForSequencer.h,v 1.5 2004/11/25 13:26:26 mato Exp $
#ifndef TIMERFORSEQUENCER_H

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Timing.h"
#include <inttypes.h> 

/** Auxilliary class. Measure the time between start and stop, and compute
 *  average, min and max. In fact, measure the cpu time, and the elapsed time
 *  but givesmin/max only for elapsed.
 *
 * Avoid usage of double precision floating point to cumulate counts and use
 * unsigned long long integers instead. The total capacity is enough (~1.8e19)
 * and there is no risk to loose counts when the available sum is big with
 * respect to the added duration. The sigma 
 * 
 * @author O.Callot
 * @author D.Piparo
 */

class TimerForSequencer {

public:
  /** Constructor. Specify the name, for later printing. */

  TimerForSequencer( std::string name, double factor ):
  m_name(name),
  m_factor(factor),
  m_startClock(0ULL),
  m_startCpu(0ULL),
  m_num(0ULL),
  m_lastTime(0ULL),
  m_lastCpu(0ULL),
  m_min(0ULL),
  m_max(0ULL),
  m_sum(0ULL),
  m_sum2(0ULL),
  m_sumCpu(0ULL){}
    
  ~TimerForSequencer() {};

  /** Start a time measurement */
  void start () {
    m_startClock = System::currentTime( System::microSec );
    m_startCpu   = System::cpuTime( System::microSec );
  }

  /** Stop time measurement and return the last elapsed time.
      @return Measured time in ms
  */
  uint64_t stop() {
    uint64_t cpuTime  =  System::cpuTime( System::microSec ) - m_startCpu ;
    uint64_t lastTime =  System::currentTime( System::microSec ) - m_startClock ;

    //== Update the counter
    m_num    += 1ULL;
    m_sum    += lastTime;
    m_sum2   += lastTime*lastTime;
    m_sumCpu += cpuTime;

    // Branchless update, only cast
    bool  numIsFirst= (1ULL == m_num) ;
    m_min += lastTime * numIsFirst;
    m_max += lastTime * numIsFirst;

    m_min = lastTime < m_min ? lastTime : m_min;
    m_max = lastTime > m_max ? lastTime : m_max;

    m_lastTime = lastTime;
    m_lastCpu  = cpuTime;

    return lastTime;
  }

  /** returns the name **/
  std::string name() const { return m_name; }

  /** returns the last measured time **/
  double lastTime()   const { return m_lastTime*m_factor; }

  /** returns the last measured time **/
  double lastCpu()   const { return m_lastCpu*m_factor; }

  /** returns the total elapsed time */
  double elapsedTotal() 
  {
    return m_sum*m_factor;
  }

  /** returns the toptal cpu time */
  double cpuTotal() 
  {
    return m_sumCpu*m_factor;
  }

  /** Returns the number run count*/
  uint64_t count() 
  {
    return m_num;
  }

  /** Write measured time into the message stream. */
  MsgStream & fillStream(MsgStream & s) const {
    float ave = 0.f;
    float cpu = 0.f;

    if ( 0ULL != m_num ) {
      ave = m_sum    / m_num;
      cpu = m_sumCpu / m_num;
    }
    
    ave*=m_factor;
    cpu*=m_factor;
    float min = m_min * m_factor;
    float max = m_max * m_factor;
    float sum = m_sum * m_factor;

    // Calculate the sigma with 2 momenta. ROOT histos call this quantity
    // RMS but just to be consistent with paw.
    // The division is by N-1 since one degree of freedom is used to calculate
    // the average. See your favourite book for the proof!
    float  sigma = m_num <= 1ULL ? 0.f : m_factor * sqrt( (m_sum2 - m_sum*m_sum/m_num)/(m_num-1) );
  
    return s << m_name
             << format( "| %9.3f | %9.3f | %8.3f %9.1f %8.2f | %7d | %9.3f |",
                        cpu, ave, min, max, sigma, m_num, sum * 0.001f );
  }

  /** header matching the previous format **/
  static std::string header( std::string::size_type size ) {
    if ( size < 21 ) size = 21;
    std::string blank( size - 20, ' ' );
    std::string s = "Algorithm" + blank + "(millisec) |    <user> |   <clock> |";
    s += "      min       max    sigma | entries | total (s) |";
    return s;
  }

private:
  std::string m_name;
  double m_factor;
  uint64_t m_startClock;
  uint64_t m_startCpu;

  uint64_t m_num;
  uint64_t m_lastTime;
  uint64_t m_lastCpu;
  uint64_t m_min;
  uint64_t m_max;
  uint64_t m_sum;
  uint64_t m_sum2;  
  uint64_t m_sumCpu;

};

inline MsgStream& operator<<(MsgStream& ms, const TimerForSequencer& count) {
  return count.fillStream( ms );
}

#endif // TIMERFORSEQUENCER_H
