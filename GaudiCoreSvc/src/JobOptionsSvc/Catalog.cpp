// ============================================================================
#include "Catalog.h"
// ============================================================================
// Boost:
// ============================================================================
#include <boost/foreach.hpp>
#include <boost/format.hpp>
// ============================================================================
// Namesapce aliases:
// ============================================================================
namespace gp = Gaudi::Parsers;
// ============================================================================
std::vector<std::string> gp::Catalog::ClientNames() const {
    std::vector<std::string> result;
    BOOST_FOREACH(const CatalogSet::value_type& prop, catalog_) {
        result.push_back(prop.first);
    }
    return result;
}
// ============================================================================
bool gp::Catalog::Add(Property* property) {
  assert( property != NULL);
  CatalogSet::iterator it = catalog_.find(property->ClientName());
  if (it == catalog_.end()) {
    CatalogSet::mapped_type properties;
    properties.insert(property);
    catalog_.insert(CatalogSet::value_type(property->ClientName(), properties));
    return true;
  }
  it->second.erase(*property);
  it->second.insert(property);
  //TODO: check return value
  return true;
}
// ============================================================================
gp::Property* gp::Catalog::Find(const std::string& client,
    const std::string& name) {
  CatalogSet::iterator it = catalog_.find(client);
  if (it == catalog_.end()) return NULL;

  CatalogSet::mapped_type::iterator pit = std::find_if(it->second.begin(),
      it->second.end(), Property::Equal(name));
  if (pit == it->second.end()) return NULL;
  return &*pit;

}
// ============================================================================
std::string gp::Catalog::ToString() const {
  std::string result;
  BOOST_FOREACH(const CatalogSet::value_type& client, catalog_) {
    for (CatalogSet::mapped_type::const_iterator current = client.second.begin();
        current != client.second.end(); ++current) {
      result += current->ToString()+"\n";
    }
  }
  return result;
}
// ============================================================================
// print the content of the catalogue to std::ostream
// ============================================================================
std::ostream& Gaudi::Parsers::Catalog::fillStream ( std::ostream& o ) const
{
  o << "// " << std::string(82,'=') << std::endl
    << "//       Parser catalog " << std::endl
    << "// " << std::string(82,'=') << std::endl ;

  size_t nComponents = 0 ;
  size_t nProperties = 0 ;

  BOOST_FOREACH(const CatalogSet::value_type& client, catalog_)   {
    o << boost::format("// Properties of '%1%' %|43t|# = %2%" )
        % client.first % client.second.size() << std::endl ;
        ++nComponents ;
        nProperties += client.second.size() ;
   for (CatalogSet::mapped_type::const_iterator current = client.second.begin();
       current != client.second.end(); ++current) {
     o << boost::format("%1%   %|44t| = %2% ; ")
           % current->FullName()
           % current->ValueAsString()
       << std::endl;
   }
  }
  o << "// " << std::string(82,'=') << std::endl
      << boost::format("// End parser catalog #Components=%1% #Properties=%2%")
  % nComponents % nProperties     << std::endl
  << "// " << std::string(82,'=') << std::endl ;
  return o ;
}
