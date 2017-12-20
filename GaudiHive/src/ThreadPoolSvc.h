#ifndef GAUDIHIVE_THREADPOOLSVC_H
#define GAUDIHIVE_THREADPOOLSVC_H

#include "GaudiKernel/IThreadInitTool.h"
#include "GaudiKernel/IThreadPoolSvc.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/ToolHandle.h"

#include "boost/thread.hpp"
#include "tbb/spin_mutex.h"
#include "tbb/task_scheduler_init.h"

#include <memory>
#include <vector>

/** @class ThreadPoolSvc
  * @brief A service which initializes a TBB thread pool.
  *
  * This service can be configured with an array of IThreadInitTools which
  * will each be invoked concurrently on each worker thread. A ThreadInitTask
  * is created for each thread and given the list of tools. A boost::barrier
  * is used to synchronize the calling of each tool concurrently on all
  * threads at the same time.
  *
  */
class ThreadPoolSvc : public extends<Service, IThreadPoolSvc>
{
public:
  /// Constructor
  ThreadPoolSvc( const std::string& name, ISvcLocator* svc );

  /// Initialise
  StatusCode initialize() override final;

  /// Finalise
  StatusCode finalize() override final;

  /// Initialize the thread pool and launch the ThreadInitTasks.
  StatusCode initPool( const int& poolSize ) override final;

  /// Terminate the thread pool and launch thread termination tasks.
  StatusCode terminatePool() override final;

  int poolSize() const override final { return m_threadPoolSize; }

  virtual bool isInit() const { return m_init; }

private:
  /// Launch tasks to execute the ThreadInitTools
  StatusCode launchTasks( bool finalize = false );

  /// Handle array of thread init tools
  ToolHandleArray<IThreadInitTool> m_threadInitTools = {this};

  /// Was the thread pool initialized?
  bool m_init = false;

  /// Size of the thread pool allocated
  int m_threadPoolSize = 0;

  /// Mutex used to protect the initPool and terminatePool methods.
  tbb::spin_mutex m_initMutex;

  /// TBB task scheduler initializer
  std::unique_ptr<tbb::task_scheduler_init> m_tbbSchedInit;

  /// Barrier used to synchronization thread init tasks
  std::unique_ptr<boost::barrier> m_barrier;
};

#endif
