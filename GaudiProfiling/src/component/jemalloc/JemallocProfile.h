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
class JemallocProfile : public GaudiAlgorithm {
public: 
  /// Standard constructor
  JemallocProfile( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~JemallocProfile( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

private:
  int m_nStartFromEvent; // Event to start profiling at
  int m_nStopAtEvent;  // Event to stop profiling at
  bool m_profiling; // whether we are profiling...
  int m_dumpPeriod;  // Period at which to dump the heap
  int m_eventNumber;   // Current event number
};
#endif // JEMALLOC_PROFILE_H
