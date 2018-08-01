/*****************************************************************************\
* (c) Copyright 2013 CERN                                                     *
*                                                                             *
* This software is distributed under the terms of the GNU General Public      *
* Licence version 3 (GPL Version 3), copied verbatim in the file "LICENCE".   *
*                                                                             *
* In applying this licence, CERN does not waive the privileges and immunities *
* granted to it by virtue of its status as an Intergovernmental Organization  *
* or submit itself to any jurisdiction.                                       *
\*****************************************************************************/
/**
 * Compile-time test for all known PluginService use-cases
 *
 * @author Marco Clemencic <marco.clemencic@cern.ch>
 */

#define GAUDI_PLUGIN_SERVICE_V2

#include <Gaudi/PluginService.h>

// standard use, 0 arguments
class Base
{
public:
  typedef Gaudi::PluginService::Factory<Base*()> Factory;
  virtual ~Base() {}
};
class Component0 : public Base
{
};
DECLARE_COMPONENT( Component0 )

class Component1 : public Base
{
};

#define DECLARE_COMPONENT_WITH_PROPS( type )                                                                           \
  namespace                                                                                                            \
  {                                                                                                                    \
    ::Gaudi::PluginService::DeclareFactory<type> _INTERNAL_FACTORY_REGISTER_CNAME{{{"name", #type}}};                  \
  }

DECLARE_COMPONENT_WITH_PROPS( Component1 )

// standard use, 2 arguments
class Base2
{
public:
  typedef Gaudi::PluginService::Factory<Base2*( const std::string&, int )> Factory;
  virtual ~Base2() {}
  virtual void abstractMethod() = 0;
};

class Component2 : public Base2
{
public:
  Component2( std::string _s, int _i ) : i( _i ), s( std::move( _s ) ) {}
  void abstractMethod() override {}

  int         i;
  std::string s;
};

DECLARE_COMPONENT( Component2 )

// namespaces
namespace Test
{
  class ComponentA : public Base
  {
  };
  class ComponentB : public Base
  {
  };
  class ComponentC : public Base
  {
  };
  class ComponentD : public Base
  {
  };
}

DECLARE_COMPONENT( Test::ComponentA )

namespace
{
  using Test::ComponentB;
  DECLARE_COMPONENT( ComponentB )
}

namespace Test
{
  DECLARE_COMPONENT( ComponentC )
}

namespace
{
  using TC = Test::ComponentD;
  DECLARE_COMPONENT( TC )
}

// using ids
DECLARE_COMPONENT_WITH_ID( Component2, "Id2" )
DECLARE_COMPONENT_WITH_ID( Test::ComponentB, "B" )

// explicit factory
DECLARE_FACTORY_WITH_ID( Test::ComponentA, "A", Base::Factory )

// custom factory example
// -- declaration --
struct MyInterface {
  virtual ~MyInterface() = default;

  virtual const std::string& name() const = 0;
};

struct BaseSetupHelper;

struct MyBase : MyInterface {
  using Factory = Gaudi::PluginService::Factory<MyInterface*( const std::string& )>;

  const std::string& name() const override { return m_name; }
private:
  friend BaseSetupHelper;
  std::string m_name;
};

// -- implementation --
struct MyComponent : MyBase {
  MyComponent() {}
};

struct BaseSetupHelper {
  static void setName( MyBase* base, const std::string& name ) { base->m_name = name; }
};

namespace
{
  std::unique_ptr<MyInterface> creator( const std::string& name )
  {
    auto p = std::make_unique<MyComponent>();
    BaseSetupHelper::setName( p.get(), name );
    return std::move( p );
  }
  Gaudi::PluginService::DeclareFactory<MyComponent> _{creator};
}

// -- use --
void useComponent()
{
  auto c = MyBase::Factory::create( "MyComponent", "TheName" );
  // ...
}

// factory from lambda
namespace SpecialId
{
  struct MyComponent : Base {
    MyComponent( std::string n ) : name{std::move( n )} {}

    std::string name;
  };
  using namespace Gaudi::PluginService;
  DeclareFactory<MyComponent> __some_random_name( "special-id",
                                                  []() -> MyComponent::Factory::ReturnType {
                                                    return std::make_unique<MyComponent>( "special-id" );
                                                  },
                                                  {{"MyProperty", "special"}} );
}

// customized factory wrapper
namespace CustomFactoryWrapper
{
  class Base;

  // helper to initialize base class members
  void initBase( Base*, const std::string& );

  // helper to use the default constructor of T, followed by initialization with initBase
  template <typename T>
  std::enable_if_t<std::is_default_constructible<T>::value, std::unique_ptr<Base>>
  baseConstructorHelper( const std::string& name )
  {
    auto p = std::make_unique<T>();
    initBase( p.get(), name );
    return std::move( p );
  }

  // helper to use the special constructor of T (backward compatibility)
  template <typename T>
  std::enable_if_t<!std::is_default_constructible<T>::value, std::unique_ptr<Base>>
  baseConstructorHelper( const std::string& name )
  {
    return std::make_unique<T>( name );
  }
}

namespace Gaudi
{
  namespace PluginService
  {
    GAUDI_PLUGIN_SERVICE_V2_INLINE namespace v2
    {
      namespace Details
      {
        // custom implementation of DefaultFactory to wrap the call to T constructor
        template <typename T>
        struct DefaultFactory<T, Factory<CustomFactoryWrapper::Base*( const std::string& )>> {
          inline typename Factory<CustomFactoryWrapper::Base*( const std::string& )>::ReturnType
          operator()( const std::string& name )
          {
            return CustomFactoryWrapper::baseConstructorHelper<T>( name );
          }
        };
      }
    }
  }
}

namespace CustomFactoryWrapper
{
  class Base
  {
    friend void initBase( Base*, const std::string& );
    std::string m_name;

  public:
    using Factory = Gaudi::PluginService::Factory<Base*( const std::string& name )>;

    Base() {}
    Base( const std::string& name ) : m_name{name} {}

    const std::string& name() const { return m_name; }
  };

  void initBase( Base* b, const std::string& name ) { b->m_name = name; }

  struct ComponentNew : Base {
  };
  struct ComponentOld : Base {
    ComponentOld( const std::string& name ) : Base{name} {}
  };
  DECLARE_COMPONENT( ComponentNew )
  DECLARE_COMPONENT( ComponentOld )
}

// ATLAS Custom factories
// see http://acode-browser1.usatlas.bnl.gov/lxr/source/athena/Control/AthenaKernel/AthenaKernel/TPCnvFactory.h
// see http://acode-browser1.usatlas.bnl.gov/lxr/source/athena/Control/AthenaServices/src/test/testConverters.cxx
namespace Athena
{
  struct TPCnvVers {
    enum Value { Old = 0, Current = 1 };
  };

  struct TPCnvType {
    enum Value { Athena = 0, ARA = 1, Trigger = 2 };
  };
}
struct ITPCnvBase {
  typedef Gaudi::PluginService::Factory<ITPCnvBase*()> Factory;
};

#define DO_ATHTPCNV_FACTORY_REGISTER_CNAME( name, serial ) _register_##_##serial

#define DO_ATHTPCNV_PLUGINSVC_FACTORY_WITH_ID( type, id, trans_type, pers_type, is_last_version, cnv_type, signature,  \
                                               serial )                                                                \
  namespace                                                                                                            \
  {                                                                                                                    \
    struct DO_ATHTPCNV_FACTORY_REGISTER_CNAME( type, serial ) {                                                        \
      DO_ATHTPCNV_FACTORY_REGISTER_CNAME( type, serial )()                                                             \
      {                                                                                                                \
        using ::Gaudi::PluginService::DeclareFactory;                                                                  \
        std::string prefix;                                                                                            \
        if ( cnv_type == Athena::TPCnvType::ARA )                                                                      \
          prefix = "_ARA";                                                                                             \
        else if ( cnv_type == Athena::TPCnvType::Trigger )                                                             \
          prefix = "_TRIG";                                                                                            \
        DeclareFactory<type> normal{};                                                                                 \
        if ( is_last_version == Athena::TPCnvVers::Current )                                                           \
          DeclareFactory<type> transient{prefix + "_TRANS_" + #trans_type};                                            \
        DeclareFactory<type>   persistent{prefix + "_PERS_" + #pers_type};                                             \
      }                                                                                                                \
    } DO_ATHTPCNV_FACTORY_REGISTER_CNAME( s_##type, serial );                                                          \
  }

#define DO_ATHTPCNV_PLUGINSVC_FACTORY( type, trans_type, pers_type, is_last_version, cnv_type, signature, serial )     \
  DO_ATHTPCNV_PLUGINSVC_FACTORY_WITH_ID( type, ::Gaudi::PluginService::Details::demangle<type>(), trans_type,          \
                                         pers_type, is_last_version, cnv_type, signature, serial )

#define ATHTPCNV_PLUGINSVC_FACTORY( type, trans_type, pers_type, is_last_version, signature )                          \
  DO_ATHTPCNV_PLUGINSVC_FACTORY( type, trans_type, pers_type, is_last_version, Athena::TPCnvType::Athena, signature,   \
                                 __LINE__ )
#define ARATPCNV_PLUGINSVC_FACTORY( type, trans_type, pers_type, is_last_version, signature )                          \
  DO_ATHTPCNV_PLUGINSVC_FACTORY( type, trans_type, pers_type, is_last_version, Athena::TPCnvType::ARA, signature,      \
                                 __LINE__ )
#define TRIGTPCNV_PLUGINSVC_FACTORY( type, trans_type, pers_type, is_last_version, signature )                         \
  DO_ATHTPCNV_PLUGINSVC_FACTORY( type, trans_type, pers_type, is_last_version, Athena::TPCnvType::Trigger, signature,  \
                                 __LINE__ )

#define ATHTPCNV_PLUGINSVC_FACTORY_WITH_ID( type, id, trans_type, pers_type, is_last_version, signature )              \
  DO_ATHTPCNV_PLUGINSVC_FACTORY_WITH_ID( type, id, trans_type, pers_type, is_last_version, Athena::TPCnvType::Athena,  \
                                         signature, __LINE__ )
#define ARATPCNV_PLUGINSVC_FACTORY_WITH_ID( type, id, trans_type, pers_type, is_last_version, signature )              \
  DO_ATHTPCNV_PLUGINSVC_FACTORY_WITH_ID( type, id, trans_type, pers_type, is_last_version, Athena::TPCnvType::ARA,     \
                                         signature, __LINE__ )
#define TRIGTPCNV_PLUGINSVC_FACTORY_WITH_ID( type, id, trans_type, pers_type, is_last_version, signature )             \
  DO_ATHTPCNV_PLUGINSVC_FACTORY_WITH_ID( type, id, trans_type, pers_type, is_last_version, Athena::TPCnvType::Trigger, \
                                         signature, __LINE__ )

//********************************************************************
// Macros that users should use.
//

#define DECLARE_TPCNV_FACTORY( x, trans_type, pers_type, is_last_version )                                             \
  ATHTPCNV_PLUGINSVC_FACTORY( x, trans_type, pers_type, is_last_version, ITPCnvBase*() )

#define DECLARE_ARATPCNV_FACTORY( x, trans_type, pers_type, is_last_version )                                          \
  ARATPCNV_PLUGINSVC_FACTORY( x, trans_type, pers_type, is_last_version, ITPCnvBase*() )

#define DECLARE_TRIGTPCNV_FACTORY( x, trans_type, pers_type, is_last_version )                                         \
  TRIGTPCNV_PLUGINSVC_FACTORY( x, trans_type, pers_type, is_last_version, ITPCnvBase*() )

#define DECLARE_NAMED_TPCNV_FACTORY( x, n, trans_type, pers_type, is_last_version )                                    \
  ATHTPCNV_PLUGINSVC_FACTORY_WITH_ID( x, std::string( #n ), trans_type, pers_type, is_last_version, ITPCnvBase*() )

#define DECLARE_NAMED_ARATPCNV_FACTORY( x, n, trans_type, pers_type, is_last_version )                                 \
  ARATPCNV_PLUGINSVC_FACTORY_WITH_ID( x, std::string( #n ), trans_type, pers_type, is_last_version, ITPCnvBase*() )

#define DECLARE_NAMED_TRIGTPCNV_FACTORY( x, n, trans_type, pers_type, is_last_version )                                \
  TRIGTPCNV_PLUGINSVC_FACTORY_WITH_ID( x, std::string( #n ), trans_type, pers_type, is_last_version, ITPCnvBase*() )

namespace AthenaServicesTestConverters
{
  class TestConverterBase : public ITPCnvBase
  {
  };

  class TestConverter_TA_PA1 : public TestConverterBase
  {
  };
  class TestConverter_TA_PA2 : public TestConverterBase
  {
  };

  class TestConverter_TB_PB1 : public TestConverterBase
  {
  };
  class TestConverter_TB_PB1_ARA : public TestConverterBase
  {
  };
  class TestConverter_TBTRIG_PB1 : public TestConverterBase
  {
  };

} // namespace AthenaServicesTestConverters

DECLARE_TPCNV_FACTORY( AthenaServicesTestConverters::TestConverter_TA_PA1, AthenaServicesTestConverters::TA,
                       AthenaServicesTestConverters::PA1, Athena::TPCnvVers::Old )
DECLARE_TPCNV_FACTORY( AthenaServicesTestConverters::TestConverter_TA_PA2, AthenaServicesTestConverters::TA,
                       AthenaServicesTestConverters::PA2, Athena::TPCnvVers::Current )

DECLARE_TPCNV_FACTORY( AthenaServicesTestConverters::TestConverter_TB_PB1, AthenaServicesTestConverters::TB,
                       AthenaServicesTestConverters::PB1, Athena::TPCnvVers::Current )
DECLARE_ARATPCNV_FACTORY( AthenaServicesTestConverters::TestConverter_TB_PB1_ARA, AthenaServicesTestConverters::TB,
                          AthenaServicesTestConverters::PB1, Athena::TPCnvVers::Current )
DECLARE_TRIGTPCNV_FACTORY( AthenaServicesTestConverters::TestConverter_TBTRIG_PB1, AthenaServicesTestConverters::TBTRIG,
                           AthenaServicesTestConverters::PB1, Athena::TPCnvVers::Current )
