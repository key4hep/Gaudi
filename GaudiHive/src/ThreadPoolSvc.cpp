/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "ThreadPoolSvc.h"

#include "GaudiKernel/ConcurrencyFlags.h"
#include "ThreadInitTask.h"

#include "tbb/task_group.h"

#include <chrono>
#include <thread>

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

namespace Gaudi {
  namespace Concurrency {
    extern thread_local bool ThreadInitDone;
  }
} // namespace Gaudi

DECLARE_COMPONENT( ThreadPoolSvc )

//=============================================================================

ThreadPoolSvc::ThreadPoolSvc( const std::string& name, ISvcLocator* svcLoc ) : extends( name, svcLoc ) {
  declareProperty( "ThreadInitTools", m_threadInitTools, "ToolHandleArray of IThreadInitTools" );
}

//-----------------------------------------------------------------------------

StatusCode ThreadPoolSvc::initialize() {

  // Initialise mother class (read properties, ...)
  if ( Service::initialize().isFailure() ) {
    warning() << "Base class could not be initialized" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( m_threadInitTools.retrieve().isFailure() ) {
    error() << "Unable to retrieve ThreadInitTools Array" << endmsg;

    return StatusCode::FAILURE;
  }

  if ( m_threadInitTools.size() != 0 )
    info() << "retrieved " << m_threadInitTools.size() << " thread init tools" << endmsg;
  else
    info() << "no thread init tools attached" << endmsg;

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------

StatusCode ThreadPoolSvc::finalize() {

  if ( !m_init )
    warning() << "Looks like the ThreadPoolSvc was created, but thread pool "
              << "was never initialized" << endmsg;

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------

StatusCode ThreadPoolSvc::initPool( const int& poolSize ) {

  tbb::spin_mutex::scoped_lock lock( m_initMutex );

  m_threadPoolSize = poolSize;

  ON_DEBUG debug() << "ThreadPoolSvc::initPool() poolSize = " << poolSize << endmsg;
  // There is a problem in the piece of the code below.  if
  // m_threadPoolSize is set to something negative which is < -1,
  // algorithm below might not behave as expected. For the time being
  // I've choosen to create the barrier with the default number of
  // threads created by the task scheduler init assuming that a
  // negative value will choose automatic thread creation which will
  // create default number of threads.
  // SK

  // -100 prevents the creation of the pool and the scheduler directly
  // executes the tasks.
  // -1 means use all available hardware threads

  if ( -100 != m_threadPoolSize ) {
    ON_DEBUG debug() << "Initialising a thread pool of size " << m_threadPoolSize << endmsg;

    if ( m_threadPoolSize == -1 ) {
      m_threadPoolSize = std::thread::hardware_concurrency();
    } else if ( m_threadPoolSize < -1 ) {
      fatal() << "Unexpected ThreadPoolSize \"" << m_threadPoolSize << "\". Allowed negative values are "
              << "-1 (use all available cores) and -100 (don't use a thread pool)" << endmsg;
      return StatusCode::FAILURE;
    }

#if TBB_INTERFACE_VERSION_MAJOR < 12
    m_tbbSchedInit = std::make_unique<tbb::task_scheduler_init>( m_threadPoolSize + 1 );
#endif // TBB_INTERFACE_VERSION_MAJOR < 12

    ON_VERBOSE verbose() << "Maximum allowed parallelism before adjusting: "
                         << tbb::global_control::active_value( tbb::global_control::max_allowed_parallelism ) << endmsg;

    // to get the number of threads we need, request one thread more to account for how TBB calculates
    // its soft limit of the number of threads for the global thread pool
    m_tbbgc =
        std::make_unique<tbb::global_control>( tbb::global_control::max_allowed_parallelism, m_threadPoolSize + 1 );

    Gaudi::Concurrency::ConcurrencyFlags::setNumThreads( m_threadPoolSize );

    // Create the barrier for task synchronization at termination
    // (here we increase the number of threads by one to account for calling thread)
    m_barrier = std::make_unique<boost::barrier>( m_threadPoolSize + 1 );

  } else {
    // don't use a thread pool
    Gaudi::Concurrency::ConcurrencyFlags::setNumThreads( 1 );
    m_tbbgc = std::make_unique<tbb::global_control>( tbb::global_control::max_allowed_parallelism, 0 );
  }

  ON_DEBUG debug() << "Thread Pool initialization complete. Maximum allowed parallelism: "
                   << tbb::global_control::active_value( tbb::global_control::max_allowed_parallelism ) << endmsg;

  m_init = true;

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------

StatusCode ThreadPoolSvc::terminatePool() {
  tbb::spin_mutex::scoped_lock lock( m_initMutex );

  ON_DEBUG debug() << "ThreadPoolSvc::terminatePool()" << endmsg;

  if ( !m_init ) {
    error() << "Trying to terminate uninitialized thread pool!" << endmsg;
    return StatusCode::FAILURE;
  }

  // Launch the termination tasks
  const bool terminate = true;
  if ( launchTasks( terminate ).isFailure() ) return StatusCode::FAILURE;

  ON_DEBUG debug() << "Thread pool termination complete!" << endmsg;

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------

StatusCode ThreadPoolSvc::launchTasks( bool terminate ) {

  const std::string taskType = terminate ? "termination" : "initialization";

  // If we have a thread pool (via a scheduler), then we want to queue
  // the tasks in TBB to execute on each thread.
  if ( tbb::global_control::active_value( tbb::global_control::max_allowed_parallelism ) > 0 ) {

    tbb::task_group taskGroup;

    // Create one task for each worker thread in the pool
    for ( int i = 0; i < m_threadPoolSize; ++i ) {

      ON_DEBUG debug() << "creating ThreadInitTask " << i << endmsg;

      // Queue the task
      taskGroup.run( ThreadInitTask( m_threadInitTools, m_barrier.get(), serviceLocator(), terminate ) );
      std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }

    // Now wait for all the workers to reach the barrier
    ON_DEBUG debug() << "waiting at barrier for all ThreadInitTool to finish executing" << endmsg;
    m_barrier->wait();

    // task gropu's wait() must come after the barrier's wait()
    taskGroup.wait();

    // Check to make sure all Tools were invoked.
    // I'm not sure this mechanism is worthwhile.
    for ( auto& t : m_threadInitTools ) {
      // Number of threads initialized but not terminated.
      int numInit = t->nInit();
      // Expected number based on the type of task.
      int expectedNumInit = terminate ? 0 : m_threadPoolSize;
      if ( numInit != expectedNumInit ) {
        std::ostringstream ost;
        ost << "not all threads " << ( terminate ? "terminated" : "initialized" ) << " for tool " << t << " : "
            << t->nInit() << " out of " << m_threadPoolSize << " are currently active.";
        if ( terminate ) {
          // it is likely the case that TBB activated new threads
          // late in the game, and extra initializations were done
          info() << ost.str() << endmsg;
        } else {
          error() << ost.str() << endmsg;
          return StatusCode::FAILURE;
        }
      }
    }

  }

  // In single-threaded mode, there is no scheduler, so we simply call
  // the task wrapper directly in this thread.
  else {
    ON_DEBUG debug() << "launching ThreadInitTask " << taskType << "in this thread." << endmsg;

    boost::barrier* noBarrier = nullptr;
    ThreadInitTask( m_threadInitTools, noBarrier, serviceLocator(), terminate )();
  }

  // Now, we do some error checking
  if ( ThreadInitTask::execFailed() ) {
    error() << "a ThreadInitTask failed to execute successfully" << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------

// tbb will actually create more threads than requested, and will sometimes
// activate them late. This method is used to initialize one of these threads
// when it is detected

void ThreadPoolSvc::initThisThread() {

  if ( Gaudi::Concurrency::ThreadInitDone ) {
    // this should never happen
    error() << "initThisThread triggered, but thread already initialized" << endmsg;
    throw GaudiException( "initThisThread triggered, but thread already initialized", name(), StatusCode::FAILURE );
  }

  boost::barrier* noBarrier = nullptr;
  ThreadInitTask( m_threadInitTools, noBarrier, serviceLocator(), false )();
}
