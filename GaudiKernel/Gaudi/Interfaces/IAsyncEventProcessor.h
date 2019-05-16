#pragma once

#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/IEventProcessor.h>
#include <future>
#include <tuple>

namespace Gaudi::Interfaces {
  /// Extend IEventProcessor for asynchronous scheduling of events.
  struct IAsyncEventProcessor : virtual IEventProcessor {
    DeclareInterfaceID( IAsyncEventProcessor, 1, 0 );

    /// Schedule the processing of the given event.
    /// When the event is completed, the returned future will contain the result of
    /// the processing (StatusCode) and the passed EventContext.
    virtual std::future<std::tuple<StatusCode, EventContext>> asyncExecuteEvent( EventContext&& ctx ) = 0;
  };
} // namespace Gaudi::Interfaces
