/***********************************************************************************\
* (c) Copyright 2025 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Application.h>
#include <GaudiKernel/IRndmGenSvc.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/RndmGenerators.h>
#include <atomic>
#include <chrono>
#include <format>
#include <thread>

constexpr int N_threads = 10;
constexpr int N_numbers = 10000;

int main() {
  Gaudi::Application::Options opts;
  opts["ApplicationMgr.EvtSel"]         = "NONE";
  opts["ApplicationMgr.JobOptionsType"] = "NONE";

  // This makes the random number generator thread-safe (default)
  opts["RndmGenSvc.ThreadSafe"] = "true";

  auto app = Gaudi::Application::create( "Gaudi::Application", std::move( opts ) );

  return app->run( []( SmartIF<IStateful>& app ) {
    app->initialize().orThrow( "failed to initialize", "main" );
    auto svcLoc = app.as<ISvcLocator>();
    assert( svcLoc );
    auto randomSvc = svcLoc->service<IRndmGenSvc>( "RndmGenSvc" );
    assert( randomSvc );
    auto      msgSvc = app.as<IMessageSvc>();
    MsgStream log( msgSvc, "test_random_mt" );
    log << MSG::INFO << "Starting test_random_mt" << endmsg;
    std::atomic<double> global_sum{};

    std::chrono::time_point<std::chrono::high_resolution_clock> global_start;
    {
      Rndm::Numbers dist{};
      dist.initialize( randomSvc, Rndm::Flat( 0.0, 1.0 ) )
          .orThrow( "failed to initialize random number distribution", "main" );
      std::vector<std::jthread> threads;
      threads.reserve( 10 );

      global_start = std::chrono::high_resolution_clock::now();
      for ( int n = 0; n < N_threads; ++n ) {
        threads.emplace_back( [&dist, n, &msgSvc, &global_sum]() {
          double sum   = 0.0;
          auto   start = std::chrono::high_resolution_clock::now();
          for ( int i = 0; i < N_numbers; ++i ) { sum += dist.shoot(); }
          auto elapsed = std::chrono::high_resolution_clock::now() - start;
          global_sum += sum / N_numbers;
          MsgStream log( msgSvc, std::format( "thread {}", n ) );
          log << MSG::INFO << "produced " << N_numbers << " numbers in "
              << std::chrono::duration_cast<std::chrono::microseconds>( elapsed ).count() << " us" << endmsg;
        } );
      }
    }
    auto global_elapsed = std::chrono::high_resolution_clock::now() - global_start;
    log << MSG::INFO << "Finished test_random_mt in "
        << std::chrono::duration_cast<std::chrono::microseconds>( global_elapsed ).count() << " us" << endmsg;

    auto avg = global_sum.load() / N_threads;
    log << MSG::INFO << "average = " << avg << endmsg;

    int ret_value = 0;
    if ( avg >= 0.496768 && avg <= 0.496770 ) {
      log << MSG::INFO << "average is OK (0.496769)" << endmsg;
      ret_value = 0;
    } else {
      log << MSG::ERROR << "average is outside expected range" << endmsg;
      ret_value = 1;
    }

    app->finalize().orThrow( "failed to finalize", "main" );
    return ret_value;
  } );
}
