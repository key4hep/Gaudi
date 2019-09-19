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
