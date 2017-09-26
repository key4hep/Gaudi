/// Test file used to check if the new Plugin Service is able to handle the
/// strings used in the old one.
/// For example the class PluginServiceTest::MyAlg was identified by the string
/// "PluginServiceTest__MyAlg".

#include "GaudiKernel/Algorithm.h"

#include <string>
#include <vector>

namespace PluginServiceTest
{
  class MyAlg : public Algorithm
  {
  public:
    MyAlg( const std::string& name, ISvcLocator* svcloc ) : Algorithm( name, svcloc ) {}
    ~MyAlg() override {}

    StatusCode execute() override { return StatusCode::SUCCESS; }
  };

  template <class T1, class T2>
  class MyTemplatedAlg : public Algorithm
  {
  public:
    MyTemplatedAlg( const std::string& name, ISvcLocator* svcloc ) : Algorithm( name, svcloc ) {}
    ~MyTemplatedAlg() override {}

    StatusCode execute() override { return StatusCode::SUCCESS; }
  };
}

DECLARE_COMPONENT( PluginServiceTest::MyAlg )
namespace
{
  typedef PluginServiceTest::MyTemplatedAlg<int&, double*> _dummy;
}
DECLARE_COMPONENT( _dummy )
