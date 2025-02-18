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

#include "FiberManager.h"

FiberManager::FiberManager( int n_threads ) {
  m_schedAlgoList.resize( n_threads + 1 );
  auto* main_algo_ptr             = new SchedAlgo( true );
  m_schedAlgoList.at( n_threads ) = main_algo_ptr;
#if ( BOOST_VERSION >= 108400 )
  boost::fibers::initialize_thread( main_algo_ptr,
                                    boost::fibers::make_stack_allocator_wrapper<boost::fibers::default_stack>() );
#else
  boost::fibers::context::active()->get_scheduler()->set_algo( main_algo_ptr );
#endif

  for ( int i = 0; i < n_threads; ++i ) {
    m_threads.emplace_back( std::thread( [this, i]() {
      auto* algo_ptr          = new SchedAlgo( true );
      m_schedAlgoList.at( i ) = algo_ptr;
#if ( BOOST_VERSION >= 108400 )
      boost::fibers::initialize_thread( algo_ptr,
                                        boost::fibers::make_stack_allocator_wrapper<boost::fibers::default_stack>() );
#else
      boost::fibers::context::active()->get_scheduler()->set_algo( algo_ptr );
#endif
      std::unique_lock lck{ m_shuttingDown_mtx };
      m_shuttingDown_cv.wait( lck );
    } ) );
  }
}

FiberManager::~FiberManager() {
  m_shuttingDown_cv.notify_all();
  for ( std::thread& t : m_threads ) { t.join(); }
}
