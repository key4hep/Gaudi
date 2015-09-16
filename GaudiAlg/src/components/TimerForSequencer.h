#ifndef TIMERFORSEQUENCER_H

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Timing.h"

/** Auxilliary class. Measure the time between start and stop, and compute
 *  average, min and max. In fact, measure the cpu time, and the elapsed time
 *  but givesmin/max only for elapsed.
 *
 * @author O.Callot
 */

class TimerForSequencer
{

public:

  /** Constructor. Specify the name, for later printing. */
  TimerForSequencer( std::string name,
                     unsigned int size,
                     double factor )
    : m_name     ( std::move(name) ),
      m_size     ( size ),
      m_factor   ( factor )
  { }

  ~TimerForSequencer() = default;

  /** Start a time measurement */
  inline void start ()
  {
    m_startClock = System::currentTime( System::microSec );
    m_startCpu   = System::cpuTime    ( System::microSec );
  }

  /** Stop time measurement and return the last elapsed time.
      @return Measured time in ms
  */
  double stop();

  /** returns the name **/
  inline const std::string& name() const { return m_name; }

  /** returns the last measured time **/
  inline double lastTime()   const { return m_lastTime; }

  /** returns the last measured time **/
  inline double lastCpu()   const { return m_lastCpu; }

  /** returns the total elapsed time */
  inline double elapsedTotal() const { return m_sum; }

  /** returns the total cpu time */
  inline double cpuTotal() const { return m_sumCpu; }

  /** Returns the number run count*/
  inline double count() const { return m_num; }

  /** Write measured time into the message stream. */
  MsgStream & fillStream(MsgStream & s) const;

  /** header matching the previous format **/
  static std::string header( std::string::size_type size );

private:

  std::string m_name;
  unsigned int m_size;
  double m_factor;
  long long m_startClock = 0LL ;
  long long m_startCpu =  0LL ;

  long long m_num =  0LL;
  double m_lastTime = 0.;
  double m_lastCpu = 0.;
  double m_min = 0.;
  double m_max = 0.;
  double m_sum = 0.;
  double m_sumCpu = 0.;
};

inline MsgStream& operator<<(MsgStream& ms, const TimerForSequencer& count)
{
  return count.fillStream( ms );
}

#endif // TIMERFORSEQUENCER_H
