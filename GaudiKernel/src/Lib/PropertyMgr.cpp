/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// hide deprecation warnings here... the whole file is deprecated
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
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

using Gaudi::Details::PropertyBase;

namespace {
  // ==========================================================================
  /// case insensitive comparison of strings
  constexpr struct NoCaseCmp_t {
    inline bool operator()( const std::string& v1, const std::string& v2 ) const {
      return v1.size() == v2.size() && std::equal( std::begin( v1 ), std::end( v1 ), std::begin( v2 ),
                                                   []( char c1, char c2 ) { return toupper( c1 ) == toupper( c2 ); } );
    }
  } noCaseCmp{};
  // ==========================================================================
  /// get the property by name
  struct PropByName {
    std::string m_name;

    inline bool operator()( const PropertyBase* p ) const { return p && noCaseCmp( p->name(), m_name ); }
  };
  // ==========================================================================
} // namespace
// ====================================================================
// constructor from the interface
// ====================================================================
PropertyMgr::PropertyMgr( IInterface* iface ) : m_pOuter( iface ) {
  addRef(); // initial reference count set to 1
}
// ====================================================================
// Declare a remote property
// ====================================================================
PropertyBase* PropertyMgr::declareRemoteProperty( const std::string& name, IProperty* rsvc, const std::string& rname ) {
  if ( !rsvc ) return nullptr;
  const std::string& nam = rname.empty() ? name : rname;
  PropertyBase*      p   = property( nam, rsvc->getProperties() );
  m_remoteProperties.emplace_back( name, std::make_pair( rsvc, nam ) );
  return p;
}
// ============================================================================
// Declare a property
// ============================================================================
PropertyBase* PropertyMgr::declareProperty( const std::string& name, GaudiHandleBase& ref, const std::string& doc ) {
  assertUniqueName( name );
  m_todelete.emplace_back( new typename GaudiHandleBase::PropertyType( name, ref ) );
  Property* p = m_todelete.back().get();
  //
  p->setDocumentation( doc );
  m_properties.push_back( p );
  //
  return p;
}
// ============================================================================
PropertyBase* PropertyMgr::declareProperty( const std::string& name, GaudiHandleArrayBase& ref,
                                            const std::string& doc ) {
  assertUniqueName( name );
  m_todelete.emplace_back( new typename GaudiHandleArrayBase::PropertyType( name, ref ) );
  Property* p = m_todelete.back().get();
  //
  p->setDocumentation( doc );
  m_properties.push_back( p );
  //
  return p;
}

// ============================================================================
PropertyBase* PropertyMgr::declareProperty( const std::string& name, DataObjectHandleBase& ref,
                                            const std::string& doc ) {
  assertUniqueName( name );
  m_todelete.emplace_back( new typename DataObjectHandleBase::PropertyType( name, ref ) );
  Property* p = m_todelete.back().get();
  //
  p->setDocumentation( doc );
  m_properties.push_back( p );
  //
  return p;
}
// ====================================================================
// get the property by name form the proposed list
// ====================================================================
PropertyBase* PropertyMgr::property( const std::string& name, const std::vector<PropertyBase*>& props ) const {
  auto it = std::find_if( props.begin(), props.end(), PropByName{name} );
  return ( it != props.end() ) ? *it : nullptr; // RETURN
}
// ====================================================================
// retrieve the property by name
// ====================================================================
PropertyBase* PropertyMgr::property( const std::string& name ) const {
  // local property ?
  PropertyBase* lp = property( name, m_properties );
  if ( lp ) return lp; // RETURN
  // look for remote property
  for ( const auto& it : m_remoteProperties ) {
    if ( !noCaseCmp( it.first, name ) ) continue; // CONTINUE
    const IProperty* p = it.second.first;
    if ( !p ) continue;
    return property( it.second.second, p->getProperties() ); // RETURN
  }
  return nullptr; // RETURN
}
// ====================================================================
/* set a property from another property
 *  Implementation of IProperty::setProperty
 */
