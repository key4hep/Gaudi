// ============================================================================
// Include files
// ============================================================================
// STD& STL
// ============================================================================
#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/PropertyMgr.h"
// ============================================================================
namespace
{
  // ==========================================================================
  /// case insensitive comparison of strings
  constexpr struct NoCaseCmp_t {
    inline bool operator()( const std::string& v1, const std::string& v2 ) const
    {
      return v1.size() == v2.size() && std::equal( std::begin( v1 ), std::end( v1 ), std::begin( v2 ),
                                                   []( char c1, char c2 ) { return toupper( c1 ) == toupper( c2 ); } );
    }
  } noCaseCmp{};
  // ==========================================================================
  /// get the property by name
  struct PropByName {
    std::string m_name;

    inline bool operator()( const Property* p ) const { return p && noCaseCmp( p->name(), m_name ); }
  };
  // ==========================================================================
}
// ====================================================================
// Declare a remote property
// ====================================================================
Property* PropertyMgr::declareRemoteProperty( const std::string& name, IProperty* rsvc, const std::string& rname )
{
  if ( !rsvc ) {
    return nullptr;
  }
  const std::string& nam = rname.empty() ? name : rname;
  Property* p            = property( nam, rsvc->getProperties() );
  m_remoteProperties.emplace_back( name, std::make_pair( rsvc, nam ) );
  return p;
}
// ====================================================================
// get the property by name form the proposed list
// ====================================================================
Property* PropertyMgr::property( const std::string& name, const std::vector<Property*>& props ) const
{
  auto it = std::find_if( props.begin(), props.end(), PropByName{name} );
  return ( it != props.end() ) ? *it : nullptr; // RETURN
}
// ====================================================================
// retrieve the property by name
// ====================================================================
Property* PropertyMgr::property( const std::string& name ) const
{
  // local property ?
  Property* lp = property( name, m_properties );
  if ( lp ) {
    return lp;
  } // RETURN
  // look for remote property
  for ( const auto& it : m_remoteProperties ) {
    if ( !noCaseCmp( it.first, name ) ) {
      continue;
    } // CONTINUE
    const IProperty* p = it.second.first;
    if ( !p ) {
      continue;
    }                                                        // CONTINUE
    return property( it.second.second, p->getProperties() ); // RETURN
  }
  return nullptr; // RETURN
}
// ====================================================================
/* set a property from another property
 *  Implementation of IProperty::setProperty
 */
// =====================================================================
StatusCode PropertyMgr::setProperty( const Property& p )
{
  Property* pp = property( p.name() );
  try {
    if ( pp && pp->assign( p ) ) {
      return StatusCode::SUCCESS;
    }
  } // RETURN
  catch ( ... ) {
  }
  //
  return StatusCode::FAILURE;
}
// ====================================================================
/* set a property from the stream
 *  Implementation of IProperty::setProperty
 */
// =====================================================================
StatusCode PropertyMgr::setProperty( const std::string& i )
{
  std::string name;
  std::string value;
  StatusCode sc = Gaudi::Parsers::parse( name, value, i );
  if ( sc.isFailure() ) {
    return sc;
  }
  return setProperty( name, value );
}
// =====================================================================
/* set a property from the string
 *  Implementation of IProperty::setProperty
 */
// =====================================================================
StatusCode PropertyMgr::setProperty( const std::string& n, const std::string& v )
{
  Property* p = property( n );
  return ( p && p->fromString( v ) ) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
// =====================================================================
/* Retrieve the value of a property
 *  Implementation of IProperty::getProperty
 */
// =====================================================================
StatusCode PropertyMgr::getProperty( Property* p ) const
{
  try {
    const Property* pp = property( p->name() );
    if ( pp && pp->load( *p ) ) return StatusCode::SUCCESS; // RETURN
  } catch ( ... ) {
  }
  return StatusCode::FAILURE; // RETURN
}
// =====================================================================
/* Get the property by name
 *  Implementation of IProperty::getProperty
 */
// =====================================================================
const Property& PropertyMgr::getProperty( const std::string& name ) const
{
  const Property* p = property( name );
  if ( !p ) throw std::out_of_range( "Property " + name + " not found." ); // Not found
  return *p;                                                               // RETURN
}
// =====================================================================
/* Get the property by name
 *  Implementation of IProperty::getProperty
 */
// =====================================================================
StatusCode PropertyMgr::getProperty( const std::string& n, std::string& v ) const
{
  // get the property
  const Property* p = property( n );
  if ( !p ) {
    return StatusCode::FAILURE;
  }
  // convert the value into the string
  v = p->toString();
  //
  return StatusCode::SUCCESS;
}
// =====================================================================
// Implementation of IProperty::getProperties
// =====================================================================
const std::vector<Property*>& PropertyMgr::getProperties() const { return m_properties; }
// =====================================================================
// Implementation of IProperty::hasProperty
// =====================================================================
bool PropertyMgr::hasProperty( const std::string& name ) const
{
  return any_of( begin( m_properties ), end( m_properties ),
                 [&name]( const Property* prop ) { return noCaseCmp( prop->name(), name ); } );
}
void PropertyMgr::assertUniqueName( const std::string& name ) const
{
  if ( UNLIKELY( hasProperty( name ) ) ) {
    auto msgSvc = Gaudi::svcLocator()->service<IMessageSvc>( "MessageSvc" );
    if ( !msgSvc ) {
      std::cerr << "error: cannot get MessageSvc!" << std::endl;
    }
    MsgStream log( msgSvc, this->name() );
    log << MSG::WARNING << "duplicated property name '" << name << "', see https://its.cern.ch/jira/browse/GAUDI-1023"
        << endmsg;
  }
}
// =====================================================================
// The END
// =====================================================================
