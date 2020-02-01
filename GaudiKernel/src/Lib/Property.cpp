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
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/GaudiHandle.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/PropertyHolder.h"
#include "GaudiKernel/SmartIF.h"
#include <Gaudi/Property.h>
#include <GaudiKernel/ToStream.h>
// ============================================================================
// Boost
// ============================================================================
#include <boost/algorithm/string/compare.hpp>
#include <boost/utility/string_ref.hpp>
// ============================================================================
namespace {
  /// helper class to compare pointers to string using the strings
  struct PtrCmp {
    bool operator()( const std::unique_ptr<std::string>& a, const std::unique_ptr<std::string>& b ) const {
      return *a == *b;
    }
  };
  struct PtrHash {
    std::size_t operator()( const std::unique_ptr<std::string>& s ) const { return std::hash<std::string>()( *s ); }
  };
  /// storage for property names and docs
  std::unordered_set<std::unique_ptr<std::string>, PtrHash, PtrCmp> all_strings;
} // namespace

using Gaudi::Details::PropertyBase;

std::string_view PropertyBase::to_view( std::string str ) {
  return **( all_strings.insert( std::make_unique<std::string>( std::move( str ) ) ).first );
}

// ============================================================================
// the printout of the property value
// ============================================================================
std::ostream& PropertyBase::fillStream( std::ostream& stream ) const {
  return stream << " '" << name() << "':" << toString();
}

// ============================================================================
/*  simple function which check the existence of the property with
 *  the given name.
 *
 *  @code
 *
 *  IInterface* p = .
 *
 *  const bool = hasProperty( p , "Context" ) ;
 *
 *  @endcode
 *
 *  @param  p    pointer to IInterface   object (any component)
 *  @param  name property name (case insensitive)
 *  @return true if "p" has a property with such name
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-09-09
 */
// ============================================================================
bool Gaudi::Utils::hasProperty( const IInterface* p, std::string_view name ) {
  // delegate to another method after trivial check
  return p && getProperty( p, name );
}
// ============================================================================
/*  simple function which check the existence of the property with
 *  the given name.
 *
 *  @code
 *
 *  const IProperty* p = ... ;
 *
 *  const bool = hasProperty( p , "Context" ) ;
 *
 *  @endcode
 *
 *  @param  p    pointer to IProperty object
 *  @param  name property name (case insensitive)
 *  @return true if "p" has a property with such name
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-09-09
 */
// ============================================================================
bool Gaudi::Utils::hasProperty( const IProperty* p, std::string_view name ) {
  // delegate the actual work to another method ;
  return p && p->hasProperty( name );
}
// ============================================================================
//
// GaudiHandleProperty implementation
//
GaudiHandleProperty::GaudiHandleProperty( std::string name_, GaudiHandleBase& ref )
    : PropertyWithHandlers( std::move( name_ ), typeid( GaudiHandleBase ) ), m_pValue( &ref ) {
  m_pValue->setPropertyName( name() );
}

bool GaudiHandleProperty::setValue( const GaudiHandleBase& value ) {
  m_pValue->setTypeAndName( value.typeAndName() );
  return useUpdateHandler();
}

std::string GaudiHandleProperty::toString() const {
  useReadHandler();
  return m_pValue->typeAndName();
}

void GaudiHandleProperty::toStream( std::ostream& out ) const {
  useReadHandler();
  out << m_pValue->typeAndName();
}

StatusCode GaudiHandleProperty::fromString( const std::string& s ) {
  boost::string_ref tn = s;
  /// \fixme strip optional quotes around the string
  if ( ( tn.starts_with( '"' ) && tn.ends_with( '"' ) ) || ( tn.starts_with( '\'' ) && tn.ends_with( '\'' ) ) ) {
    tn.remove_prefix( 1 );
    tn.remove_suffix( 1 );
    m_pValue->setTypeAndName( static_cast<std::string>( tn ) );
  } else {
    m_pValue->setTypeAndName( s );
  }
  useUpdateHandler();
  return StatusCode::SUCCESS;
}

//
// GaudiHandlePropertyArray implementation
//
GaudiHandleArrayProperty::GaudiHandleArrayProperty( std::string name_, GaudiHandleArrayBase& ref )
    : PropertyWithHandlers( std::move( name_ ), typeid( GaudiHandleArrayBase ) ), m_pValue( &ref ) {
  m_pValue->setPropertyName( name() );
}

bool GaudiHandleArrayProperty::setValue( const GaudiHandleArrayBase& value ) {
  m_pValue->setTypesAndNames( value.typesAndNames() );
  return useUpdateHandler();
}