// =====================================================================
StatusCode PropertyMgr::setProperty( const PropertyBase& p ) {
  PropertyBase* pp = property( p.name() );
  try {
    if ( pp && pp->assign( p ) ) return StatusCode::SUCCESS;
  } // RETURN
  catch ( ... ) {}
  //
  return StatusCode::FAILURE;
}
// ====================================================================
/* set a property from the stream
 *  Implementation of IProperty::setProperty
 */
// =====================================================================
StatusCode PropertyMgr::setProperty( const std::string& i ) {
  std::string name, value;
  StatusCode  sc = Gaudi::Parsers::parse( name, value, i );
  return sc.isFailure() ? sc : setProperty( name, value );
}
// =====================================================================
/* set a property from the string
 *  Implementation of IProperty::setProperty
 */
// =====================================================================
StatusCode PropertyMgr::setProperty( const std::string& n, const std::string& v ) {
  PropertyBase* p = property( n );
  return ( p && p->fromString( v ) ) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
// =====================================================================
/* Retrieve the value of a property
 *  Implementation of IProperty::getProperty
 */
// =====================================================================
StatusCode PropertyMgr::getProperty( PropertyBase* p ) const {
  try {
    const PropertyBase* pp = property( p->name() );
    if ( pp && pp->load( *p ) ) return StatusCode::SUCCESS; // RETURN
  } catch ( ... ) {}
  return StatusCode::FAILURE; // RETURN
}
// =====================================================================
/* Get the property by name
 *  Implementation of IProperty::getProperty
 */
// =====================================================================
const PropertyBase& PropertyMgr::getProperty( const std::string& name ) const {
  const PropertyBase* p = property( name );
  if ( !p ) throw std::out_of_range( "Property " + name + " not found." ); // Not found
  return *p;                                                               // RETURN
}
// =====================================================================
/* Get the property by name
 *  Implementation of IProperty::getProperty
 */
// =====================================================================
StatusCode PropertyMgr::getProperty( const std::string& n, std::string& v ) const {
  // get the property
  const PropertyBase* p = property( n );
  if ( !p ) return StatusCode::FAILURE;
  // convert the value into the string
  v = p->toString();
  //
  return StatusCode::SUCCESS;
}
// =====================================================================
// Implementation of IProperty::getProperties
// =====================================================================
const std::vector<PropertyBase*>& PropertyMgr::getProperties() const { return m_properties; }
// =====================================================================
// Implementation of IInterface::queryInterface
// =====================================================================
StatusCode PropertyMgr::queryInterface( const InterfaceID& iid, void** pinterface ) {
  // try local interfaces
  StatusCode sc = base_class::queryInterface( iid, pinterface );
  if ( sc.isSuccess() ) return sc;
  // fall back on the owner
  return m_pOuter ? m_pOuter->queryInterface( iid, pinterface ) : sc; // FAILURE
}
// =====================================================================
// Implementation of IProperty::hasProperty
// =====================================================================
bool PropertyMgr::hasProperty( const std::string& name ) const {
  return any_of( begin( m_properties ), end( m_properties ),
                 [&name]( const PropertyBase* prop ) { return noCaseCmp( prop->name(), name ); } );
}
void PropertyMgr::assertUniqueName( const std::string& name ) const {
  if ( LIKELY( !hasProperty( name ) ) ) return;
  auto msgSvc = Gaudi::svcLocator()->service<IMessageSvc>( "MessageSvc" );
  if ( !msgSvc ) std::cerr << "error: cannot get MessageSvc!" << std::endl;
  auto      owner = SmartIF<INamedInterface>( m_pOuter );
  MsgStream log( msgSvc, owner ? owner->name() : "PropertyMgr" );
  log << MSG::WARNING << "duplicated property name '" << name << "', see https://its.cern.ch/jira/browse/GAUDI-1023"
      << endmsg;
}
// =====================================================================
// The END
// =====================================================================
