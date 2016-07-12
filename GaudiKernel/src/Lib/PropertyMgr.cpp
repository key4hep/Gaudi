// hide deprecation warnings here... the whole file is deprecated
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
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
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
// ============================================================================
namespace
{
  // ==========================================================================
  /// case insensitive comparison of strings
  constexpr struct NoCaseCmp_t
  {
    inline bool operator() ( boost::string_ref v1 ,
                             boost::string_ref v2 ) const
    {
      return v1.size() == v2.size() &&
             std::equal(std::begin(v1),std::end(v1),std::begin(v2),
                        [](char c1, char c2) {
                            return toupper(c1) == toupper(c2);
                        } );
    }
  } noCaseCmp {} ;
  // ==========================================================================
  /// get the property by name
  struct PropByName
  {
    boost::string_ref m_name ;

    inline bool operator() ( const Property* p ) const
    { return  p && noCaseCmp( p->name() , m_name ) ; }
  } ;
  // ==========================================================================
}
// ====================================================================
// constructor from the interface
// ====================================================================
PropertyMgr::PropertyMgr(IInterface* iface)
  : m_pOuter           ( iface )
{
  addRef(); // initial reference count set to 1
}
// ====================================================================
// Declare a remote property
// ====================================================================
Property* PropertyMgr::declareRemoteProperty
( boost::string_ref name  ,
  IProperty*         rsvc  ,
  boost::string_ref rname )
{
  if ( !rsvc ) { return nullptr ; }
  boost::string_ref nam = rname.empty() ? name : rname ;
  Property* p = property ( nam , rsvc->getProperties() )  ;
  m_remoteProperties.emplace_back ( name , std::make_pair( rsvc , nam ) ) ;
  return p ;
}
// ====================================================================
// get the property by name form the proposed list
// ====================================================================
Property* PropertyMgr::property
( boost::string_ref            name  ,
  const std::vector<Property*>& props ) const
{
  auto it = std::find_if( props.begin(), props.end(), PropByName{ name } ) ;
  return ( it != props.end() ) ? *it : nullptr;            // RETURN
}
// ====================================================================
// retrieve the property by name
// ====================================================================
Property* PropertyMgr::property
( boost::string_ref name  ) const
{
  // local property ?
  Property* lp = property ( name , m_properties ) ;
  if ( lp ) { return lp ; }                           // RETURN
  // look for remote property
  for ( const auto& it : m_remoteProperties )
  {
    if ( !noCaseCmp(it.first,name) ) { continue ; }   // CONTINUE
    const IProperty* p = it.second.first ;
    if ( !p ) { continue ; }                          // CONTINUE
    return property ( it.second.second , p->getProperties() ) ; // RETURN
  }
  return nullptr ;                                    // RETURN
}
// ====================================================================
/* set a property from another property
 *  Implementation of IProperty::setProperty
 */
// =====================================================================
StatusCode PropertyMgr::setProperty( const Property& p )
{
  Property* pp = property( p.name() ) ;
  try
  { if ( pp && pp->assign(p) ) { return StatusCode::SUCCESS ; } } // RETURN
  catch(...) {  }
  //
  return StatusCode::FAILURE;
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
  return ( p && p->fromString(v) ) ? StatusCode::SUCCESS : StatusCode::FAILURE ;
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
    if ( pp && pp->load(*p) ) return StatusCode::SUCCESS;      // RETURN
  }
  catch ( ... ) {  }
  return StatusCode::FAILURE ;                                // RETURN
}
// =====================================================================
/* Get the property by name
 *  Implementation of IProperty::getProperty
 */
// =====================================================================
const Property& PropertyMgr::getProperty( const std::string& name) const
{
  const Property* p = property( name ) ;
  if ( !p ) throw std::out_of_range( "Property "+name+" not found." );    // Not found
  return *p ;                        // RETURN
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
  if ( !p ) { return StatusCode::FAILURE ; }
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
  return m_pOuter ? m_pOuter->queryInterface(iid, pinterface)
                  : sc; // FAILURE
}
// =====================================================================
// Implementation of IProperty::hasProperty
// =====================================================================
bool PropertyMgr::hasProperty(const std::string& name) const {
  return any_of(begin(m_properties), end(m_properties),
      [&name](const Property* prop) {
    return noCaseCmp(prop->name(), name);
  });
}
void PropertyMgr::assertUniqueName(boost::string_ref name) const {
  if (UNLIKELY(hasProperty(name.to_string()))) {
    auto owner = SmartIF<INamedInterface>( m_pOuter );
    auto msgSvc = Gaudi::svcLocator()->service<IMessageSvc>("MessageSvc");
    if (!msgSvc) { std::cerr<< "error: cannot get MessageSvc!" << std::endl; }
    MsgStream log(msgSvc, owner ? owner->name() : "PropertyMgr"  );
    log << MSG::WARNING
        << "duplicated property name '" << name
        << "', see https://its.cern.ch/jira/browse/GAUDI-1023"<< endmsg;
  }
}
// =====================================================================
// The END
// =====================================================================
