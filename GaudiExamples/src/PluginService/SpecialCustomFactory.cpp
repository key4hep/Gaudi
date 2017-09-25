#include "Gaudi/PluginService.h"
#include "GaudiAlg/GaudiAlgorithm.h"

#include <iostream>

namespace PluginServiceTest
{
  class CustomFactoryAlgorithm : public GaudiAlgorithm
  {
  public:
    CustomFactoryAlgorithm( const std::string& name, ISvcLocator* svcloc ) : GaudiAlgorithm( name, svcloc ) {}
    ~CustomFactoryAlgorithm() override {}

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
  class _register__CustomFactoryAlgorithm
  {
  public:
    typedef Algorithm::Factory s_t;
    static IAlgorithm* creator( const std::string& name, ISvcLocator*&& svcLoc )
    {
      CustomFactoryAlgorithm* p = new CustomFactoryAlgorithm( name, svcLoc );
      std::cout << "created CustomFactoryAlgorithm at " << p << std::endl;
      return p;
    }
    _register__CustomFactoryAlgorithm()
    {
      using ::Gaudi::PluginService::Details::Registry;
      Registry::instance().add<s_t, CustomFactoryAlgorithm>(
          ::Gaudi::PluginService::Details::demangle<CustomFactoryAlgorithm>(), creator );
    }
  } _register__CustomFactoryAlgorithm;
}
