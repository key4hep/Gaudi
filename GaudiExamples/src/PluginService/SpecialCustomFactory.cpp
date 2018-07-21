#include "Gaudi/PluginService.h"
#include "GaudiAlg/GaudiAlgorithm.h"

#include <iostream>

namespace PluginServiceTest
{
  class CustomFactoryAlgorithm : public GaudiAlgorithm
  {
  public:
    using GaudiAlgorithm::GaudiAlgorithm;

    StatusCode execute() override
    {
      info() << "running" << endmsg;
      return StatusCode::SUCCESS;
    }
  };
}

namespace
{
  using PluginServiceTest::CustomFactoryAlgorithm;

  std::unique_ptr<IAlgorithm> creator( const std::string& name, ISvcLocator* svcLoc )
  {
    auto p = std::make_unique<CustomFactoryAlgorithm>( name, svcLoc );

    // do not print messages if we are created in genconf
    const std::string cmd = System::cmdLineArgs()[0];
    if ( cmd.find( "genconf" ) == std::string::npos ) {
      std::cout << "created CustomFactoryAlgorithm at " << p.get() << std::endl;
    }

    return std::move( p );
  }

  Gaudi::PluginService::DeclareFactory<CustomFactoryAlgorithm> _{creator};
}
