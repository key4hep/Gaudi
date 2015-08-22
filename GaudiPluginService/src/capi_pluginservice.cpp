#include <vector>
#include <algorithm>
#include "capi_pluginservice.h"
#include <Gaudi/PluginService.h>

#include <iostream>
using namespace Gaudi::PluginService::Details;

cgaudi_pluginsvc_t
cgaudi_pluginsvc_instance()
{
  static Registry& cxxreg = Registry::instance();
  return {  (void*)(&cxxreg) };
}

int
cgaudi_pluginsvc_get_factory_size(cgaudi_pluginsvc_t self)
{
  const Registry::FactoryMap &fmap = ((const Registry*)self.registry)->factories();
  return int(fmap.size());
}

constexpr struct select1st_t {
    template <typename S,typename T>
    const S& operator()(const std::pair<S,T>& p) const 
    { return p.first; }
    template <typename S,typename T>
    S& operator()(std::pair<S,T>& p) const 
    { return p.first; }
} select1st {};

cgaudi_factory_t
cgaudi_pluginsvc_get_factory_at(cgaudi_pluginsvc_t self, int n)
{
  const Registry *reg = ((const Registry*)self.registry);
  std::vector<Registry::KeyType> keys;
  keys.reserve(reg->factories().size());
  std::transform(reg->factories().begin(),reg->factories().end(),
                 std::back_inserter(keys),
                 select1st );
  return { self, keys[n].c_str() };
}

const char*
cgaudi_factory_get_library(cgaudi_factory_t self)
{
  Registry &reg = Registry::instance();
  return reg.getInfo(self.id).library.c_str();
}

const char*
cgaudi_factory_get_type(cgaudi_factory_t self)
{
  Registry &reg = Registry::instance();
  return reg.getInfo(self.id).type.c_str();
}

const char*
cgaudi_factory_get_rtype(cgaudi_factory_t self)
{
  Registry &reg = Registry::instance();
  return reg.getInfo(self.id).rtype.c_str();
}

const char*
cgaudi_factory_get_classname(cgaudi_factory_t self)
{
  Registry &reg = Registry::instance();
  return reg.getInfo(self.id).className.c_str();
}

int
cgaudi_factory_get_property_size(cgaudi_factory_t self)
{
  Registry &reg = Registry::instance();
  const Registry::FactoryInfo& fi = reg.getInfo(self.id);
  return int(fi.properties.size());
}


cgaudi_property_t
cgaudi_factory_get_property_at(cgaudi_factory_t self, int n)
{
  cgaudi_property_t cprop{ self.registry, self.id, nullptr };
  Registry &reg = Registry::instance();
  const Registry::FactoryInfo& fi = reg.getInfo(cprop.id);
  if (n<(long)fi.properties.size()) {
    auto itr = std::next(fi.properties.cbegin(),n);
    cprop.key = itr->first.c_str();
  }
  return cprop;
}

const char*
cgaudi_property_get_key(cgaudi_property_t self)
{
  return self.key;
}

const char*
cgaudi_property_get_value(cgaudi_property_t self)
{
  Registry &reg = Registry::instance();
  const Registry::FactoryInfo& fi = reg.getInfo(self.id);
  auto prop = fi.properties.find(self.key);
  return  prop != fi.properties.end() ? prop->second.c_str() : nullptr;
}
