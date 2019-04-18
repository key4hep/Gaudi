#pragma once

#include <Gaudi/PluginService.h>
#include <GaudiKernel/IStateful.h>
#include <GaudiKernel/SmartIF.h>
#include <map>
#include <string>
#include <string_view>

namespace Gaudi {
  /// Standard Gaudi application main logic.
  /// It can be specialized to override the steering of the event loop (via the method `run`).
  class Application {
  public:
    using Options = std::map<std::string, std::string>;
    using Factory = Gaudi::PluginService::Factory<Application( Options )>;

    /// Construct and configure the application from the provided options.
    Application( Options opts );
    virtual ~Application();

    /// Factory function to instantiate a derived class via GaudiPluginService.
    static Factory::ReturnType create( std::string_view type, Options opts );

    /// Implement the application main logic:
    /// - prepare for processing (initialize + start)
    /// - loop over events
    /// - terminate (stop + finalize)
    virtual int run();

  protected:
    /// Handle to the ApplicationMgr instance.
    SmartIF<IStateful> app;
  };
} // namespace Gaudi