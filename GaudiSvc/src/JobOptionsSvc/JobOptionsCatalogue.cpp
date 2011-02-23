// $Id: JobOptionsCatalogue.cpp,v 1.13 2007/05/25 11:27:59 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $ 
// ============================================================================
// Include files
// ===========================================================================
// STL&STL
// ===========================================================================
#include <iostream>
// ===========================================================================
// Boost.string_algo
// ===========================================================================
#include "boost/algorithm/string.hpp"
// ===========================================================================
// Local
// ===========================================================================
#include "JobOptionsCatalogue.h"
#include <sstream>
#include "GaudiKernel/Parsers.h"
// ===========================================================================
JobOptionsCatalogue::JobOptionsCatalogue()
{ m_catalogue = new ObjectsT(); }
// ===========================================================================
JobOptionsCatalogue::~JobOptionsCatalogue()
{
  for (ObjectsT::const_iterator cur=m_catalogue->begin();
       cur!=m_catalogue->end();cur++)
  {
    for(PropertiesT::const_iterator prop=cur->second.begin();
        prop!=cur->second.end(); prop++)
    { if(NULL != *prop){ delete *prop; } }
  }
  delete m_catalogue;
}
// ============================================================================
StatusCode JobOptionsCatalogue::addProperty
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
    m_catalogue->insert(std::pair<std::string,PropertiesT>(client,toInsert));  
  }
  return StatusCode::SUCCESS;
}
// ============================================================================
StatusCode 
JobOptionsCatalogue::removeProperty
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
const JobOptionsCatalogue::PropertiesT* 
JobOptionsCatalogue::getProperties
( const std::string& client) const { return findProperties(client); }
// ============================================================================
std::vector<std::string> JobOptionsCatalogue::getClients() const
{
  std::vector<std::string> result;
  for (ObjectsT::const_iterator cur = m_catalogue->begin(); 
       cur != m_catalogue->end(); cur++) { result.push_back(cur->first); }
  return result;
}
// ============================================================================
JobOptionsCatalogue::PropertiesT* 
JobOptionsCatalogue::findProperties( const std::string& client) const
{
  ObjectsT::iterator result;       
  if((result = m_catalogue->find(client))==m_catalogue->end()){ return 0; }
  return &result->second;
}
// ============================================================================
bool JobOptionsCatalogue::findProperty
( JobOptionsCatalogue::PropertiesT* props ,
  const std::string&                name  , 
  JobOptionsCatalogue::PropertiesT::iterator& result)
{
  for(result = props->begin();result!=props->end();result++){
    if(boost::to_lower_copy((*result)->name()) == boost::to_lower_copy(name))
    { return true; }
  } 
  return false;
}
// ============================================================================
std::ostream& JobOptionsCatalogue::fillStream( std::ostream& o ) const 
{
  // loop over the clients:
  for ( ObjectsT::const_iterator iclient = m_catalogue->begin(); 
        m_catalogue->end() != iclient ; ++iclient ) 
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
std::ostream& operator<<( std::ostream& o , const JobOptionsCatalogue& c ) 
{ return c.fillStream ( o ) ; }
// ============================================================================
// The END 
// ============================================================================
