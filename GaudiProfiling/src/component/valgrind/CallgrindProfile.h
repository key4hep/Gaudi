#ifndef VALGRIND_CALLGRINDPROFILE_H
#define VALGRIND_CALLGRINDPROFILE_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"


/** @class CallgrindProfile CallgrindProfile.h valgrind/CallgrindProfile.h
 *
 * Algorithm to enable/disable the profiling by Callgrind at given events.
 *
 *  @author Ben Couturier
 *  @date   2014-08-22
 */
class CallgrindProfile : public GaudiAlgorithm {
public:
  using GaudiAlgorithm::GaudiAlgorithm;

  ~CallgrindProfile() override = default; ///< Destructor

  StatusCode initialize() override;    ///< Algorithm initialization
  StatusCode execute   () override;    ///< Algorithm execution
  StatusCode finalize  () override;    ///< Algorithm finalization

private:

  IntegerProperty  m_nStartFromEvent {this, "StartFromEventN",  1,  "After what event we start profiling."};
  IntegerProperty  m_nStopAtEvent {this, "StopAtEventN",  0,  "After what event we stop profiling. If 0 than we also profile finalization stage. Default = 0."};
  IntegerProperty  m_nDumpAtEvent {this, "DumpAtEventN",  0,  "After what event we stop profiling. If 0 than we also profile finalization stage. Default = 0."};
  IntegerProperty  m_nZeroAtEvent {this, "ZeroAtEventN",  0,  "After what event we stop profiling. If 0 than we also profile finalization stage. Default = 0."};
  StringProperty  m_dumpName {this, "DumpName",  "",  "Label for the callgrind dump"};

  int m_eventNumber = 0;   // Current event number
  bool m_profiling = false; // Whether valgrind is profiling or not
  bool m_dumpDone = false;  // Whether the counters were dumped
};
#endif // VALGRIND_CALLGRINDPROFILE_H
