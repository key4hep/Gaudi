/***********************************************************************************\
* (c) Copyright 2013-2022 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef _GAUDI_PLUGIN_SERVICE_V2_H_
#define _GAUDI_PLUGIN_SERVICE_V2_H_

/// @author Marco Clemencic <marco.clemencic@cern.ch>
/// See @ref GaudiPluginService-readme

#include <Gaudi/Details/PluginServiceDetailsV2.h>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

#if __cplusplus > 201703L && __has_include( <source_location> )
#  include <source_location>
#  include <sstream>
#  include <string_view>
#endif

namespace Gaudi {
  /// See @ref GaudiPluginService-readme
  namespace PluginService {
    GAUDI_PLUGIN_SERVICE_V2_INLINE namespace v2 {
#if __cplusplus > 201703L && __has_include( <source_location> )
      using std::source_location;
#else
      // dummy implementation for backward compatibility
      struct source_location {
        static source_location current() { return {}; }
      };
#endif

      /// \cond FWD_DECL
      template <typename>
      struct Factory;
      /// \endcond

      /// Class wrapping the signature for a factory with any number of arguments.
      template <typename R, typename... Args>
      struct Factory<R( Args... )> {
        using Traits          = Details::Traits<R( Args... )>;
        using ReturnType      = typename Traits::ReturnType;
        using FactoryType     = typename Traits::FactoryType;
        using ReturnValueType = R;

        /// Function to call to create an instance of type identified by `id` and that uses this factory signature.
        template <typename T>
        static ReturnType create( const T& id, Args... args ) {
          try {
            return Details::Registry::instance().get<FactoryType>( Details::stringify_id( id ) )(
                std::forward<Args>( args )... );
          } catch ( std::bad_any_cast& ) {
            Details::reportBadAnyCast( typeid( FactoryType ), Details::stringify_id( id ) );
            return nullptr;
          }
        }
      };

      /// Helper to declare the factory implementation for a user defined type `T`.
      ///
      /// The basic use is:
      /// ```cpp
      /// namespace {
      ///   Gaudi::PluginService::DeclareFactory<MyComponent> __some_random_name;
      /// }
      /// ```
      /// which is the equivalent of `DECLARE_COMPONENT( MyComponent )`.
      ///
      /// It's possible to specify a custom factory type (instead of the default type alias `MyComponent::Factory`):
      /// ```cpp
      /// namespace {
      ///   using namespace Gaudi::PluginService;
      ///   DeclareFactory<MyComponent, Factory<MyBase*( int, int )>> __some_random_name;
      /// }
      /// ```
      ///
      /// We can pass arguments to the constructor to use a custom factory function, or a special _id_, or properties:
      /// ```cpp
      /// namespace {
      ///   using namespace Gaudi::PluginService;
      ///   DeclareFactory<MyComponent> __some_random_name( "special-id",
      ///                                                   []() -> MyComponent::Factory::ReturnType {
      ///                                                     return std::make_unique<MyComponent>( "special-id" );
      ///                                                   },
      ///                                                   {{"MyProperty", "special"}} );
      /// }
      /// ```
      template <typename T, typename F = typename T::Factory>
      struct DeclareFactory {
        using DefaultFactory = Details::DefaultFactory<T, F>;

        DeclareFactory( typename F::FactoryType f = DefaultFactory{}, Details::Registry::Properties props = {},
                        source_location src_loc = source_location::current() )
            : DeclareFactory( Details::demangle<T>(), std::move( f ), std::move( props ), src_loc ) {}

        DeclareFactory( const std::string& id, typename F::FactoryType f = DefaultFactory{},
                        Details::Registry::Properties    props   = {},
                        [[maybe_unused]] source_location src_loc = source_location::current() ) {
          using Details::Registry;

          if ( props.find( "ClassName" ) == end( props ) ) props.emplace( "ClassName", Details::demangle<T>() );
#if __cplusplus > 201703L && __has_include( <source_location> )
          // keep only the file name
          std::string_view fn  = src_loc.file_name();
          auto             pos = fn.rfind( '/' );
          if ( pos != std::string_view::npos ) { fn.remove_prefix( pos + 1 ); }
          std::stringstream s;
          s << fn << ':' << src_loc.line();
          props["declaration_location"] = s.str();
#endif
          Registry::instance().add( id, { libraryName(), std::move( f ), std::move( props ) } );
        }

        DeclareFactory( Details::Registry::Properties props, source_location src_loc = source_location::current() )
            : DeclareFactory( DefaultFactory{}, std::move( props ), src_loc ) {}

      private:
        /// Helper to record the name of the library that declare the factory.
        static std::string libraryName() { return Details::getDSONameFor( reinterpret_cast<void*>( libraryName ) ); }
      };
    }
  } // namespace PluginService
} // namespace Gaudi

#define _PS_V2_DECLARE_COMPONENT( type )                                                                               \
  namespace {                                                                                                          \
    ::Gaudi::PluginService::v2::DeclareFactory<type> _PS_V2_INTERNAL_FACTORY_REGISTER_CNAME{};                         \
  }

#define _PS_V2_DECLARE_COMPONENT_WITH_ID( type, id )                                                                   \
  namespace {                                                                                                          \
    ::Gaudi::PluginService::v2::DeclareFactory<type> _PS_V2_INTERNAL_FACTORY_REGISTER_CNAME{                           \
        ::Gaudi::PluginService::v2::Details::stringify_id( id ) };                                                     \
  }

#define _PS_V2_DECLARE_FACTORY( type, factory )                                                                        \
  namespace {                                                                                                          \
    ::Gaudi::PluginService::v2::DeclareFactory<type, factory> _PS_V2_INTERNAL_FACTORY_REGISTER_CNAME{};                \
  }

#define _PS_V2_DECLARE_FACTORY_WITH_ID( type, id, factory )                                                            \
  namespace {                                                                                                          \
    ::Gaudi::PluginService::v2::DeclareFactory<type, factory> _PS_V2_INTERNAL_FACTORY_REGISTER_CNAME{                  \
        ::Gaudi::PluginService::v2::Details::stringify_id( id ) };                                                     \
  }

#if GAUDI_PLUGIN_SERVICE_USE_V2
#  define DECLARE_COMPONENT( type ) _PS_V2_DECLARE_COMPONENT( type )
#  define DECLARE_COMPONENT_WITH_ID( type, id ) _PS_V2_DECLARE_COMPONENT_WITH_ID( type, id )
#  define DECLARE_FACTORY( type, factory ) _PS_V2_DECLARE_FACTORY( type, factory )
#  define DECLARE_FACTORY_WITH_ID( type, id, factory ) _PS_V2_DECLARE_FACTORY_WITH_ID( type, id, factory )
#endif

#endif //_GAUDI_PLUGIN_SERVICE_H_
