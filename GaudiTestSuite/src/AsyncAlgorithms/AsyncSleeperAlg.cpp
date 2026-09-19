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
#include <Gaudi/AsynchronousAlgorithm.h>
#include <Gaudi/Property.h>
#include <chrono>

namespace Gaudi::TestSuite {
  class AsyncSleeperAlg : public Gaudi::AsynchronousAlgorithm {
  public:
    using Gaudi::AsynchronousAlgorithm::AsynchronousAlgorithm;
    StatusCode execute( const EventContext& ) const override {
      // Normal blocking sleep for the specified duration to simulate work being done
      std::this_thread::sleep_for( std::chrono::microseconds( m_sleepDuration.value() ) );
      info() << "Will suspend execution with a sleep " << m_sleepNumber.value() << " time(s), each for around "
             << m_sleepDuration.value() << " microseconds" << endmsg;
      for ( int i = 0; i < m_sleepNumber.value(); ++i ) {
        auto start = std::chrono::high_resolution_clock::now();
        // Suspend the fiber for the specified duration to simulate asynchronous work being done
        auto sc   = sleep_for( std::chrono::microseconds( m_sleepDuration.value() ) );
        auto stop = std::chrono::high_resolution_clock::now();
        if ( !sc ) {
          error() << "Failed to suspend execution: " << sc << endmsg;
          return sc;
        }
        info() << "Suspended execution for "
               << std::chrono::duration_cast<std::chrono::microseconds>( stop - start ).count() << " microseconds"
               << endmsg;
        // Normal blocking sleep for the specified duration to simulate work being done
        std::this_thread::sleep_for( std::chrono::microseconds( m_sleepDuration.value() ) );
      }
      return StatusCode::SUCCESS;
    }

  private:
    Gaudi::Property<int> m_sleepDuration{ this, "SuspensionDuration", 100, "Suspension duration in microseconds" };
    Gaudi::Property<int> m_sleepNumber{ this, "SuspensionPoints", 1, "Number of suspension points" };
  };
  DECLARE_COMPONENT( AsyncSleeperAlg )

} // namespace Gaudi::TestSuite
