/***********************************************************************************\
* (c) Copyright 2023-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <Gaudi/Algorithm.h>
#include <GaudiKernel/IHiveWhiteBoard.h>
#include <boost/fiber/all.hpp>
#include <chrono>

namespace Gaudi {
  /** Base class for asynchronous algorithms.
   *
   *  Augments Gaudi::Algorithm by saving and restoring current slot whenever
   *  fiber is suspended and resumed. This requires using the member functions for
   *  suspending instead of the boost::fiber functions directly.
   *
   *  @author Beojan Stanislaus
   *  @date 2023
   */

  class GAUDI_API AsynchronousAlgorithm : public Gaudi::Algorithm {
  public:
    using Gaudi::Algorithm::Algorithm;
    StatusCode sysInitialize() override;
    StatusCode sysExecute( const EventContext& ctx ) override;

    /// Restore after suspend
    virtual StatusCode restoreAfterSuspend() const;

    /// Forwards to boost::this_fiber::yield
    StatusCode yield() const;

    /// Forwards to boost::this_fiber::sleep_until
    template <typename Clock, typename Duration>
    StatusCode sleep_until( std::chrono::time_point<Clock, Duration> const& sleep_time ) const {
      boost::this_fiber::sleep_until( sleep_time );
      return restoreAfterSuspend();
    }

    /// Forwards to boost::this_fiber::sleep_for
    template <typename Rep, typename Period>
    StatusCode sleep_for( std::chrono::duration<Rep, Period> const& dur ) const {
      boost::this_fiber::sleep_for( dur );
      return restoreAfterSuspend();
    }

  private:
    /// Contains current slot
    boost::fibers::fiber_specific_ptr<std::size_t> s_currentSlot{};
  };
} // namespace Gaudi
