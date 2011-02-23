// $Id: PropertyMgr.cpp,v 1.23 2008/04/03 17:27:01 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.23 $
// ============================================================================
// Include files
// ============================================================================
// STD& STL
// ============================================================================
#include <iostream>
#include <sstream>
#include <string>
#include <functional>
#include <stdexcept>
#include <algorithm>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/PropertyMgr.h"
// ============================================================================
namespace
{
  // ==========================================================================
  /// case insensitive comparison of strings
  struct Nocase : public std::binary_function<std::string,std::string,bool>
  {
    inline bool operator() ( const std::string& v1 ,
                             const std::string& v2 ) const
    {
      std::string::const_iterator i1 = v1.begin() ;
      std::string::const_iterator i2 = v2.begin() ;
      for ( ; v1.end() != i1 && v2.end() != i2 ; ++i1 , ++i2 )
      { if ( toupper(*i1) != toupper(*i2) ) { return false ; } }
      return v1.size() == v2.size() ;
    }
  };
  // ==========================================================================
  /// get the property by name
  struct PropByName : public std::unary_function<const Property*,bool>
  {
    PropByName ( const std::string& name ) : m_name ( name ) {} ;
    inline bool operator() ( const Property* p ) const
    { return ( 0 == p ) ? false : m_cmp ( p->name() , m_name ) ; }
  private:
    std::string m_name ;
    Nocase      m_cmp  ;
  } ;
  // ==========================================================================
}
// ====================================================================
// constructor from the interface
// ====================================================================
PropertyMgr::PropertyMgr(IInterface* iface)
  : m_properties       ()
  , m_remoteProperties ()
  , m_todelete         ()
  , m_pOuter           ( iface )
{
  addRef(); // initial reference count set to 1
}
// ====================================================================
// copy constructor
// ====================================================================
PropertyMgr::PropertyMgr ( const PropertyMgr& right )
  : IInterface(right),
    IProperty(right),
    extend_interfaces_base(right),
    base_class(right)
  , m_properties       ( right.m_properties       )
  , m_remoteProperties ( right.m_remoteProperties )
  , m_todelete         ( right.m_todelete         )
  , m_pOuter           ( right.m_pOuter )
{
  addRef(); // initial reference count set to 1
  std::transform
    ( m_todelete.begin() , m_todelete.end  () ,
      m_todelete.begin() , std::mem_fun(&Property::clone));
}
// ====================================================================
// destructor
// ====================================================================
PropertyMgr::~PropertyMgr()
{
  /// delete all owned properties
  for ( Properties::iterator ip  = m_todelete.begin() ;
        m_todelete.end() != ip ; ++ip ) { delete *ip ; }
}
// ====================================================================
// assignment operator
// ====================================================================
PropertyMgr& PropertyMgr::operator=( const PropertyMgr& right )
{
  if  ( &right == this ) { return *this ; }
  //
  for ( Properties::iterator ip  = m_todelete.begin() ;
        m_todelete.end() != ip ; ++ip ) { delete *ip ; }
  //
  m_properties       = right.m_properties       ;
  m_remoteProperties = right.m_remoteProperties ;
  m_todelete         = right.m_todelete         ;
  m_pOuter           = right.m_pOuter           ;
  //
  std::transform
    ( m_todelete.begin() , m_todelete.end  () ,
      m_todelete.begin() , std::mem_fun(&Property::clone));
  //
  return *this ;
}
// ====================================================================
// Declare a remote property
// ====================================================================
Property* PropertyMgr::declareRemoteProperty
( const std::string& name  ,
  IProperty*         rsvc  ,
  const std::string& rname )
{
  if ( 0 == rsvc ) { return 0 ; }
  const std::string& nam = rname.empty() ? name : rname ;
  Property* p = property ( nam , rsvc->getProperties() )  ;
  m_remoteProperties.push_back ( RemProperty ( name , std::make_pair ( rsvc , nam ) ) ) ;
  return p ;
}
// ====================================================================
// get the property by name form the proposed list
// ====================================================================
Property* PropertyMgr::property
( const std::string&            name  ,
  const std::vector<Property*>& props ) const
{
  Properties::const_iterator it =
    std::find_if ( props.begin() , props.end() , PropByName( name ) ) ;
  if ( props.end() != it ) { return *it ; }                // RETURN
  return 0 ;                                               // RETURN
}
// ====================================================================
// retrieve the property by name
// ====================================================================
Property* PropertyMgr::property
( const std::string& name  ) const
{
  // local property ?
  Property* lp = property ( name , m_properties ) ;
  if ( 0 != lp ) { return lp ; }                       // RETURN
  // look for remote property
  Nocase cmp ;
  for ( RemoteProperties::const_iterator it = m_remoteProperties.begin() ;
        m_remoteProperties.end() != it ; ++it )
  {
    if ( !cmp(it->first,name) ) { continue ; }   // CONTINUE
    const IProperty* p = it->second.first ;
    if ( 0 == p               ) { continue ; }   // CONITNUE
    return property ( it->second.second , p->getProperties() ) ; // RETURN
  }
  return 0 ;                                           // RETURN
}
// ====================================================================
/* set a property from another property
 *  Implementation of IProperty::setProperty
 */
