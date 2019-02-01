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
#include "GaudiKernel/Property.h"
#include "GaudiKernel/PropertyHolder.h"
#include "GaudiKernel/SmartIF.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/algorithm/string/compare.hpp"
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

boost::string_ref PropertyBase::to_view( std::string str ) {
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
bool Gaudi::Utils::hasProperty( const IInterface* p, const std::string& name ) {
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
bool Gaudi::Utils::hasProperty( const IProperty* p, const std::string& name ) {
  // delegate the actual work to another method ;
  return p && getProperty( p, name );
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
  m_pValue->setTypeAndName( s );
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
  template <typename C, typename BinaryPredicate>
  bool equal_( const C& c1, const C& c2, BinaryPredicate&& p ) {
    return c1.size() == c2.size() &&
           std::equal( std::begin( c1 ), std::end( c1 ), std::begin( c2 ), std::forward<BinaryPredicate>( p ) );
  }

  // match (case insensitive) property by name
  struct is_iByName {
    /// constructor from name
    is_iByName( const std::string& name ) : m_name( name ) {}
    /// the most essential method:
    bool operator()( const PropertyBase* p ) const {
      return p && equal_( m_name, p->name(), boost::algorithm::is_iequal{} );
    };

  private:
    const std::string& m_name;
  };
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
PropertyBase* Gaudi::Utils::getProperty( const IProperty* p, const std::string& name ) {
  // trivial check
  if ( !p ) { return nullptr; } // RETURN
  // get all properties
  const auto& props = p->getProperties();
  // comparison criteria:
  auto ifound = std::find_if( props.begin(), props.end(), is_iByName{name} );
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
PropertyBase* Gaudi::Utils::getProperty( const IInterface* p, const std::string& name ) {
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
bool Gaudi::Utils::hasProperty( const std::vector<const PropertyBase*>* p, const std::string& name ) {
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
const PropertyBase* Gaudi::Utils::getProperty( const std::vector<const PropertyBase*>* p, const std::string& name ) {
  // trivial check
  if ( !p ) { return nullptr; } // RETURN
  auto ifound = std::find_if( p->begin(), p->end(), is_iByName{name} );
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
  if ( !hasProperty( component, name ) ) { return StatusCode::FAILURE; }
  StatusCode sc = component->setProperty( name, value );
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

// ============================================================================
// The END
// ============================================================================
