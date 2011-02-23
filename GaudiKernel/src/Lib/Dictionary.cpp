#include "GaudiKernel/System.h"
namespace GaudiDict {
  std::string typeName(const std::type_info& typ);
  std::string vectorName(const std::type_info& typ);
  std::string vectorName(const std::string& typ);
  std::string pairName(const std::type_info& typ1, const std::type_info& typ2);
  std::string relationName(const std::string& prefix, const std::type_info& typ1, const std::type_info& typ2);
  std::string templateName1(const std::string& templ, const std::type_info& typ);
  std::string templateName1(const std::string& templ, const std::string& typ);
  std::string templateName2(const std::string& templ, const std::type_info& typ1, const std::type_info& typ2);
  std::string templateName3(const std::string& templ, const std::type_info& typ1, const std::type_info& typ2, const std::type_info& typ3);
  std::string keyedContainerName(const std::string& prefix, const std::type_info& typ1);
}

static std::string clean(const std::string& s)  {
  std::string c = s;
  for(size_t occ=c.find(" *"); occ != std::string::npos; occ=c.find(" *"))
    c.replace(occ,2,"*");
  return c;
}

std::string GaudiDict::typeName(const std::type_info& typ)  {
  std::string r = clean(System::typeinfoName(typ));
  //if ( r.substr(0,4) == "enum" )  {
  //  r = "int";
  //}
  return r;
}

std::string GaudiDict::templateName1(const std::string& templ, const std::type_info& typ)  {
  return templateName1(templ, typeName(typ));
}

std::string GaudiDict::templateName1(const std::string& templ, const std::string& typ)  {
  std::string s = templ + "<";
  s += typ;
  s += (s[s.length()-1] == '>') ? " >" : ">";
  return clean(s);
}

std::string GaudiDict::templateName2(const std::string& templ, const std::type_info& typ1, const std::type_info& typ2)  {
  std::string s = templ + "<";
  s += typeName(typ1);
  s += ",";
  s += typeName(typ2);
  s += (s[s.length()-1] == '>') ? " >" : ">";
  return clean(s);
}

std::string GaudiDict::templateName3(const std::string& templ, const std::type_info& typ1, const std::type_info& typ2, const std::type_info& typ3)  {
  std::string s = templ + "<";
  s += typeName(typ1);
  s += ",";
  s += typeName(typ2);
  s += ",";
  s += typeName(typ3);
  s += (s[s.length()-1] == '>') ? " >" : ">";
  return clean(s);
}

std::string GaudiDict::vectorName(const std::type_info& typ)   {
  return templateName1("std::vector", typ);
}

std::string GaudiDict::vectorName(const std::string& typ)   {
  return templateName1("std::vector", typ);
}

std::string GaudiDict::pairName(const std::type_info& typ1, const std::type_info& typ2)  {
  return templateName2("std::pair", typ1, typ2);
}

std::string GaudiDict::keyedContainerName(const std::string& mgr_typ, const std::type_info& typ1)   {
  std::string s = "KeyedContainer<";
  s += typeName(typ1);
  s += ",";
  s += mgr_typ;
  if ( mgr_typ[mgr_typ.length()-1]=='>' ) s += " ";
  s += ">";
  return clean(s);
}

std::string GaudiDict::relationName(const std::string& /* prefix */, const std::type_info& /* typ1 */, const std::type_info& /* typ2 */)   {
  return "";
}
