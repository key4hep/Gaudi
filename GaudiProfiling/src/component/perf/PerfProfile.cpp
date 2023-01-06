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
#include "GaudiAlg/Consumer.h"
#include <fcntl.h>
#include <string_view>
#include <sys/stat.h>

/** Algorithm to enable/disable profiling with Linux perf at given events.
 *
 *  Needs at least perf 5.9. To control perf record, start it as
 *
 *    perf record -D -1 --control fifo:GaudiPerfProfile.fifo ... gaudirun.py ...
 *
 *  The path to the control fifo (GaudiPerfProfile.fifo) is configurable
 *  with the FIFOPath property. The fifo must be created before running perf.
 *
 */
struct PerfProfile final : Gaudi::Functional::Consumer<void()> {
  using Consumer::Consumer;

  StatusCode initialize() override {
    return Consumer::initialize().andThen( [this]() {
      m_fifo = ::open( m_fifoPath.value().c_str(), O_WRONLY | O_NONBLOCK );
      if ( m_fifo == -1 ) {
        fatal() << "open(\"" << m_fifoPath.value() << "\"): " << ::strerror( errno ) << endmsg;
        return StatusCode::FAILURE;
      }
      return StatusCode::SUCCESS;
    } );
  }

  StatusCode finalize() override {
    if ( m_fifo != -1 ) { ::close( m_fifo ); }
    return Consumer::finalize();
  }

  void operator()() const override {
    // We could use EventContext::evt(), however it is not always valid, as is the case with EvtSel="NONE". Instead, use
    // an atomic counter.
    auto eventNumber = m_eventNumber++;
    if ( eventNumber == m_nStartFromEvent.value() ) {
      warning() << "Starting perf profile at event " << eventNumber << endmsg;
      fifo_write( "enable\n" );
    }

    if ( m_nStopAtEvent > 0 && eventNumber == m_nStopAtEvent.value() ) {
      warning() << "Stopping perf profile at event " << eventNumber << endmsg;
      fifo_write( "disable\n" );
    }
  }

private:
  void fifo_write( std::string_view s ) const {
    if ( ::write( m_fifo, s.data(), s.size() ) < ssize_t( s.size() ) ) {
      error() << "Write of \"" << s << "\" to FIFO failed: " << ::strerror( errno ) << endmsg;
    }
  }

  mutable std::atomic<long unsigned> m_eventNumber = 0;
  int                                m_fifo        = -1;

  Gaudi::Property<std::string>   m_fifoPath{ this, "FIFOPath", "GaudiPerfProfile.fifo", "Path to perf control FIFO." };
  Gaudi::Property<unsigned long> m_nStartFromEvent{ this, "StartFromEventN", 1,
                                                    "After what event we start profiling." };
  Gaudi::Property<unsigned long> m_nStopAtEvent{
      this, "StopAtEventN", 0,
      "After what event we stop profiling. If 0 than we also profile finalization stage. Default = 0." };
};

DECLARE_COMPONENT( PerfProfile )
