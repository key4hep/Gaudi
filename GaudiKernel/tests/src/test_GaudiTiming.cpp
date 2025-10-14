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
 * Time_test.cpp
 *
 *  Created on: Jan 13, 2011
 *      Author: Marco Clemencic
 */

#include <GaudiKernel/ChronoEntity.h>
#include <GaudiKernel/Memory.h>
#include <GaudiKernel/Sleep.h>
#include <GaudiKernel/Timing.h>
#include <cmath>

#if __has_include( <catch2/catch.hpp>)
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_test_macros.hpp>
#endif

TEST_CASE( "System time adjustment", "[GaudiTiming]" ) {
  long long t = 12345678901234560LL; // units of 100 nanoseconds
  // 1234567890123456000 nanoseconds
  //    1234567890123456 microseconds
  //       1234567890123 milliseconds
  //          1234567890 seconds
  //            20576131 minutes
  //              342935 hours
  //               14288 days
  //                 476 months (of 30 days)
  //                  39 years (of 365 days)
  //   12345678901234560 native
  CHECK( System::adjustTime<System::nanoSec>( t ) == 1234567890123456000LL );
  CHECK( System::adjustTime<System::microSec>( t ) == 1234567890123456LL );
  CHECK( System::adjustTime<System::milliSec>( t ) == 1234567890123LL );
  CHECK( System::adjustTime<System::Sec>( t ) == 1234567890LL );
  CHECK( System::adjustTime<System::Min>( t ) == 20576131LL );
  CHECK( System::adjustTime<System::Hour>( t ) == 342935LL );
  CHECK( System::adjustTime<System::Day>( t ) == 14288LL );
  CHECK( System::adjustTime<System::Month>( t ) == 476LL );
  CHECK( System::adjustTime<System::Year>( t ) == 39LL );
  CHECK( System::adjustTime<System::Native>( t ) == 12345678901234560LL );
}

TEST_CASE( "Current time test", "[GaudiTiming]" ) {
  long long now = System::currentTime( System::microSec );
  CHECK( now ); // non-zero

  Gaudi::Sleep( 1 ); // wait a wee bit

  long long later = System::currentTime( System::microSec );
  CHECK( later ); // non-zero

  CHECK( later > now );
  float seconds = float( later - now ) / 1000000;
  CHECK( fabs( seconds - 1.f ) < 0.01 );
}

TEST_CASE( "ProcessTime test", "[GaudiTiming]" ) {
  System::ProcessTime t0 = System::getProcessTime();
  Gaudi::Sleep( 1 );
  System::ProcessTime t1 = System::getProcessTime();

  // light check on the conversion
  CHECK( t1.kernelTime<System::Sec>() == t1.kernelTime<System::milliSec>() / 1000 );
  // measure the elapsed time
  CHECK( ( t1 - t0 ).elapsedTime<System::Sec>() == 1LL );

  // This should result in non-zero user and kernel times
  t0       = System::getProcessTime();
  double x = 1.5;

  // this variable is only needed to ensure that the timed loop
  // takes a measurable time
  [[maybe_unused]] long m = 0;
  for ( int i = 0; i < 10000; i++ ) {
    x *= sin( x ) / atan( x ) * tanh( x ) * sqrt( x );
    m += System::virtualMemory();
  }
  t1 = System::getProcessTime();

  CHECK( ( t1 - t0 ).userTime<System::microSec>() > 0 );
  CHECK( ( t1 - t0 ).kernelTime<System::microSec>() > 0 );
}

TEST_CASE( "ProcessTime performance test", "[GaudiTiming]" ) {
  System::ProcessTime t, t0, t1;

  t0    = System::getProcessTime();
  int N = 1e6;
  for ( int i = 0; i < N; i++ ) { t = System::getProcessTime(); }
  t1 = System::getProcessTime();
  std::cout << " (" << ( t1 - t0 ).elapsedTime<System::nanoSec>() / N << " ns per call)";
  // Just check that we can measure something
  CHECK( ( t1 - t0 ).elapsedTime<System::nanoSec>() > 0 );
}

TEST_CASE( "ChronoEntity test", "[GaudiTiming]" ) {
  ChronoEntity c1;
  for ( int i = 0; i < 10; i++ ) {
    c1.start();
    Gaudi::NanoSleep( 1e7 ); // 10ms
    c1.stop();
  }
  CHECK( c1.nOfMeasurements() == 10UL );
  // average time may be affected by the load of the machine
  CHECK( c1.eMeanTime() >= c1.eMinimalTime() );
  CHECK( c1.eMeanTime() <= c1.eMaximalTime() );
  CHECK( c1.eMeanTime() >= 8000 );
  CHECK( c1.eMeanTime() <= 19000 );
  CHECK( c1.eMaximalTime() >= c1.eMinimalTime() );

  ChronoEntity c2;
  c2 += c1;
  c2 += c1;
  CHECK( c1 < c2 );

  // Performance
  ChronoEntity        c3;
  System::ProcessTime t0 = System::getProcessTime();
  int                 N  = 1e6;
  for ( int i = 0; i < N; i++ ) {
    c3.start();
    c3.stop();
  }
  System::ProcessTime t1 = System::getProcessTime();
  std::cout << " (" << ( t1 - t0 ).elapsedTime<System::nanoSec>() / N << " ns per stop/start)";
  // Just check that we can measure something
  CHECK( ( t1 - t0 ).elapsedTime<System::nanoSec>() > 0 );
}