std::string GaudiHandleArrayProperty::toString() const {
  // treat as if a Gaudi::Property<std::vector<std::string>>
  useReadHandler();
  return Gaudi::Utils::toString( m_pValue->typesAndNames() );
}

void GaudiHandleArrayProperty::toStream( std::ostream& out ) const {
  // treat as if a Gaudi::Property<std::vector<std::string>>
  useReadHandler();
  Gaudi::Utils::toStream( m_pValue->typesAndNames(), out );
}

StatusCode GaudiHandleArrayProperty::fromString( const std::string& source ) {
  // treat as if a Gaudi::Property<std::vector<std::string>>
  std::vector<std::string> tmp;
  StatusCode               sc = Gaudi::Parsers::parse( tmp, source );
  if ( sc.isFailure() ) return sc;
  if ( !m_pValue->setTypesAndNames( std::move( tmp ) ) ) return StatusCode::FAILURE;
  useUpdateHandler();
  return StatusCode::SUCCESS;
}

// ============================================================================
namespace {
  template <typename C1, typename C2, typename BinaryPredicate>
  bool equal_( const C1& c1, const C2& c2, BinaryPredicate&& p ) {
    return std::equal( begin( c1 ), end( c1 ), begin( c2 ), end( c2 ), std::forward<BinaryPredicate>( p ) );
  }

  // match (case insensitive) property by name
  template <typename String>
  auto is_iByName( String&& name ) {
    return [name = std::forward<String>( name )]( const PropertyBase* p ) {
      return p && equal_( name, p->name(), boost::algorithm::is_iequal{} );
    };
  }
} // namespace
// ============================================================================
/*  simple function which gets the property with given name
 *  from the component
 *
 *  @code
 *
 *  const IProperty* p = ... ;
 *
 *  auto pro = getProperty( p , "Context" ) ;
 *
 *  @endcode
 *
 *  @param  p    pointer to IProperty object
 *  @param  name property name (case insensitive)
 *  @return property with the given name (if exists), NULL otherwise
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-09-09
 */
// ============================================================================
PropertyBase* Gaudi::Utils::getProperty( const IProperty* p, std::string_view name ) {
  // trivial check
  if ( !p ) { return nullptr; } // RETURN
  // get all properties
  const auto& props = p->getProperties();
  // comparison criteria:
  auto ifound = std::find_if( props.begin(), props.end(), is_iByName( name ) );
  return ifound != props.end() ? *ifound : nullptr;
}
// ============================================================================
/*  simple function which gets the property with given name
 *  from the component
 *
 *  @code
 *
 *  const IInterface* p = ... ;
 *
 *  auto pro = getProperty( p , "Context" ) ;
 *
 *  @endcode
 *
 *  @param  p    pointer to IInterface object
 *  @param  name property name (case insensitive)
 *  @return property with the given name (if exists), NULL otherwise
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-09-09
 */
// ============================================================================
PropertyBase* Gaudi::Utils::getProperty( const IInterface* p, std::string_view name ) {
  // trivial check
  if ( !p ) { return nullptr; } // RETURN
  // remove const-qualifier
  IInterface* _i = const_cast<IInterface*>( p );
  if ( !_i ) { return nullptr; } // RETURN
  SmartIF<IProperty> property( _i );
  return property ? getProperty( property, name ) : nullptr;
}
// ============================================================================
/*  check  the property by name from  the list of the properties
 *
 *  @code
 *
 *   IJobOptionsSvc* svc = ... ;
 *
 *   const std::string client = ... ;
 *
 *  // get the property:
 *  bool context =
 *      hasProperty ( svc->getProperties( client ) , "Context" )
 *
 *  @endcode
 *
 *  @see IJobOptionsSvc
 *
 *  @param  p    list of properties
 *  @param  name property name (case insensitive)
 *  @return true if the property exists
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-09-09
 */
// ============================================================================
bool Gaudi::Utils::hasProperty( const std::vector<const PropertyBase*>* p, std::string_view name ) {
  // delegate to another method
  return getProperty( p, name );
}
// ============================================================================
/*  get the property by name from  the list of the properties
 *
 *  @code
 *
 *   IJobOptionsSvc* svc = ... ;
 *
 *   const std::string client = ... ;
 *
 *  // get the property:
 *  auto context = getProperty ( svc->getProperties( client ) , "Context" )
 *
 *  @endcode
 *
 *  @see IJobOptionsSvc
 *
 *  @param  p    list of properties
 *  @param  name property name (case insensitive)
 *  @return property with the given name (if exists), NULL otherwise
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date   2006-09-09
 */
