#include "ThreadPoolSvc.h"

#include "GaudiKernel/SvcFactory.h"
#include "ThreadInitTask.h"

#include "tbb/task_scheduler_init.h"
#include "tbb/task_scheduler_observer.h"
#include "tbb/task.h"
#include "tbb/tick_count.h"
#include "tbb/tbb_thread.h"

using namespace tbb;

DECLARE_SERVICE_FACTORY(ThreadPoolSvc)

//=============================================================================


ThreadPoolSvc::ThreadPoolSvc( const std::string& name, ISvcLocator* svcLoc ):
  base_class(name,svcLoc),
  m_threadInitTools(this),
  m_init(false),
  m_threadPoolSize(0),
  m_tbbSchedInit(nullptr),
  m_barrier(nullptr)
{

  declareProperty("ThreadInitTools", m_threadInitTools,
                  "ToolHandleArray of IThreadInitTools");

}

//-----------------------------------------------------------------------------

StatusCode
ThreadPoolSvc::initialize() {

  // Initialise mother class (read properties, ...)
  StatusCode sc(Service::initialize());
  if (!sc.isSuccess()) {
    warning () << "Base class could not be initialized" << endmsg;
    return StatusCode::FAILURE;
  }

  if (m_threadInitTools.retrieve().isFailure()) {
    error() << "Unable to retrieve ThreadInitTools Array" << endmsg;

    return StatusCode::FAILURE;
  } else {
    if (m_threadInitTools.size() != 0) {
      info() << "retrieved " << m_threadInitTools.size() << " thread init tools"
             << endmsg;
    } else {
      info() << "no thread init tools attached" << endmsg;
    }
  }

  return StatusCode::SUCCESS;

}

//-----------------------------------------------------------------------------

StatusCode
ThreadPoolSvc::finalize() {

  if (!m_init) {
    warning() << "Looks like the ThreadPoolSvc was created, but thread pool "
              << "was never initialized" << endmsg;
  }

  return StatusCode::SUCCESS;

}

//-----------------------------------------------------------------------------

StatusCode
ThreadPoolSvc::initPool(const int& poolSize) {

  tbb::spin_mutex::scoped_lock lock( m_initMutex );

  m_threadPoolSize = poolSize;

  if (msgLevel(MSG::DEBUG))
    debug() << "ThreadPoolSvc::initPool() poolSize = " << poolSize << endmsg;
  //There is a problem in the piece of the code below.  if
  // m_threadPoolSize is set to something negative which is < -1,
  // algorithm below might not behave as expected. For the time being
  // I've choosen to create the barrier with the default number of
  // threads created by the task scheduler init assuming that a
  // negative value will choose automatic thread creation which will
  // create default number of threads.
  // SK

  // -100 prevents the creation of the pool and the scheduler directly
  // executes the tasks.
  if (-100 != m_threadPoolSize) {
    if (msgLevel(MSG::DEBUG))
      debug() << "Initialising a thread pool of size "
              << m_threadPoolSize << endmsg;

    // Leave -1 in case selected, increment otherwise
    // - What?
    int thePoolSize = m_threadPoolSize;
    if (thePoolSize != -1) thePoolSize += 1;


    // Create the TBB task scheduler
    m_tbbSchedInit = std::unique_ptr<tbb::task_scheduler_init>( new tbb::task_scheduler_init(thePoolSize) );
    // Create the barrier for task synchronization
    if(m_threadPoolSize<=-1)thePoolSize=m_tbbSchedInit->default_num_threads();
    if (msgLevel(MSG::DEBUG)){
      debug() << "creating barrier of size " << thePoolSize << endmsg;
    }
    m_barrier = std::unique_ptr<boost::barrier>( new boost::barrier(thePoolSize) );

  }

  // Launch the init tool tasks
  const bool terminate = false;
  if (launchTasks(terminate).isFailure())
    return StatusCode::FAILURE;

  if (msgLevel(MSG::DEBUG))
    debug() << "Thread Pool initialization complete!" << endmsg;

  m_init = true;

  return StatusCode::SUCCESS;

}

//-----------------------------------------------------------------------------

StatusCode
ThreadPoolSvc::terminatePool() {
  tbb::spin_mutex::scoped_lock lock( m_initMutex );
  if (msgLevel(MSG::DEBUG))
    debug() << "ThreadPoolSvc::terminatePool()" << endmsg;

  if (!m_init) {
    error() << "Trying to terminate uninitialized thread pool!" << endmsg;
    return StatusCode::FAILURE;
  }

  // Launch the termination tasks
  const bool terminate = true;
  if (launchTasks(terminate).isFailure())
    return StatusCode::FAILURE;

  if (msgLevel(MSG::DEBUG))
    debug() << "Thread pool termination complete!" << endmsg;

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------

std::vector<IThreadInitTool*>
ThreadPoolSvc::getThreadInitTools() const {

  std::vector<IThreadInitTool*> tools;

  ToolHandleArray<IThreadInitTool>::const_iterator ito = m_threadInitTools.begin();
  for ( ; ito != m_threadInitTools.end(); ++ito ) {
    IThreadInitTool* it = &(**ito);
    tools.push_back(it);
  }

  return tools;
}

//-----------------------------------------------------------------------------

StatusCode
ThreadPoolSvc::launchTasks(bool terminate) {

  if (m_threadInitTools.empty()) return StatusCode::SUCCESS;

  const std::string taskType = terminate? "termination" : "initialization";

  // If we have a thread pool (via a scheduler), then we want to queue
  // the tasks in TBB to execute on each thread.
  if(m_tbbSchedInit) {

    // Create one task for each worker thread in the pool
    for (int i = 0; i < m_threadPoolSize; ++i) {
      if (msgLevel(MSG::DEBUG))
        debug() << "creating ThreadInitTask " << i << endmsg;
      tbb::task* t = new(tbb::task::allocate_root())
        ThreadInitTask( m_threadInitTools, m_barrier.get(), serviceLocator(), terminate );

      // Queue the task
      tbb::task::enqueue( *t );
      this_tbb_thread::sleep(tbb::tick_count::interval_t(.1));
    }

    // Now wait for all the workers to reach the barrier
    if (msgLevel(MSG::DEBUG))
      debug() << "waiting at barrier for all ThreadInitTool to finish executing" << endmsg;
    m_barrier->wait();

    // Check to make sure all Tools were invoked.
    // I'm not sure this mechanism is worthwhile.
    for (auto& t : m_threadInitTools) {
      // Number of threads initialized but not terminated.
      int numInit = t->nInit();
      // Expected number based on the type of task.
      int expectedNumInit = terminate? 0 : m_threadPoolSize;
      if (numInit != expectedNumInit) {
        error() << "not all threads " << (terminate? "terminated" : "initialized")
                << " for tool " << t << " : "
                << t->nInit() << " out of " << m_threadPoolSize
                << " are currently active" << endmsg;
        return StatusCode::FAILURE;
      }
    }

  }

  // In single-threaded mode, there is no scheduler, so we simply call
  // the task wrapper directly in this thread.
  else {
    if (msgLevel(MSG::DEBUG))
      debug() << "launching ThreadInitTask " << taskType << "in this thread." << endmsg;
    boost::barrier* noBarrier = nullptr;
    ThreadInitTask theTask(m_threadInitTools, noBarrier, serviceLocator(), terminate);
    theTask.execute();
  }

  // Now, we do some error checking
  if (ThreadInitTask::execFailed()) {
    error() << "a ThreadInitTask failed to execute successfully" << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}
