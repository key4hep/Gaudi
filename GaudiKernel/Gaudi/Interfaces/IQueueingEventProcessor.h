/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/IEventProcessor.h>
#include <GaudiKernel/StatusCode.h>
#include <optional>
#include <tuple>

namespace Gaudi::Interfaces {
  /// Extend IEventProcessor for asynchronous scheduling of events via queue-like interface.
  struct IQueueingEventProcessor : virtual IEventProcessor {
    DeclareInterfaceID( IQueueingEventProcessor, 1, 0 );

    /// Type of objects in the output queue.
    using ResultType = std::tuple<StatusCode, EventContext>;

    /// Schedule the processing of an event.
    /// May block, depending on the concrete implementation.
    virtual void push( EventContext&& ctx ) = 0;

    /// Tell if the processor has events in the queues.
    /// Useful to know when all the work has been done (including retrieving the results).
    virtual bool empty() const = 0;

    /// Get the next available result.
    /// If none is available, return std::nullopt.
    virtual std::optional<ResultType> pop() = 0;
  };
} // namespace Gaudi::Interfaces
