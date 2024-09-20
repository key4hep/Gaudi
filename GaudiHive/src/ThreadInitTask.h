/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIHIVE_THREADINITTASK_H
#define GAUDIHIVE_THREADINITTASK_H

#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/IThreadInitTool.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/ToolHandle.h>

#include <boost/thread.hpp>
#include <string>
#include <vector>

/** @class ThreadInitTask
 *  @brief Special TBB task used by ThreadPoolSvc to wrap execution of
 *  IThreadInitTools.
 *
 *  @author Charles Leggett
 */

class ThreadInitTask {

public:
  ThreadInitTask( ToolHandleArray<IThreadInitTool>& tools, boost::barrier* b, ISvcLocator* svcLocator,
                  bool terminate = false )
      : m_tools( tools ), m_barrier( b ), m_serviceLocator( svcLocator ), m_terminate( terminate ){};

  /** @brief Execute the task
   *
   *  Will loop over IThreadInitTools and execute each one.
   */
  void operator()() const;

  static bool execFailed() { return m_execFailed; }

private:
  ToolHandleArray<IThreadInitTool> m_tools;

  boost::barrier*      m_barrier = nullptr;
  SmartIF<ISvcLocator> m_serviceLocator;
  bool                 m_terminate = false;

  static std::atomic<bool> m_execFailed;
};

#endif
