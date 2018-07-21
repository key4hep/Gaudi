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

#define GAUDI_PLUGIN_SERVICE_V1

#include <Gaudi/PluginService.h>

// standard use, 0 arguments
class Base
{
public:
  typedef Gaudi::PluginService::Factory<Base*> Factory;
  virtual ~Base() {}
};
class Component0 : public Base
{
};
DECLARE_COMPONENT( Component0 )

class Component1 : public Base
{
};
#define DECLARE_COMPONENT_WITH_PROPS( type ) DECLARE_FACTORY_WITH_PROPS( type, type::Factory )
#define DECLARE_FACTORY_WITH_PROPS( type, factory )                                                                    \
  DECLARE_FACTORY_WITH_ID_AND_PROPS( type, ::Gaudi::PluginService::Details::demangle<type>(), factory )
#define DECLARE_FACTORY_WITH_ID_AND_PROPS( type, id, factory )                                                         \
  _INTERNAL_DECLARE_FACTORY_WITH_PROPS( type, id, factory, __LINE__ )
#define _INTERNAL_DECLARE_FACTORY_WITH_PROPS( type, id, factory, serial )                                              \
  _INTERNAL_DECLARE_FACTORY_WITH_CREATOR_AND_PROPS( type, ::Gaudi::PluginService::Details::Factory<type>, id, factory, \
                                                    serial )
#define _INTERNAL_DECLARE_FACTORY_WITH_CREATOR_AND_PROPS( type, typecreator, id, factory, serial )                     \
  namespace                                                                                                            \
  {                                                                                                                    \
    class _PS_V1_INTERNAL_FACTORY_REGISTER_CNAME( type, serial )                                                       \
    {                                                                                                                  \
    public:                                                                                                            \
      typedef factory      s_t;                                                                                        \
      typedef typecreator  f_t;                                                                                        \
      static s_t::FuncType creator() { return &f_t::create<s_t>; }                                                     \
      _PS_V1_INTERNAL_FACTORY_REGISTER_CNAME( type, serial )()                                                         \
      {                                                                                                                \
        using ::Gaudi::PluginService::Details::Registry;                                                               \
        Registry::instance().add<s_t, type>( id, creator() ).addProperty( "name", #type );                             \
      }                                                                                                                \
    } _PS_V1_INTERNAL_FACTORY_REGISTER_CNAME( s_##type, serial );                                                      \
  }

DECLARE_COMPONENT_WITH_PROPS( Component1 )

// standard use, 2 arguments
class Base2
{
public:
  typedef Gaudi::PluginService::Factory<Base2*, const std::string&, int> Factory;
  virtual ~Base2() {}
};
class Component2 : public Base2
{
public:
  Component2( std::string _s, int _i ) : i( _i ), s( std::move( _s ) ) {}
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
}

namespace
{
  using Test::ComponentA;
  DECLARE_COMPONENT( ComponentA )
}

DECLARE_COMPONENT( Test::ComponentB )

namespace Test
{
  DECLARE_COMPONENT( ComponentC )
}

// using ids
DECLARE_COMPONENT_WITH_ID( Component2, "Id2" )
DECLARE_COMPONENT_WITH_ID( Test::ComponentB, "B" )

// explicit factory
DECLARE_FACTORY_WITH_ID( Test::ComponentA, "A", Base::Factory )

// custom factory example
namespace
{
  bool _custom_factory_called = false;

  struct CompWithCustomFactory : Base {
  };

  class _register__CompWithCustomFactory
  {
  public:
    typedef Base::Factory s_t;
    static Base*          creator()
    {
      _custom_factory_called = true;
      return new CompWithCustomFactory{};
    }
    _register__CompWithCustomFactory()
    {
      using ::Gaudi::PluginService::Details::Registry;
      Registry::instance().add<s_t, CompWithCustomFactory>( "CompWithCustomFactory", creator );
    }
  } _register__CompWithCustomFactory;
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
  typedef Gaudi::PluginService::Factory<ITPCnvBase*> Factory;
};

#define DO_ATHTPCNV_FACTORY_REGISTER_CNAME( name, serial ) _register_##_##serial

#define DO_ATHTPCNV_PLUGINSVC_FACTORY_WITH_ID( type, id, trans_type, pers_type, is_last_version, cnv_type, signature,  \
                                               serial )                                                                \
  namespace                                                                                                            \
  {                                                                                                                    \
    class DO_ATHTPCNV_FACTORY_REGISTER_CNAME( type, serial )                                                           \
    {                                                                                                                  \
    public:                                                                                                            \
      typedef type::Factory                                  s_t;                                                      \
      typedef ::Gaudi::PluginService::Details::Factory<type> f_t;                                                      \
      static s_t::FuncType                                   creator() { return &f_t::create<s_t>; }                   \
      DO_ATHTPCNV_FACTORY_REGISTER_CNAME( type, serial )()                                                             \
      {                                                                                                                \
        using ::Gaudi::PluginService::Details::Registry;                                                               \
        std::string prefix;                                                                                            \
        if ( cnv_type == Athena::TPCnvType::ARA )                                                                      \
          prefix = "_ARA";                                                                                             \
        else if ( cnv_type == Athena::TPCnvType::Trigger )                                                             \
          prefix = "_TRIG";                                                                                            \
        Registry::instance().add<s_t, type>( id, creator() );                                                          \
        if ( is_last_version == Athena::TPCnvVers::Current )                                                           \
          Registry::instance().add<s_t, type>( prefix + "_TRANS_" + #trans_type, creator() );                          \
        Registry::instance().add<s_t, type>( prefix + "_PERS_" + #pers_type, creator() );                              \
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