// ============================================================================
const PropertyBase* Gaudi::Utils::getProperty( const std::vector<const PropertyBase*>* p, std::string_view name ) {
  // trivial check
  if ( !p ) { return nullptr; } // RETURN
  auto ifound = std::find_if( p->begin(), p->end(), is_iByName( name ) );
  return p->end() != ifound ? *ifound : nullptr; // RETURN
}
// ============================================================================
/* the full specialization of the
 *  method setProperty( IProperty, std::string, const TYPE&)
 *  for C-strings
 *
 *  @param component component which needs to be configured
 *  @param name      name of the property
 *  @param value     value of the property
 *  @param doc       the new documentation string
 *
 *  @see IProperty
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-05-13
 */
// ============================================================================
StatusCode Gaudi::Utils::setProperty( IProperty* component, const std::string& name, const char* value,
                                      const std::string& doc ) {
  return Gaudi::Utils::setProperty( component, name, std::string{value}, doc );
}
// ============================================================================
/* the full specialization of the
 * method Gaudi::Utils::setProperty( IProperty, std::string, const TYPE&)
 * for standard strings
 *
 *  @param component component which needs to be configured
 *  @param name      name of the property
 *  @param value     value of the property
 *
 *  @see IProperty
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-05-13
 */
// ============================================================================
StatusCode Gaudi::Utils::setProperty( IProperty* component, const std::string& name, const std::string& value,
                                      const std::string& doc ) {
  if ( !component ) { return StatusCode::FAILURE; } // RETURN
  if ( !component->hasProperty( name ) ) { return StatusCode::FAILURE; }
  StatusCode sc = component->setPropertyRepr( name, value );
  if ( !doc.empty() ) {
    PropertyBase* p = getProperty( component, name );
    if ( p ) { p->setDocumentation( doc ); }
  }
  sc.ignore();
  return sc;
}
// ============================================================================
/*  simple function to set the property of the given object from another
 *  property
 *
 *  @code
 *
 *  IProperty* component = ... ;
 *
 *  const Gaudi::Details::PropertyBase* prop = ... ;
 *  StatusCode sc = setProperty ( component , "Data" ,  prop  ) ;
 *
 *  @endcode
 *
 * @param component component which needs to be configured
 * @param name      name of the property
 * @param property  the property
 * @param doc       the new documentation string
 *
 * @see IProperty
 * @author Vanya BELYAEV ibelyaev@physics.syr.edu
 * @date 2007-05-13
 */
// ============================================================================
StatusCode Gaudi::Utils::setProperty( IProperty* component, const std::string& name, const PropertyBase* property,
                                      const std::string& doc ) {
  if ( !component || !property ) return StatusCode::FAILURE;
  PropertyBase* p = getProperty( component, name );
  if ( !p || !p->assign( *property ) ) return StatusCode::FAILURE;
  if ( !doc.empty() ) { p->setDocumentation( doc ); }
  return StatusCode::SUCCESS;
}
// ============================================================================
/* simple function to set the property of the given object from another
 *  property
 *
 *  @code
 *
 *  IProperty* component = ... ;
 *
 *  const Gaudi::Details::PropertyBase& prop = ... ;
 *  StatusCode sc = setProperty ( component , "Data" ,  prop  ) ;
 *
 *  @endcode
 *
 * @param component component which needs to be configured
 * @param name      name of the property
 * @param property  the property
 * @param doc       the new documentation string
 *
 * @see IProperty
 * @author Vanya BELYAEV ibelyaev@physics.syr.edu
 * @date 2007-05-13
 */
// ============================================================================
StatusCode Gaudi::Utils::setProperty( IProperty* component, const std::string& name, const PropertyBase& property,
                                      const std::string& doc ) {
  return setProperty( component, name, &property, doc );
}
// ============================================================================
/*  the full specialization of the
 *  method setProperty( IInterface , std::string, const TYPE&)
 *  for standard strings
 *
 *  @param component component which needs to be configured
 *  @param name      name of the property
 *  @param value     value of the property
 *  @param doc       the new documentation string
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-05-13
 */
