#pragma once

#include <Gaudi/PluginService.h>
#include <GaudiKernel/IStateful.h>
#include <GaudiKernel/SmartIF.h>
#include <map>
#include <string>
#include <string_view>

namespace Gaudi {
  /// Gaudi application entry point.
  ///
  /// Gaudi::Application can be used to bootstrap a standard Gaudi application or to implement
  /// custom applications, either via a specialization that overrides the method run (which can
  /// be instantiated either directly or via the GaudiPluginService, with the helper `create`) or
  /// by passing a callable object to the dedicated `run` method.
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

    /// Run a user provided implementation of the application main logic.
    int run( std::function<int( SmartIF<IStateful>& )> action ) { return action( app ); }

  protected:
    /// Handle to the ApplicationMgr instance.
    SmartIF<IStateful> app;
  };
} // namespace Gaudi