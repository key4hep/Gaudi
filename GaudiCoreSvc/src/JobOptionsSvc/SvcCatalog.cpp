// $Id:$
// ============================================================================
// CVS tag $Name:  $
// ============================================================================
// Include files
// ===========================================================================
// STD & STL:
// ===========================================================================
#include <iostream>
#include <sstream>
// ===========================================================================
// Boost:
// ===========================================================================
#include "boost/algorithm/string.hpp"
// ===========================================================================
// Local
// ===========================================================================
#include "SvcCatalog.h"
// ===========================================================================
SvcCatalog::SvcCatalog()
{ m_catalog = new ObjectsT(); }
// ===========================================================================
SvcCatalog::~SvcCatalog()
{
  for (ObjectsT::const_iterator cur=m_catalog->begin();
       cur!=m_catalog->end();cur++)
  {
    for(PropertiesT::const_iterator prop=cur->second.begin();
        prop!=cur->second.end(); prop++)
    { if(NULL != *prop){ delete *prop; } }
  }
  delete m_catalog;
}
// ============================================================================
StatusCode SvcCatalog::addProperty
(const std::string& client,
 const Property* property )
{
  PropertiesT*  props = findProperties(client);
  if ( props != 0 ){
    removeProperty(client,property->name()).ignore();
    props->push_back(property);
  }else{
    PropertiesT toInsert;
    toInsert.push_back(property);
    m_catalog->insert(std::pair<std::string,PropertiesT>(client,toInsert));
  }
  return StatusCode::SUCCESS;
}
// ============================================================================
StatusCode
SvcCatalog::removeProperty
( const std::string& client,
  const std::string& name)
{
  PropertiesT*  props = findProperties(client);
  if (props)
  {
    PropertiesT::iterator toRemove;
    if(findProperty(props,name,toRemove))
    {
      delete *toRemove;
      props->erase(toRemove);
    }
  }
  return StatusCode::SUCCESS;
}
// ============================================================================
const SvcCatalog::PropertiesT*
SvcCatalog::getProperties
( const std::string& client) const { return findProperties(client); }
// ============================================================================
std::vector<std::string> SvcCatalog::getClients() const
{
  std::vector<std::string> result;
  for (ObjectsT::const_iterator cur = m_catalog->begin();
       cur != m_catalog->end(); cur++) { result.push_back(cur->first); }
  return result;
}
// ============================================================================
SvcCatalog::PropertiesT*
SvcCatalog::findProperties( const std::string& client) const
{
  ObjectsT::iterator result;
  if((result = m_catalog->find(client)) == m_catalog->end()){ return 0; }
  return &result->second;
}
// ============================================================================
bool SvcCatalog::findProperty
( SvcCatalog::PropertiesT* props ,
  const std::string&                name  ,
  SvcCatalog::PropertiesT::iterator& result)
{
  for(result = props->begin();result!=props->end();result++){
    if(boost::to_lower_copy((*result)->name()) == boost::to_lower_copy(name))
    { return true; }
  }
  return false;
}
// ============================================================================
std::ostream& SvcCatalog::fillStream( std::ostream& o ) const
{
  // loop over the clients:
  for ( ObjectsT::const_iterator iclient = m_catalog->begin();
        m_catalog->end() != iclient ; ++iclient )
  {
    const PropertiesT& props  = iclient->second ;
    o << "Client '" << iclient->first << "'" << std::endl ;
    for ( PropertiesT::const_iterator ip = props.begin() ;
          props.end() != ip ; ++ip )
    {
      const Property* p = *ip ;
      if ( 0 == p ) { continue ; }                             // CONTINUE
      o << "\t" << (*p) << std::endl ;
    }
  }
  //
  return o ;                                                   // RETURN
}
// ============================================================================
// printoput operator
// ============================================================================
std::ostream& operator<<( std::ostream& o , const SvcCatalog& c )
{ return c.fillStream ( o ) ; }
// ============================================================================
// The END
// ============================================================================
