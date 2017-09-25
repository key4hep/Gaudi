#ifndef GAUDIHIVE_THREADINITTASK_H
#define GAUDIHIVE_THREADINITTASK_H

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IThreadInitTool.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ToolHandle.h"

#include "boost/thread.hpp"
#include "tbb/task.h"
#include <string>
#include <vector>

/** @class ThreadInitTask
 *  @brief Special TBB task used by ThreadPoolSvc to wrap execution of
 *  IThreadInitTools.
 *
 *  @author Charles Leggett
 */

class ThreadInitTask : virtual public tbb::task

{

public:
  ThreadInitTask( ToolHandleArray<IThreadInitTool>& tools, boost::barrier* b, ISvcLocator* svcLocator,
                  bool terminate = false )
      : m_tools( tools ), m_barrier( b ), m_serviceLocator( svcLocator ), m_terminate( terminate ){};

  /** @brief Execute the task
   *
   *  Will loop over IThreadInitTools and execute each one.
   */
  tbb::task* execute() override;

  static bool execFailed() { return m_execFailed; }

private:
  ToolHandleArray<IThreadInitTool>& m_tools;

  boost::barrier* m_barrier;
  SmartIF<ISvcLocator> m_serviceLocator;
  bool m_terminate;

  static std::atomic<bool> m_execFailed;
};

#endif
