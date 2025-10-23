/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
/*
 * test_LockedHandle.cpp
 *
 *  Created on: 2016-08-15
 *      Author: Charles Leggett (copied from test_GaudiTiming.cpp)
 */

#include <GaudiKernel/LockedHandle.h>
#include <boost/thread/mutex.hpp>
#include <chrono>
#include <mutex>
#include <thread>

#if __has_include( <catch2/catch.hpp>)
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_test_macros.hpp>
#endif

template <class M = std::mutex>
class Hist {
public:
  Hist( const std::string& id ) : m_id( id ) {}
  const std::string& id() const { return m_id; }
  void               acc() {
    // std::cout << "in Hist::access\n";
    m_i++;
  }
  void incr() { m_i++; }
  int  val() const { return m_i; }

  M& mut() { return m_mut; }

private:
  int         m_i{ 0 };
  std::string m_id;
  M           m_mut;
};

template <class T>
void Task( T* lh, [[maybe_unused]] int i, size_t nIter ) {

  // std::cout << "spawn Task " << i << "\n";
  std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

  for ( size_t j = 0; j < nIter; ++j ) { ( *lh )->incr(); }
  std::this_thread::sleep_for( std::chrono::milliseconds( 40 ) );
  // std::cout << "done Task " << i << "\n";
}

TEST_CASE( "LockedHandle test", "[LockedHandle]" ) {

  // test default template args
  auto                 h1 = std::make_unique<Hist<>>( "first" );
  LockedHandle<Hist<>> lh1( h1.get(), h1->mut() );
  lh1->acc();

  // test explicit mutex type
  typedef std::mutex               mut_t;
  auto                             h2 = std::make_unique<Hist<mut_t>>( "first" );
  LockedHandle<Hist<mut_t>, mut_t> lh2( h2.get(), h2->mut() );
  lh2->acc();

  // // test a different mutex type
  typedef boost::mutex                 mut_b_t;
  auto                                 h3 = std::make_unique<Hist<mut_b_t>>( "first" );
  LockedHandle<Hist<mut_b_t>, mut_b_t> lh3( h3.get(), h3->mut() );
  lh3->acc();

  // do a lot to see if we can get a race
  std::vector<std::thread> threads;
  size_t                   nthreads{ 10 };
  size_t                   nIter{ 10000 };
  for ( size_t i = 0; i < nthreads; ++i ) {
    threads.push_back( std::thread{ Task<LockedHandle<Hist<>>>, &lh1, i, nIter } );
  }

  for ( auto& t : threads ) { t.join(); }

  CHECK( h1->val() == int( 1 + ( nthreads * nIter ) ) );
}
