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
  /// Standard constructor
  CallgrindProfile( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~CallgrindProfile( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

private:
  int m_nStartFromEvent; // Event to start profiling at
  int m_nStopAtEvent;  // Event to stop profiling at
  int m_nDumpAtEvent; // Event at which to dump the stats (Destructor by default)
  int m_nZeroAtEvent; //Event at which to zero the stats
  int m_eventNumber;   // Current event number
  bool m_profiling; // Whether valgrind is profiling or not
  bool m_dumpDone;  // Whether the counters were dumped
  std::string m_dumpName; // Name to pass to the dump macro
};
#endif // VALGRIND_CALLGRINDPROFILE_H
