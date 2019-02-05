#ifndef GAUDIHIVE_IOBOUNDALGSCHEDULERSVC_H
#define GAUDIHIVE_IOBOUNDALGSCHEDULERSVC_H

#include <functional>
#include <thread>

// Framework include files
#include "GaudiKernel/IAccelerator.h"
#include "GaudiKernel/IAlgTask.h"
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/Service.h"

// External libs
#include "tbb/concurrent_queue.h"

//---------------------------------------------------------------------------

/**@class IOBoundAlgSchedulerSvc IOBoundAlgSchedulerSvc.h GaudiHive/src/IOBoundAlgSchedulerSvc.h
 *
 * Please refer to the full documentation of the methods for more details.
 *
 *  @author  Illya Shapoval
 *  @version 1.0
 */
class IOBoundAlgSchedulerSvc : public extends<Service, IAccelerator> {
public:
  using extends::extends;

  /// Initialise
  StatusCode initialize() override;

  /// Finalise
  StatusCode finalize() override;

  /// Add an algorithm to local queue to run on accelerator
  StatusCode push( IAlgTask& task ) override;

private:
  // Utils and shortcuts ----------------------------------------------------

  /// Activate scheduler
  void activate();

  /// Deactivate scheduler
  StatusCode deactivate();

  /// Flag to track if the scheduler is active or not
  bool m_isActive = false;

  /// The thread in which the activate function runs
  std::thread m_thread;

  // Actions management -----------------------------------------------------

  typedef std::function<StatusCode()> action;
  /// This is done since the copy of the lambda storage is too expensive
  // typedef std::shared_ptr<action> actionPtr;
  /// Queue where closures are stored and picked for execution
  tbb::concurrent_bounded_queue<action> m_actionsQueue;
};

#endif // GAUDIHIVE_IOBOUNDALGSCHEDULERSVC_H