// =====================================================================
StatusCode PropertyMgr::setProperty( const Property& p )
{
  Property* pp = property( p.name() ) ;
  if ( 0 == pp            ) { return StatusCode::FAILURE ; } // RETURN
  //
  try
  { if ( !pp->assign(p) ) { return StatusCode::FAILURE ; } } // RETURN
  catch(...)              { return StatusCode::FAILURE ;   } // RETURN
  //
  return StatusCode::SUCCESS;      // Property value set
}
// ====================================================================
/* set a property from the stream
 *  Implementation of IProperty::setProperty
 */
// =====================================================================
StatusCode
PropertyMgr::setProperty( const std::string& i )
{
  std::string name  ;
  std::string value ;
  StatusCode sc = Gaudi::Parsers::parse( name , value , i ) ;
  if ( sc.isFailure() ) { return sc ; }
  return setProperty ( name , value ) ;
}
// =====================================================================
/* set a property from the string
 *  Implementation of IProperty::setProperty
 */
// =====================================================================
StatusCode
PropertyMgr::setProperty( const std::string& n, const std::string& v )
{
  Property* p = property( n ) ;
  if ( 0 == p ) { return StatusCode::FAILURE ; }  // RETURN
  bool result =  p->fromString( v ) != 0 ;
  return result ? StatusCode::SUCCESS : StatusCode::FAILURE ;
}
// =====================================================================
/* Retrieve the value of a property
 *  Implementation of IProperty::getProperty
 */
// =====================================================================
StatusCode PropertyMgr::getProperty(Property* p) const
{
  try
  {
    const Property* pp = property( p->name() ) ;
    if ( 0 == pp         ) { return StatusCode::FAILURE ; }   // RETURN
    if ( !pp->load( *p ) ) { return StatusCode::FAILURE ; }   // RETURN
  }
  catch ( ... )            { return StatusCode::FAILURE;  }   // RETURN
  return StatusCode::SUCCESS ;                                // RETURN
}
// =====================================================================
/* Get the property by name
 *  Implementation of IProperty::getProperty
 */
// =====================================================================
const Property& PropertyMgr::getProperty( const std::string& name) const
{
  const Property* p = property( name ) ;
  if ( 0 != p ) { return *p ; }                        // RETURN
  //
  throw std::out_of_range( "Property "+name+" not found." );    // Not found
}
// =====================================================================
/* Get the property by name
 *  Implementation of IProperty::getProperty
 */
// =====================================================================
StatusCode PropertyMgr::getProperty
( const std::string& n ,
  std::string&       v ) const
{
  // get the property
  const Property* p = property( n ) ;
  if ( 0 == p ) { return StatusCode::FAILURE ; }
  // convert the value into the string
  v = p->toString() ;
  //
  return StatusCode::SUCCESS ;
}
// =====================================================================
// Implementation of IProperty::getProperties
// =====================================================================
const std::vector<Property*>&
PropertyMgr::getProperties( ) const { return m_properties; }
// =====================================================================
// Implementation of IInterface::queryInterface
// =====================================================================
StatusCode PropertyMgr::queryInterface(const InterfaceID& iid, void** pinterface) {
  // try local interfaces
  StatusCode sc= base_class::queryInterface(iid, pinterface);
  if (sc.isSuccess()) return sc;
  // fall back on the owner
  return (0 != m_pOuter)? m_pOuter->queryInterface(iid, pinterface)
                        : sc; // FAILURE
}
// =====================================================================
// The END
// =====================================================================
