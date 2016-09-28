#ifndef GAUDIHIVE_SEQUENTIALSCHEDULERSVC_H
#define GAUDIHIVE_SEQUENTIALSCHEDULERSVC_H

// Framework include files
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/Service.h"

// C++ include files
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// External libs
#include "tbb/concurrent_queue.h"

//---------------------------------------------------------------------------

/**@class SequentialSchedulerSvc SequentialSchedulerSvc.h
 *
 *  This SchedulerSvc implements the IScheduler interface.
 * It executes all the algorithms in sequence, pulling them from the
 * AlgResourcePool. No task level parallelism is involved.
 * Given its sequential nature, the scheduler does not run in its own thread.
 * It has therefore to be treated a as a THREAD UNSAFE and NON REENTRANT
 * entity.
 *
 *  @author  Danilo Piparo
 *  @author  Benedikt Hegner
 *  @version 1.1
 */
class SequentialSchedulerSvc : public extends<Service, IScheduler>
{
public:
  /// Constructor
  using extends::extends;

  /// Destructor
  ~SequentialSchedulerSvc() override = default;

  /// Initialise
  StatusCode initialize() override;

  /// Finalise
  StatusCode finalize() override;

  /// Make an event available to the scheduler
  StatusCode pushNewEvent( EventContext* eventContext ) override;

  // Make multiple events available to the scheduler
  StatusCode pushNewEvents( std::vector<EventContext*>& eventContexts ) override;

  /// Blocks until an event is availble
  StatusCode popFinishedEvent( EventContext*& eventContext ) override;

  /// Try to fetch an event from the scheduler
  StatusCode tryPopFinishedEvent( EventContext*& eventContext ) override;

  /// Get free slots number
  unsigned int freeSlots() override;

private:
  Gaudi::Property<bool> m_useTopAlgList{this, "UseTopAlgList", true,
                                        "Decide if the top alglist or its flat version has to be used"};

  /// Cache the list of algs to be executed
  std::list<IAlgorithm*> m_algList;

  /// The context of the event being processed
  EventContext* m_eventContext = nullptr;

  /// The number of free slots (0 or 1)
  int m_freeSlots = 1;
};

#endif // GAUDIHIVE_SEQUENTIALSCHEDULERSVC_H
