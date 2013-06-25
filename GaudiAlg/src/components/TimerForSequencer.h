// $Id: TimerForSequencer.h,v 1.5 2004/11/25 13:26:26 mato Exp $
#ifndef TIMERFORSEQUENCER_H

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Timing.h"
#include <inttypes.h>

/** Auxiliary class. Measure the time between start and stop, and compute
 *  average, min and max. In fact, measure the cpu time, and the elapsed time
 *  but gives min/max only for elapsed.
 *
 * Avoid usage of double precision floating point to cumulate counts and use
 * unsigned long long integers instead. The total capacity is enough (~1.8e19)
 * and there is no risk to loose counts when the available sum is big with
 * respect to the added duration. The sigma
 *
 * @author O.Callot
 * @author D.Piparo
 */

class TimerForSequencer
{

public:
  /** Constructor. Specify the name, for later printing. */

  TimerForSequencer( std::string name,
                     const unsigned int size,
                     double factor )
    : m_name        ( name   ),
      m_size        ( size   ),
      m_factor      ( factor ),
      m_startClock  ( 0ULL   ),
      m_startCpu    ( 0ULL   ),
      m_num         ( 0ULL   ),
      m_lastTime    ( 0ULL   ),
      m_lastCpu     ( 0ULL   ),
      m_min         ( 0ULL   ),
      m_max         ( 0ULL   ),
      m_sum         ( 0ULL   ),
      m_sum2        ( 0ULL   ),
      m_sumCpu      ( 0ULL   )
  { }

  ~TimerForSequencer() {}

  /** Start a time measurement */
  void start ()
  {
    m_startClock = System::currentTime( System::microSec );
    m_startCpu   = System::cpuTime    ( System::microSec );
  }

  /** Stop time measurement and return the last elapsed time.
      @return Measured time in ms
  */
  uint64_t stop();

  /** returns the name **/
  inline const std::string& name() const { return m_name; }

  /** returns the last measured time **/
  inline double lastTime() const { return m_lastTime * m_factor; }

  /** returns the last measured time **/
  inline double lastCpu() const { return m_lastCpu * m_factor; }

  /** returns the total elapsed time */
  inline double elapsedTotal() const { return m_sum * m_factor; }

  /** returns the toptal cpu time */
  inline double cpuTotal() const { return m_sumCpu * m_factor; }

  /** Returns the number run count*/
  inline uint64_t count() const { return m_num; }

  /** Write measured time into the message stream. */
  MsgStream & fillStream(MsgStream & s) const;

  /** header matching the previous format **/
  static std::string header( std::string::size_type size );

private:
  std::string m_name;
  unsigned int m_size;
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

inline MsgStream& operator<<(MsgStream& ms, const TimerForSequencer& count)
{
  return count.fillStream( ms );
}

#endif // TIMERFORSEQUENCER_H
