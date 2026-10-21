/***********************************************************************************\
* (c) Copyright 2013-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

/* @author Sebastien Binet <binet@cern.ch> */

#define GAUDI_PLUGIN_SERVICE_V2
#include <Gaudi/PluginService.h>

#include <stdint.h>

#include <sstream>
#include <string>
#include <vector>

#define CGAUDI_HASCLASSVISIBILITY

#if defined( CGAUDI_HASCLASSVISIBILITY )
#  define CGAUDI_IMPORT __attribute__( ( visibility( "default" ) ) )
#  define CGAUDI_EXPORT __attribute__( ( visibility( "default" ) ) )
#  define CGAUDI_LOCAL __attribute__( ( visibility( "hidden" ) ) )
#else
#  define CGAUDI_IMPORT
#  define CGAUDI_EXPORT
#  define CGAUDI_LOCAL
#endif

#define CGAUDI_API CGAUDI_EXPORT

using namespace Gaudi::PluginService::v2::Details;

extern "C" {

typedef struct {
  void* registry;
} cgaudi_pluginsvc_t;

typedef struct {
  cgaudi_pluginsvc_t registry;
  const char*        id;
} cgaudi_factory_t;

typedef struct {
  cgaudi_pluginsvc_t registry;
  const char*        id;
  const char*        key;
} cgaudi_property_t;

CGAUDI_API
cgaudi_pluginsvc_t cgaudi_pluginsvc_instance( void ) {
  static Registry& cxxreg = Registry::instance();
  return { &cxxreg };
}

CGAUDI_API
int cgaudi_pluginsvc_get_factory_size( cgaudi_pluginsvc_t self ) {
  const Registry::FactoryMap& fmap = static_cast<const Registry*>( self.registry )->factories();
  return int( fmap.size() );
}

CGAUDI_API
cgaudi_factory_t cgaudi_pluginsvc_get_factory_at( cgaudi_pluginsvc_t self, int n ) {
  const Registry* reg       = static_cast<const Registry*>( self.registry );
  const auto&     factories = reg->factories();
  if ( n >= static_cast<int>( factories.size() ) ) return { self, nullptr };
  return { self, next( begin( factories ), n )->first.c_str() };
}

CGAUDI_API
const char* cgaudi_factory_get_library( cgaudi_factory_t self ) {
  Registry& reg = Registry::instance();
  return reg.getInfo( self.id ).library.c_str();
}

CGAUDI_API
const char* cgaudi_factory_get_type( cgaudi_factory_t self ) {
  Registry&          reg = Registry::instance();
  static std::string cache;
  cache = demangle( reg.getInfo( self.id ).factory.type() );
  return cache.c_str();
}

CGAUDI_API
const char* cgaudi_factory_get_classname( cgaudi_factory_t self ) {
  Registry&          reg = Registry::instance();
  static std::string cache;
  cache = reg.getInfo( self.id ).getprop( "ClassName" );
  return cache.c_str();
}

CGAUDI_API
int cgaudi_factory_get_property_size( cgaudi_factory_t self ) {
  Registry&                    reg = Registry::instance();
  const Registry::FactoryInfo& fi  = reg.getInfo( self.id );
  return int( fi.properties.size() );
}

CGAUDI_API
cgaudi_property_t cgaudi_factory_get_property_at( cgaudi_factory_t self, int n ) {
  cgaudi_property_t            cprop{ self.registry, self.id, nullptr };
  Registry&                    reg = Registry::instance();
  const Registry::FactoryInfo& fi  = reg.getInfo( cprop.id );
  if ( n < static_cast<int>( fi.properties.size() ) ) cprop.key = next( begin( fi.properties ), n )->first.c_str();
  return cprop;
}

CGAUDI_API
const char* cgaudi_property_get_key( cgaudi_property_t self ) { return self.key; }

CGAUDI_API
const char* cgaudi_property_get_value( cgaudi_property_t self ) {
  Registry&                    reg  = Registry::instance();
  const Registry::FactoryInfo& fi   = reg.getInfo( self.id );
  auto                         prop = fi.properties.find( self.key );
  return prop != fi.properties.end() ? prop->second.c_str() : nullptr;
}

CGAUDI_API
const char* cgaudi_pluginsvc_plugin_search_path() {
  static std::string cache = [] {
    std::ostringstream oss;
    const auto&        paths = Gaudi::PluginService::Details::getPluginSearchPath();
    for ( auto it = paths.begin(); it != paths.end(); ++it ) {
      if ( it != paths.begin() ) {
#ifdef _WIN32
        oss << ';';
#else
        oss << ':';
#endif
      }
      oss << *it;
    }
    return oss.str();
  }();
  return cache.c_str();
}

} /* extern "C" */
