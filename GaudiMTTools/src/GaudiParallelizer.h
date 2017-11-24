#ifndef LIB_GAUDIPARALLELIZER_H
#define LIB_GAUDIPARALLELIZER_H 1
// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include <boost/bind.hpp>
#include <tbb/task_group.h>
#include <tbb/task_scheduler_init.h>

// Forward declarations
class ISequencerTimerTool;

/** @class GaudiParallelizer GaudiParallelizer.h
  *
  *
  * @author Illya Shapoval
  * @date 09/12/2011
  */
class GaudiParallelizer : public GaudiAlgorithm
{
public:
  /// Standard constructor
  GaudiParallelizer( const std::string& name, ISvcLocator* pSvcLocator );
  /// Destructor. An explicit noexcept(true) is necessary for Gaudi to build (see GAUDI-1187)
  ~GaudiParallelizer() noexcept( true ) override {}
  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization

  /** for asynchronous changes in the list of algorithms */
  void membershipHandler( Gaudi::Details::PropertyBase& theProp );

protected:
  class AlgorithmEntry
  {
  public:
    /// Standard constructor
    AlgorithmEntry( Algorithm* alg )
    {
      m_algorithm = alg;
      m_reverse   = false;
      m_timer     = 0;
    }

    virtual ~AlgorithmEntry(){}; ///< Destructor
    void setReverse( bool flag ) { m_reverse = flag; }

    Algorithm* algorithm() const { return m_algorithm; }
    bool reverse() const { return m_reverse; }
    void setTimer( int nb ) { m_timer = nb; }
    int timer() const { return m_timer; }

    /// Thread task executor method to wrap an algorithm execution in
    void run( GaudiParallelizer& prlzr )
    {
      if ( prlzr.m_measureTime ) prlzr.m_timerTool->start( timer() );
      m_returncode = m_algorithm->sysExecute( Gaudi::Hive::currentContext() );
      if ( prlzr.m_measureTime ) prlzr.m_timerTool->stop( timer() );
    }

    StatusCode m_returncode; ///< StatusCode of an algorithm execution received from a thread

  private:
    Algorithm* m_algorithm; ///< Algorithm pointer
    bool m_reverse;         ///< Indicates that the flag has to be inverted
    int m_timer;            ///< Timer number for this algorithm
  };

  /** Decode a vector of string. */
  StatusCode decodeNames();

private:
  Gaudi::Property<std::vector<std::string>> m_names{this, "Members", {}, "list of algorithms"};
  Gaudi::Property<bool> m_modeOR{this, "ModeOR", false, "use OR loginc instead of AND"};
  Gaudi::Property<bool> m_measureTime{this, "MeasureTime", false, "measure time"};
  Gaudi::Property<bool> m_returnOK{this, "ReturnOK", false, "forces the sequencer to return a good status"};
  Gaudi::Property<unsigned short> m_nthreads{this, "NumberOfThreads", 0, "number of threads in the thread pool"};

  std::vector<AlgorithmEntry> m_entries; ///< List of algorithms to process.

  ISequencerTimerTool* m_timerTool = nullptr; ///< Pointer to the timer tool
  int m_timer;                                ///< Timer number for the sequencer

  tbb::task_group m_task_group; ///< TBB task group
};

#endif // LIB_GAUDIPARALLELIZER_H
