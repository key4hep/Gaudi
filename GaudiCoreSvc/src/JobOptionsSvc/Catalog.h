// $Id:$
#ifndef JOBOPTIONSVC_CATALOG_H_
#define JOBOPTIONSVC_CATALOG_H_
// ============================================================================
// STD & STL:
// ============================================================================
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
// ============================================================================
// Boost:
// ============================================================================
#include <boost/ptr_container/ptr_set.hpp>
// ============================================================================
// Local:
// ============================================================================
#include "Property.h"
#include "PropertyName.h"
#include "PropertyValue.h"
// ============================================================================
// Namespace aliases:
// ============================================================================
// ...
// ============================================================================
namespace Gaudi { namespace Parsers {
// ============================================================================
class Catalog {
 public:
  typedef boost::ptr_set<Property, Property::LessThen> PropertySet;
  typedef std::map<std::string,  PropertySet>  CatalogSet;

  typedef CatalogSet::value_type  value_type;
  typedef CatalogSet::iterator  iterator;
  typedef CatalogSet::const_iterator  const_iterator;

  iterator begin() { return catalog_.begin();}
  const_iterator begin() const { return catalog_.begin();}
  iterator end() { return catalog_.end();}
  const_iterator end() const{ return catalog_.end();}

  std::vector<std::string> ClientNames() const;
  bool Add(Property* property);
  template<typename Value> bool Add(const std::string& client,
          const std::string& property, const Value& value);
  Property* Find(const std::string& client, const std::string& name);
  std::string ToString() const;
  /// print the content of the catalogue to std::ostream
  std::ostream& fillStream ( std::ostream& out ) const ;
 private:
  CatalogSet catalog_;
};
// ============================================================================
/// printout operator
// ============================================================================
 inline std::ostream& operator<< (std::ostream& o, const Catalog& c){
   return c.fillStream(o);
 }
// ============================================================================
}  /* Gaudi */ }  /* Parsers */

template<typename Value> inline bool Gaudi::Parsers::Catalog::Add(
        const std::string& client, const std::string& property,
        const Value& value) {
  return Add(new Property(PropertyName(client, property), PropertyValue(value)));
}

// ============================================================================
#endif  // JOBOPTIONSVC_CATALOG_H_
