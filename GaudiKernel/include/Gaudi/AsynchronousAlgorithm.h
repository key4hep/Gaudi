/***********************************************************************************\
* (c) Copyright 2023-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
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
#include <Gaudi/Property.h>
#include <GaudiKernel/Guards.h>
#include <GaudiKernel/ITimelineSvc.h>

#include <boost/fiber/fss.hpp>        // for fiber_specific_ptr
#include <boost/fiber/operations.hpp> // for this_fiber
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
    [[deprecated( "Wrap suspending or possibly suspending calls with `decorateSuspension` instead of following them "
                  "with a call to `restoreAfterSuspend`." )]] virtual StatusCode
    restoreAfterSuspend() const;

    /// Decorate a direct call to a suspending (or possibly suspending) function with the necessary pre- and
    /// post-suspension steps
    StatusCode decorateSuspension( std::function<StatusCode()> f ) const;

    /// Forwards to boost::this_fiber::yield
    StatusCode yield() const;

    /// Forwards to boost::this_fiber::sleep_until
    template <typename Clock, typename Duration>
    StatusCode sleep_until( std::chrono::time_point<Clock, Duration> const& sleep_time ) const {
      return decorateSuspension( [sleep_time]() {
        boost::this_fiber::sleep_until( sleep_time );
        return StatusCode::SUCCESS;
      } );
    }

    /// Forwards to boost::this_fiber::sleep_for
    template <typename Rep, typename Period>
    StatusCode sleep_for( std::chrono::duration<Rep, Period> const& dur ) const {
      return decorateSuspension( [dur]() {
        boost::this_fiber::sleep_for( dur );
        return StatusCode::SUCCESS;
      } );
    }

  protected:
    virtual StatusCode preSuspension() const;
    virtual StatusCode postResumption() const;

  private:
    /// Contains current slot
    boost::fibers::fiber_specific_ptr<EventContext>                           s_ctx{};
    mutable boost::fibers::fiber_specific_ptr<ITimelineSvc::TimelineRecorder> s_timelineRecorder{};

    // Flag to control asynchronous timeline recording
    bool m_doAsyncTimeline{ false };

    Gaudi::Property<bool> m_auditorSuspension{ this, "AuditSuspension",
                                               Details::getDefaultAuditorValue( serviceLocator() ),
                                               "trigger auditor on suspension of execution" };
  };
} // namespace Gaudi
