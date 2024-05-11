/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Functional/Transformer.h>
#include <Gaudi/Utils/PeriodicAction.h>
#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/Memory.h>
#include <TSystem.h>
#include <algorithm>
#include <chrono>
#include <csignal>
#include <fmt/format.h>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace {
  /// replace occurrences of `:` with `_` in a string
  std::string sanitize( std::string_view input ) {
    std::string output{ input };
    std::replace( begin( output ), end( output ), ':', '_' );
    return output;
  }

  // mutex to prevent reports from the watchdog to step on each other,
  // for example requiring a stack trace while already producing one
  std::mutex s_watchdogReportMutex;
} // namespace

using Gaudi::Utils::PeriodicAction;

namespace Gaudi {

  /// Add to the transient store a tracker that detects events that are taking too long.
  class EventWatchdogAlg : public Gaudi::Functional::Transformer<PeriodicAction( EventContext const& )> {
  public:
    EventWatchdogAlg( const std::string& name, ISvcLocator* pSvcLocator )
        : Transformer( name, pSvcLocator, KeyValue{ "TimerLocation", fmt::format( ".{}-timer", sanitize( name ) ) } ) {
      // timeout period cannot be smaller than 1
      m_eventTimeout.verifier().setLower( 1 );
    }

    PeriodicAction operator()( EventContext const& ) const override {
      using namespace std::chrono_literals;
      // we use a functor because we cannot pass mutable states to a lambda
      struct Action {
        MsgStream          log;
        const bool         doStackTrace;
        const bool         abortOnTimeout;
        const unsigned int timeout;
        int                counter{ 0 };

        const std::chrono::steady_clock::time_point eventStart{ std::chrono::steady_clock::now() };

        void operator()() {
          ++counter;
          if ( counter == 1 ) {
            log << MSG::WARNING << fmt::format( "More than {}s since the beginning of the event", timeout ) << endmsg;
          } else {
            log << MSG::WARNING << fmt::format( "Another {}s passed since last timeout", timeout ) << endmsg;
          }
          if ( log.level() <= MSG::INFO ) {
            log << MSG::INFO
                << fmt::format( "Current memory usage is virtual size = {} MB, resident set size = {} MB",
                                System::virtualMemory() / 1024., System::pagedMemory() / 1024. )
                << endmsg;
          }

          std::scoped_lock protectReport( s_watchdogReportMutex );
          if ( doStackTrace && gSystem ) {
            // TSystem::StackTrace() prints on the standard error, so we do the same
            std::cerr << "=== Stalled event: current stack trace ===" << std::endl;
            gSystem->StackTrace();
          }
          if ( abortOnTimeout ) {
            log << MSG::FATAL << "too much time on a single event: aborting process" << endmsg;
            std::raise( SIGQUIT );
          }
        }

        ~Action() {
          if ( counter ) {
            const std::chrono::duration<float, std::chrono::seconds::period> duration =
                std::chrono::steady_clock::now() - eventStart;
            log << MSG::INFO << fmt::format( "The last event took {:.3f}s", duration.count() ) << endmsg;
          }
        }
      };
      Action action{ MsgStream{ msgSvc(), "EventWatchdog" }, m_stackTrace, m_abortOnTimeout, m_eventTimeout };
      return PeriodicAction( std::move( action ), std::chrono::seconds{ m_eventTimeout } );
    }

  private:
    Gaudi::CheckedProperty<unsigned int> m_eventTimeout{ this, "EventTimeout", 600,
                                                         "Number of seconds allowed to process a single event." };
    Gaudi::Property<bool>                m_abortOnTimeout{ this, "AbortOnTimeout", false,
                                            "If set to true, the application is killed when we reach the timeout." };
    Gaudi::Property<bool> m_stackTrace{ this, "StackTrace", false, "Whether to print the stack-trace on timeout." };
  };
  DECLARE_COMPONENT( EventWatchdogAlg )
} // namespace Gaudi
