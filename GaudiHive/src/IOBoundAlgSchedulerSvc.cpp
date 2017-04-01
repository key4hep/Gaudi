#include <algorithm>
#include <map>
#include <queue>
#include <sstream>
#include <thread>
#include <unordered_set>

// Framework includes
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Algorithm.h" // will be IAlgorithm if context getter promoted to interface
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IDataManagerSvc.h"

// Local
#include "IOBoundAlgSchedulerSvc.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_COMPONENT( IOBoundAlgSchedulerSvc )

/**
 * Here, among some "bureaucracy" operations, the scheduler is activated,
 * executing the activate() function in a new thread.
**/
StatusCode IOBoundAlgSchedulerSvc::initialize()
{

  // Initialise mother class (read properties, ...)
  StatusCode sc( Service::initialize() );
  if ( !sc.isSuccess() ) warning() << "Base class could not be initialized" << endmsg;

  // Activate the scheduler in another thread.
  info() << "Activating scheduler in a separate thread" << endmsg;
  m_thread = std::thread( std::bind( &IOBoundAlgSchedulerSvc::activate, this ) );

  return sc;
}
//---------------------------------------------------------------------------

/**
 * Here the scheduler is deactivated and the thread joined.
**/
StatusCode IOBoundAlgSchedulerSvc::finalize()
{

  StatusCode sc( Service::finalize() );
  if ( !sc.isSuccess() ) warning() << "Base class could not be finalized" << endmsg;

  sc = deactivate();
  if ( !sc.isSuccess() ) warning() << "Scheduler could not be deactivated" << endmsg;

  info() << "Joining preemptive scheduler's thread" << endmsg;
  m_thread.join();

  return sc;
}
//---------------------------------------------------------------------------
/**
 * Activate the scheduler. From this moment on the queue of actions is
 * checked. The checking will stop when the m_isActive flag is false and the
 * queue is not empty. This will guarantee that all actions are executed and
 * a stall is not created.
 **/
void IOBoundAlgSchedulerSvc::activate()
{

  // Now it's running
  m_isActive = true;

  // Wait for actions pushed into the queue by finishing tasks.
  action thisAction;
  StatusCode sc( StatusCode::SUCCESS );

  // Continue to wait if the scheduler is running or there is something to do
  info() << "Start checking the queue of I/O-bound algorithm tasks.." << endmsg;
  while ( m_isActive or m_actionsQueue.size() > 0 ) {
    m_actionsQueue.pop( thisAction );
    std::thread th( thisAction );
    th.detach();
  }
}

//---------------------------------------------------------------------------

/**
 * Deactivates the scheduler. Two actions are pushed into the queue:
 *  1) Drain the scheduler until all events are finished.
 *  2) Flip the status flag m_isActive to false
 * This second action is the last one to be executed by the scheduler.
 */
StatusCode IOBoundAlgSchedulerSvc::deactivate()
{

  if ( m_isActive ) {
    // Drain the scheduler
    // m_actionsQueue.push(std::bind(&IOBoundSchedulerSvc::m_drain,
    //                              this));
    // we set the flag in this thread, not in the last action, to avoid stall,
    // since we execute tasks asynchronously, in a detached thread, and it's possible that
    // the task doesn't complete by the time the last while-iteration is entered
    m_isActive = false;
    // This would be the last (empty) action, just to trigger one last while-iteration
    m_actionsQueue.push( []() -> StatusCode { return StatusCode::SUCCESS; } );
  }

  return StatusCode::SUCCESS;
}

StatusCode IOBoundAlgSchedulerSvc::push( IAlgTask& task )
{

  // the temporary lambda should be moved into the queue in here
  auto actionn = [&]() {
    debug() << " .. launching I/O-bound algo-closure .. " << endmsg;
    return task.execute();
  };

  m_actionsQueue.push( actionn );

  // until the queue to accelerator becomes limited
  return StatusCode::SUCCESS;
}

//===========================================================================
