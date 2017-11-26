#ifndef VALGRIND_CALLGRINDPROFILE_H
#define VALGRIND_CALLGRINDPROFILE_H 1

// Include files
// from Gaudi
#include "GaudiKernel/Algorithm.h"

/** @class CallgrindProfile CallgrindProfile.h valgrind/CallgrindProfile.h
 *
 * Algorithm to enable/disable the profiling by Callgrind at given events.
 *
 *  @author Ben Couturier
 *  @date   2014-08-22
 */
class CallgrindProfile : public Algorithm
{
public:
  using Algorithm::Algorithm;

  StatusCode execute() override;  ///< Algorithm execution
  StatusCode finalize() override; ///< Algorithm finalization

private:
  Gaudi::Property<int> m_nStartFromEvent{this, "StartFromEventN", 1, "After what event we start profiling."};
  Gaudi::Property<int> m_nStopAtEvent{
      this, "StopAtEventN", 0,
      "After what event we stop profiling. If 0 than we also profile finalization stage. Default = 0."};
  Gaudi::Property<int> m_nDumpAtEvent{
      this, "DumpAtEventN", 0,
      "After what event we stop profiling. If 0 than we also profile finalization stage. Default = 0."};
  Gaudi::Property<int> m_nZeroAtEvent{
      this, "ZeroAtEventN", 0,
      "After what event we stop profiling. If 0 than we also profile finalization stage. Default = 0."};
  Gaudi::Property<std::string> m_dumpName{this, "DumpName", "", "Label for the callgrind dump"};

  int m_eventNumber = 0;     // Current event number
  bool m_profiling  = false; // Whether valgrind is profiling or not
  bool m_dumpDone   = false; // Whether the counters were dumped
};
#endif // VALGRIND_CALLGRINDPROFILE_H
