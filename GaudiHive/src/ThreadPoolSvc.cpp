#include "ThreadPoolSvc.h"

#include "GaudiKernel/ConcurrencyFlags.h"
#include "ThreadInitTask.h"

#include "tbb/task.h"
#include "tbb/task_scheduler_observer.h"
#include "tbb/tbb_thread.h"
#include "tbb/tick_count.h"

#include <chrono>
#include <thread>

using namespace tbb;

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
  StatusCode sc( Service::initialize() );
  if ( !sc.isSuccess() ) {
    warning() << "Base class could not be initialized" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( m_threadInitTools.retrieve().isFailure() ) {
    error() << "Unable to retrieve ThreadInitTools Array" << endmsg;

    return StatusCode::FAILURE;
  }
  if ( m_threadInitTools.size() != 0 ) {
    info() << "retrieved " << m_threadInitTools.size() << " thread init tools" << endmsg;
  } else {
    info() << "no thread init tools attached" << endmsg;
  }

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------

StatusCode ThreadPoolSvc::finalize() {

  if ( !m_init ) {
    warning() << "Looks like the ThreadPoolSvc was created, but thread pool "
              << "was never initialized" << endmsg;
  }

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------

StatusCode ThreadPoolSvc::initPool( const int& poolSize ) {

  tbb::spin_mutex::scoped_lock lock( m_initMutex );

  m_threadPoolSize = poolSize;

  if ( msgLevel( MSG::DEBUG ) ) debug() << "ThreadPoolSvc::initPool() poolSize = " << poolSize << endmsg;
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
  // -1 means use all available cores

  if ( -100 != m_threadPoolSize ) {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "Initialising a thread pool of size " << m_threadPoolSize << endmsg;

    // Leave -1 in case selected, increment otherwise
    // - What?
    int thePoolSize = m_threadPoolSize;
    if ( thePoolSize >= 0 ) thePoolSize += 1;

    if ( m_threadPoolSize == -1 ) {
      // if requested pool size == -1, use number of available cores
      m_threadPoolSize = std::thread::hardware_concurrency();
      thePoolSize      = m_threadPoolSize;
    } else if ( m_threadPoolSize < -1 ) {
      fatal() << "Unexpected ThreadPoolSize \"" << m_threadPoolSize << "\". Allowed negative values are "
              << "-1 (use all available cores) and -100 (don't use a thread pool)" << endmsg;
      return StatusCode::FAILURE;
    }

#if TBB_INTERFACE_VERSION_MAJOR < 12
    m_tbbSchedInit = std::make_unique<tbb::task_scheduler_init>( thePoolSize );
#endif // TBB_INTERFACE_VERSION_MAJOR < 12

    m_tbbgc = std::make_unique<tbb::global_control>( global_control::max_allowed_parallelism, thePoolSize );

    Gaudi::Concurrency::ConcurrencyFlags::setNumThreads( thePoolSize );

    // Create the barrier for task synchronization at termination
    m_barrier = std::make_unique<boost::barrier>( thePoolSize );

  } else {
    // don't use a thread pool
    Gaudi::Concurrency::ConcurrencyFlags::setNumThreads( 1 );
    m_tbbgc = std::make_unique<tbb::global_control>( global_control::max_allowed_parallelism, 0 );
  }

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "Thread Pool initialization complete. Max task concurrency: "
            << tbb::global_control::active_value( global_control::max_allowed_parallelism ) << endmsg;

  m_init = true;

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------

StatusCode ThreadPoolSvc::terminatePool() {
  tbb::spin_mutex::scoped_lock lock( m_initMutex );
  if ( msgLevel( MSG::DEBUG ) ) debug() << "ThreadPoolSvc::terminatePool()" << endmsg;

  if ( !m_init ) {
    error() << "Trying to terminate uninitialized thread pool!" << endmsg;
    return StatusCode::FAILURE;
  }

  // Launch the termination tasks
  const bool terminate = true;
  if ( launchTasks( terminate ).isFailure() ) return StatusCode::FAILURE;

  if ( msgLevel( MSG::DEBUG ) ) debug() << "Thread pool termination complete!" << endmsg;

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------

StatusCode ThreadPoolSvc::launchTasks( bool terminate ) {

  const std::string taskType = terminate ? "termination" : "initialization";

  // If we have a thread pool (via a scheduler), then we want to queue
  // the tasks in TBB to execute on each thread.
  if ( tbb::global_control::active_value( global_control::max_allowed_parallelism ) > 0 ) {

    // Create one task for each worker thread in the pool
    for ( int i = 0; i < m_threadPoolSize; ++i ) {
      if ( msgLevel( MSG::DEBUG ) ) debug() << "creating ThreadInitTask " << i << endmsg;
      tbb::task* t = new ( tbb::task::allocate_root() )
          ThreadInitTask( m_threadInitTools, m_barrier.get(), serviceLocator(), terminate );

      // Queue the task
      tbb::task::enqueue( *t );
      std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
    }

    // Now wait for all the workers to reach the barrier
    if ( msgLevel( MSG::DEBUG ) ) debug() << "waiting at barrier for all ThreadInitTool to finish executing" << endmsg;
    m_barrier->wait();

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
          // it is likely the case that tbb activated new theads
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
    if ( msgLevel( MSG::DEBUG ) ) debug() << "launching ThreadInitTask " << taskType << "in this thread." << endmsg;
    boost::barrier* noBarrier = nullptr;
    ThreadInitTask  theTask( m_threadInitTools, noBarrier, serviceLocator(), terminate );
    theTask.execute();
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

// note TBB generates address sanitizer errors here, e.g.
//
// ==51081==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7fe7decf5195 at pc 0x7fe7e5da48bf bp
// 0x7fe7decf4f70 sp 0x7fe7decf4f68 WRITE of size 1 at 0x7fe7decf5195 thread T4
//    #0 0x7fe7e5da48be in tbb::task::task()
//    /cvmfs/lhcb.cern.ch/lib/lcg/releases/tbb/2019_U1-5939b/x86_64-centos7-gcc8-dbg/include/tbb/task.h:586
//
// Use GAUDI_NO_SANITIZE_ADDRESS to suppress these.
// To be looked at again when TBB is updated.

void GAUDI_NO_SANITIZE_ADDRESS ThreadPoolSvc::initThisThread() {

  if ( Gaudi::Concurrency::ThreadInitDone ) {
    // this should never happen
    error() << "initThisThread triggered, but thread already initialized" << endmsg;
    throw GaudiException( "initThisThread triggered, but thread already initialized", name(), StatusCode::FAILURE );
  }

  boost::barrier* noBarrier = nullptr;
  ThreadInitTask  theTask( m_threadInitTools, noBarrier, serviceLocator(), false );
  theTask.execute();
}
