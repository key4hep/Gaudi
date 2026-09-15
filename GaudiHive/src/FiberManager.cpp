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

#include "FiberManager.h"
#include <boost/version.hpp>
#if ( BOOST_VERSION >= 108400 )
#  include <boost/fiber/stack_allocator_wrapper.hpp>
#endif

FiberManager::FiberManager( int n_threads ) {
  // Start a fiber scheduler with a shared work queue on the current thread.
  // The fiber scheduler is configured not to sleep in case there are no ready fibers, but this situation should never
  // happen as the main fiber of this thread should never be suspended so it's always active and scheduler will never
  // try to pick-up another fiber.
  boost::fibers::use_scheduling_algorithm<SchedAlgo>();

  // Create a pool of threads running fiber schedulers with shared work queue.
  // The scheduler in each thread is configured to sleep while no fibers are ready and needs to be manual
  // notified if it's asleep when a new work item is added.
  const auto sleep_while_idle = true;
  m_schedAlgoList.resize( n_threads );
  m_threads.reserve( n_threads );
  for ( int i = 0; i < n_threads; ++i ) {
    m_threads.emplace_back( [this, i, sleep_while_idle]() {
      // Save handle to the scheduler of this thread so it can be manually notified.
      auto* algo_ptr          = new SchedAlgo( sleep_while_idle );
      m_schedAlgoList.at( i ) = algo_ptr;
#if ( BOOST_VERSION >= 108400 )
      boost::fibers::initialize_thread( algo_ptr,
                                        boost::fibers::make_stack_allocator_wrapper<boost::fibers::default_stack>() );
#else
      boost::fibers::context::active()->get_scheduler()->set_algo( algo_ptr );
#endif
      // Suspend the main fiber of this worker so it doesn't exit prematurely.
      std::unique_lock lck{ m_shuttingDown_mtx };
      m_shuttingDown_cv.wait( lck );
    } );
  }
}

FiberManager::~FiberManager() {
  // Notify the main fibers of the worker threads to exit.
  m_shuttingDown_cv.notify_all();
}
