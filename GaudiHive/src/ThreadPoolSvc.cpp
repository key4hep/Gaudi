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

//===========================================================================


ThreadPoolSvc::ThreadPoolSvc( const std::string& name, ISvcLocator* svcLoc ):
  base_class(name,svcLoc),
  m_init(false),
  m_threadPoolSize(0),
  m_threadInitTools(this),
  m_tbbSchedInit(0),
  m_barrier(0)
{

  declareProperty("ThreadInitTools", m_threadInitTools, "ToolHandleArray of IThreadInitTools");

}

//---------------------------------------------------------------------------

ThreadPoolSvc::~ThreadPoolSvc() {

  if (m_tbbSchedInit)
    delete m_tbbSchedInit;

  if (m_barrier)
    delete m_barrier;

}

//---------------------------------------------------------------------------

StatusCode
ThreadPoolSvc::initialize() {

  // Initialise mother class (read properties, ...)
  StatusCode sc(Service::initialize());
  if (!sc.isSuccess()) {
    warning () << "Base class could not be initialized" << endmsg;
    return StatusCode::FAILURE;
  }

  if (m_threadInitTools.retrieve().isFailure()) {
    error() << "Unable to retrieve ThreadInitTools Array"
  	    << endmsg;

    return StatusCode::FAILURE;
  } else {
    if (m_threadInitTools.size() != 0) {
      info() << "retrieved " << m_threadInitTools.size() << " thread init tools"
   	     << endmsg;
    } else {
      info() << "no thread init tools attached"
   	     << endmsg;
    }
  }

  return StatusCode::SUCCESS;

}

//---------------------------------------------------------------------------

StatusCode
ThreadPoolSvc::finalize() {

  if (!m_init) {
    warning() << "Looks like the ThreadPoolSvc was created, but thread pool was never initialized"
    << endmsg;
  }

  return StatusCode::SUCCESS;

}

//---------------------------------------------------------------------------

StatusCode
ThreadPoolSvc::initPool(const int& poolSize) {

  tbb::spin_mutex::scoped_lock  lock( m_initMutex );

  m_threadPoolSize = poolSize;

  if (msgLevel(MSG::DEBUG))
    debug() << "ThreadPoolSvc::initPool() poolSize = " << poolSize << endmsg;

  // -100 prevents the creation of the pool and the scheduler directly executes
  // the tasks.
  if (-100 != m_threadPoolSize){
    if (msgLevel(MSG::DEBUG))
      debug() << "Initialising a ThreadPool thread pool of size " << m_threadPoolSize
              << endmsg;
    // Leave -1 in case selected, increment otherwise
    int thePoolSize=m_threadPoolSize;
    if (thePoolSize!=-1)
      thePoolSize+=1;
    m_tbbSchedInit = new tbb::task_scheduler_init (thePoolSize);

    // execute the Thread Init Tools if there are any, and wait at barrier
    if (m_threadInitTools.size() > 0) {
      if (msgLevel(MSG::DEBUG))
        debug() << "creating barrier of size " << m_threadPoolSize + 1 << endmsg;
      m_barrier = new boost::barrier( m_threadPoolSize + 1 );

      // create ThreadInitTasks, wait until all have executed
      for (int i=0; i<m_threadPoolSize; ++i) {
        if (msgLevel(MSG::DEBUG))
          debug() << "creating ThreadInitTask " << i << endmsg;
    	tbb::task* t = new(tbb::task::allocate_root())
    	  ThreadInitTask( m_threadInitTools, m_barrier, serviceLocator() );

    	tbb::task::enqueue( *t );
    	this_tbb_thread::sleep(tbb::tick_count::interval_t(.1));
      }
    }

  } else {
    if (msgLevel(MSG::DEBUG))
      debug() << "Thread pool size is one. Pool not initialised." << endmsg;

    if (m_threadInitTools.size() > 0) {
      if (msgLevel(MSG::DEBUG))
        debug() << "doing ThreadInitTask init in this thread" << endmsg;
      ThreadInitTask theTask(m_threadInitTools, m_barrier, serviceLocator() );
      theTask.execute();
    }

  }

  if (m_barrier) {
    if (msgLevel(MSG::DEBUG))
      debug() << "waiting at barrier for all ThreadInitTool to finish executing" << endmsg;
    m_barrier->wait();
  }

  if (ThreadInitTask::execFailed()) {
    error() << "a ThreadInitTask failed to execute successfully" << endmsg;
    return StatusCode::FAILURE;
  }

  // check to make sure all Tools were initialized
  for (auto t : m_threadInitTools) {
    if (t->nInit() != (unsigned int)m_threadPoolSize && m_threadPoolSize != -100) {
      error() << " not all threads initialized for tool " << t << " : "
  	      << t->nInit() << " out of " << m_threadPoolSize
  	      << endmsg;
      return StatusCode::FAILURE;
    }
  }

  if (msgLevel(MSG::DEBUG))
    debug() << " Thread Pool initialization complete!" << endmsg;

  m_init = true;

  return StatusCode::SUCCESS;

}

//---------------------------------------------------------------------------

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