// ============================================================================
StatusCode Gaudi::Utils::setProperty( IInterface* component, const std::string& name, const std::string& value,
                                      const std::string& doc ) {
  if ( !component ) { return StatusCode::FAILURE; }
  SmartIF<IProperty> property( component );
  return property ? setProperty( property, name, value, doc ) : StatusCode::FAILURE;
}
// ============================================================================
/*  the full specialization of the
 *  method setProperty( IInterface , std::string, const TYPE&)
 *  for C-strings
 *
 *  @param component component which needs to be configured
 *  @param name      name of the property
 *  @param value     value of the property
 *  @param doc       the new documentation string
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-05-13
 */
// ============================================================================
StatusCode Gaudi::Utils::setProperty( IInterface* component, const std::string& name, const char* value,
                                      const std::string& doc ) {
  return setProperty( component, name, std::string{value}, doc );
}
// ============================================================================
/*  simple function to set the property of the given object from another
 *  property
 *
 *  @code
 *
 *  IInterface* component = ... ;
 *
 *  const Gaudi::Details::PropertyBase* prop = ... ;
 *  StatusCode sc = setProperty ( component , "Data" ,  prop  ) ;
 *
 *  @endcode
 *
 * @param component component which needs to be configured
 * @param name      name of the property
 * @param property  the property
 * @param doc       the new documentation string
 *
 * @see IProperty
 * @author Vanya BELYAEV ibelyaev@physics.syr.edu
 * @date 2007-05-13
 */
// ============================================================================
StatusCode Gaudi::Utils::setProperty( IInterface* component, const std::string& name, const PropertyBase* property,
                                      const std::string& doc ) {
  if ( !component ) { return StatusCode::FAILURE; }
  SmartIF<IProperty> prop( component );
  if ( !prop ) { return StatusCode::FAILURE; }
  return setProperty( prop, name, property, doc );
}
// ============================================================================
/*  simple function to set the property of the given object from another
 *  property
 *
 *  @code
 *
 *  IInterface* component = ... ;
 *
 *  const Gaudi::Details::PropertyBase& prop = ... ;
 *  StatusCode sc = setProperty ( component , "Data" ,  prop  ) ;
 *
 *  @endcode
 *
 * @param component component which needs to be configured
 * @param name      name of the property
 * @param property  the property
 * @param doc       the new documentation string
 *
 * @see IProperty
 * @author Vanya BELYAEV ibelyaev@physics.syr.edu
 * @date 2007-05-13
 */
// ============================================================================
StatusCode Gaudi::Utils::setProperty( IInterface* component, const std::string& name, const PropertyBase& property,
                                      const std::string& doc ) {
  return setProperty( component, name, &property, doc );
}
// ============================================================================

Gaudi::Details::WeakPropertyRef::operator std::string() const {
  using Gaudi::Utils::toString;
  return m_property ? ( ( m_property->type_info() == &typeid( std::string ) ) ? toString( m_property->toString() )
                                                                              : m_property->toString() )
                    : m_value;
}

namespace Gaudi::Details::Property {
  namespace {
#ifndef GAUDI_PROPERTY_PARSING_ERROR_DEFAULT_POLICY
#  define GAUDI_PROPERTY_PARSING_ERROR_DEFAULT_POLICY Ignore
#endif
    ParsingErrorPolicy g_parsingErrorPolicy = ParsingErrorPolicy::GAUDI_PROPERTY_PARSING_ERROR_DEFAULT_POLICY;
  } // namespace
  ParsingErrorPolicy parsingErrorPolicy() { return g_parsingErrorPolicy; }
  ParsingErrorPolicy setParsingErrorPolicy( ParsingErrorPolicy p ) {
    auto tmp             = g_parsingErrorPolicy;
    g_parsingErrorPolicy = p;
    return tmp;
  }
} // namespace Gaudi::Details::Property

namespace {
  struct InitParsingErrorPolicy {
    InitParsingErrorPolicy() {
      using Gaudi::Details::Property::ParsingErrorPolicy;
      using Gaudi::Details::Property::setParsingErrorPolicy;
      std::string policy;
      if ( System::getEnv( "GAUDI_PROPERTY_PARSING_ERROR_DEFAULT_POLICY", policy ) ) {
        switch ( policy[0] ) {
        case 'I':
          setParsingErrorPolicy( ParsingErrorPolicy::Ignore );
          break;
        case 'W':
          setParsingErrorPolicy( ParsingErrorPolicy::Warning );
          break;
        case 'E':
          setParsingErrorPolicy( ParsingErrorPolicy::Exception );
          break;
        case 'A':
          setParsingErrorPolicy( ParsingErrorPolicy::Abort );
          break;
        default:
          break;
        }
      }
    }
  } initParsingErrorPolicy;
} // namespace
