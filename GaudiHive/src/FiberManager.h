
/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include "boost/fiber/algo/shared_work.hpp"
#include "boost/fiber/condition_variable.hpp"
#include "boost/fiber/fiber.hpp"
#include "boost/fiber/mutex.hpp"

#include "fmt/format.h"

/** @class FiberManager FiberManager.h
 *
 * The FiberManager manages a pool of threads used to run boost::fiber fibers.
 *
 * These fibers can be suspended while waiting for GPU operations (with CUDA and HIP support
 * built-in), remote I/O operations, and the like, providing a base for asynchronous algorithm
 * support.
 *
 * @author Beojan Stanislaus
 * @version 1.0
 * @date November 2023
 * */

class FiberManager {
public:
  /** FiberManager constructor
   *
   * @param n_threads Number of threads for CPU portion of asynchronous algorithms.
   *                  These are *in addition* to the TBB worker threads used for CPU algorithms.
   *    * */
  FiberManager( int n_threads ) {
    for ( int i = 0; i < n_threads; ++i ) {
      m_threads.emplace_back( std::thread( [this]() {
        boost::fibers::use_scheduling_algorithm<boost::fibers::algo::shared_work>();
        std::unique_lock lck{ m_shuttingDown_mtx };
        m_shuttingDown_cv.wait( lck );
      } ) );
    }
  }

  ~FiberManager() {
    m_shuttingDown_cv.notify_all();

    for ( std::thread& t : m_threads ) { t.join(); }
  }

  /** Schedule work to run on the asynchronous pool.
   *
   * AsynchronousAlgorithms should use a relatively small amount of CPU time,
   * and should use boost::fiber functionality to yield while waiting for offloaded
   * work to complete.
   *
   * @param func The AlgTask, when used in AvalancheSchedulerSvc
   * */
  template <typename F>
  void schedule( F&& func ) {
    if ( !m_activated ) {
      // Since we never call boost::this_fiber::yield, fibers never actually run on this thread
      boost::fibers::use_scheduling_algorithm<boost::fibers::algo::shared_work>();
      m_activated = true;
    }
    boost::fibers::fiber( boost::fibers::launch::post, std::forward<F>( func ) ).detach();
  }

private:
  boost::fibers::condition_variable m_shuttingDown_cv{};
  boost::fibers::mutex              m_shuttingDown_mtx{};
  std::vector<std::thread>          m_threads;
  bool                              m_activated = false; // set to true when first fiber is scheduled
};
