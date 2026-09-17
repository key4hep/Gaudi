
/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
#include <boost/fiber/algo/shared_work.hpp>
#include <boost/fiber/condition_variable.hpp>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/mutex.hpp>

#include <thread>
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
   *
   * @note The calling thread is initialized with fiber scheduler and starts participating in sharing the global work
   * queue of fibers, the current context becomes the main fiber of the calling thread. The main fiber should not be
   * suspended, preemptive blocking is allowed. After construction, the calling thread must not be assigned different
   * fiber scheduling algorithm.
   *
   *    * */
  FiberManager( int n_threads );
  ~FiberManager();

  /** Schedule work to run on the asynchronous pool.
   *
   * AsynchronousAlgorithms should use a relatively small amount of CPU time,
   * and should use boost::fiber functionality to yield while waiting for offloaded
   * work to complete.
   *
   * @param func The AlgTask, when used in AvalancheSchedulerSvc
   *
   * @note This function should only be called from a thread participating in the global work sharing of fibers, e.g.
   * the thread that constructed the FiberManager.
   *
   * */
  template <typename F>
  void schedule( F&& func ) {
    // Add a new fiber invoking the function to the shared work queue, then notify the workers in case they where
    // sleeping.
    boost::fibers::fiber( boost::fibers::launch::post, std::forward<F>( func ) ).detach();
    for ( auto* p : m_schedAlgoList ) { p->notify(); }
  }

private:
  using SchedAlgo = boost::fibers::algo::shared_work;
  boost::fibers::condition_variable m_shuttingDown_cv{};
  boost::fibers::mutex              m_shuttingDown_mtx{};
  std::vector<std::jthread>         m_threads{};
  std::vector<SchedAlgo*>           m_schedAlgoList{};
};
