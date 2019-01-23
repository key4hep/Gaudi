#ifndef JEMALLOC_PROFILE_H
#define JEMALLOC_PROFILE_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

/** @class JemallocProfile JemallocProfile.h jemalloc/JemallocProfile.h
 *
 * Algorithm to enable/disable the profiling of the head by Jemalloc.
 * For this to run, you must set the env variables:
 * LD_PRELOAD=<jemalloc lib>
 * MALLOC_CONF=prof:true,prof_leak:true,prof_prefix:<dumpprefix>
 *
 *
 *  @author Ben Couturier
 *  @date   2015-06-09
 */
class JemallocProfile : public GaudiAlgorithm
{
public:
  /// Standard constructor
  using GaudiAlgorithm::GaudiAlgorithm;

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization

private:
  Gaudi::Property<int> m_nStartFromEvent{this, "StartFromEventN", 1, "After what event we start profiling. "};
  Gaudi::Property<int> m_nStopAtEvent{
      this, "StopAtEventN", 0,
      "After what event we stop profiling. If 0 than we also profile finalization stage. Default = 0."};
  Gaudi::Property<int> m_dumpPeriod{this, "DumpPeriod", 100, "Period for dumping head to a file. Default=100"};

  bool m_profiling   = false; // whether we are profiling...
  int  m_eventNumber = 0;     // Current event number
};
#endif // JEMALLOC_PROFILE_H
