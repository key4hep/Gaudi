/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_PROPERTYHOLDER_H
#define GAUDIKERNEL_PROPERTYHOLDER_H
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/DataHandleProperty.h"
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/detected.h"
#include <Gaudi/Property.h>

#include "Gaudi/Interfaces/IOptionsSvc.h"
// ============================================================================
namespace Gaudi {
  namespace Details {

    template <typename T>
    struct is_gaudi_property : std::false_type {};

    template <typename TYPE, typename VERIFIER, typename HANDLERS>
    struct is_gaudi_property<Gaudi::Property<TYPE, VERIFIER, HANDLERS>> : std::true_type {};

    template <typename T>
    using PropertyType_t = typename std::remove_reference_t<T>::PropertyType;
    template <typename T>
    using PropertyType = Gaudi::cpp17::detected_or_t<Gaudi::Property<T>, PropertyType_t, T>;
  } // namespace Details
  namespace Utils {
    /// Helper for case insensitive string comparison.
    inline bool iequal( const std::string& v1, const std::string& v2 ) {
      return v1.size() == v2.size() && std::equal( std::begin( v1 ), std::end( v1 ), std::begin( v2 ),
                                                   []( char c1, char c2 ) { return toupper( c1 ) == toupper( c2 ); } );
    }
  } // namespace Utils
} // namespace Gaudi
/** Helper class to implement the IProperty interface.
 *
 *  PropertyHolder is used by components base classes (Algorithm, Service,
 *  etc.) to provide a default implementation the IProperty interface.
 *
 *  When needing to implement the IProperty interface in a class, it is
 *  enough to wrap the base of the class with PropertyHolder, as in
 *
 *  \code{.cpp}
 *  class MyClass : public PropertyHolder<BaseClass> {
 *    // ...
 *  };
 *  \endcode
 *
 *  where \c BaseClass should inherit from IProperty and INamedInterface.
 *
 *  \author Paul Maley
 *  \author David Quarrie
 *  \author Marco Clemencic
 */
template <class BASE>
class GAUDI_API PropertyHolder : public BASE {
  static_assert( std::is_base_of_v<IProperty, BASE> && std::is_base_of_v<INamedInterface, BASE>,
                 "PropertyHolder template argument must inherit from IProperty and INamedInterface" );

public:
  /// Typedef used to refer to this class from derived classes, as in
  /// \code{.cpp}
  /// class MyClass : public PropertyHolder<BaseClass> {
  ///   using PropertyHolderImpl::declareProperty;
  /// };
  /// \endcode
  using PropertyHolderImpl = PropertyHolder<BASE>;

  PropertyHolder() = default;

  /// \{
  /// prevent copies
  PropertyHolder( const PropertyHolder& ) = delete;
  PropertyHolder& operator=( const PropertyHolder& ) = delete;
  /// \}

public:
  /// Declare a property.
  /// Record a PropertyBase instance to be managed by PropertyHolder.
  inline Gaudi::Details::PropertyBase& declareProperty( Gaudi::Details::PropertyBase& prop ) {
    assertUniqueName( prop.name() );
    m_properties.push_back( &prop );
    return prop;
  }

  /// Helper to wrap a regular data member and use it as a regular property.
  /// \deprecated Prefer the signatures using a a fully initialized PropertyBase instance.
  template <typename TYPE, typename = std::enable_if_t<!Gaudi::Details::is_gaudi_property<TYPE>::value>>
  Gaudi::Details::PropertyBase* declareProperty( const std::string& name, TYPE& value,
                                                 const std::string& doc = "none" ) {
    m_todelete.push_back( std::make_unique<Gaudi::Details::PropertyType<TYPE&>>( name, value ) );
    Gaudi::Details::PropertyBase* p = m_todelete.back().get();

    p->setDocumentation( doc );
    return &declareProperty( *p );
  }

  /// Declare a PropertyBase instance setting name and documentation.
  /// \deprecated Prefer the signatures using a fully initialized PropertyBase instance.
  template <class TYPE, class VERIFIER, class HANDLERS>
  Gaudi::Details::PropertyBase* declareProperty( const std::string&                         name,
                                                 Gaudi::Property<TYPE, VERIFIER, HANDLERS>& prop,
                                                 const std::string&                         doc = "none" ) {
    Gaudi::Details::PropertyBase* p = &prop;
    p->setName( name );

    p->setDocumentation( doc );
    return &declareProperty( *p );
  }

  /// Declare a remote property.
  /// Bind \c name to the property \c rname of \c rsvc.
  Gaudi::Details::PropertyBase* declareRemoteProperty( const std::string& name, IProperty* rsvc,
                                                       const std::string& rname = "" ) {
    if ( !rsvc ) return nullptr;
    const std::string&            nam = rname.empty() ? name : rname;
    Gaudi::Details::PropertyBase* p   = property( nam, rsvc->getProperties() );
    m_remoteProperties.emplace_back( RemProperty{name, rsvc, nam} );
    return p;
  }

  /// \}

  // ==========================================================================
  // IProperty implementation
  // ==========================================================================
  using IProperty::setProperty;
  /** set the property from another property with a different name
   *  @see IProperty
   */
  StatusCode setProperty( const std::string& name, const Gaudi::Details::PropertyBase& p ) override {
    Gaudi::Details::PropertyBase* pp = property( name );
    if ( pp && pp->assign( p ) ) return StatusCode::SUCCESS;
    return StatusCode::FAILURE;
  }
  // ==========================================================================
  /** set the property from the formatted string
   *  @see IProperty
   */
  StatusCode setProperty( const std::string& s ) override {
    std::string name;
    std::string value;
    StatusCode  sc = Gaudi::Parsers::parse( name, value, s );
    if ( sc.isFailure() ) return sc;
    return setPropertyRepr( name, value );
  }
  // ==========================================================================
  /** set the property from name and value string representation
   *  @see IProperty
   */
  StatusCode setPropertyRepr( const std::string& n, const std::string& r ) override {
    try {
      Gaudi::Details::PropertyBase* p = property( n );
      /// @fixme SUCCESS is not required to be checked for compatibility with Gaudi::Utils::setProperty
      return ( p && p->fromString( r ) ) ? StatusCode::SUCCESS : StatusCode::FAILURE;
    } catch ( const std::invalid_argument& err ) {
      throw GaudiException{"error setting property " + n, this->name(), StatusCode::FAILURE, err};
    }
  }
  // ==========================================================================
  /** get the property
   *  @see IProperty
   */
  StatusCode getProperty( Gaudi::Details::PropertyBase* p ) const override {
    try {
      const Gaudi::Details::PropertyBase* pp = property( p->name() );
      if ( pp && pp->load( *p ) ) return StatusCode::SUCCESS;
    } catch ( ... ) {}
    return StatusCode::FAILURE;
  }
  // ==========================================================================
  /** get the property by name
   *  @see IProperty
   */
  const Gaudi::Details::PropertyBase& getProperty( const std::string& name ) const override {
    const Gaudi::Details::PropertyBase* p = property( name );
    if ( !p ) throw std::out_of_range( "Property " + name + " not found." );
    return *p;
  }
  // ==========================================================================
  /** convert the property to the string
   *  @see IProperty
   */
  StatusCode getProperty( const std::string& n, std::string& v ) const override {
    // get the property
    const Gaudi::Details::PropertyBase* p = property( n );
    if ( !p ) return StatusCode::FAILURE;
    // convert the value into the string
    v = p->toString();
    return StatusCode::SUCCESS;
  }
  // ==========================================================================
  /** get all properties
   *  @see IProperty
   */
  const std::vector<Gaudi::Details::PropertyBase*>& getProperties() const override { return m_properties; }
  // ==========================================================================
  /** Return true if we have a property with the given name.
   *  @see IProperty
   */
  bool hasProperty( const std::string& name ) const override {
    return any_of( begin( m_properties ), end( m_properties ),
                   [&name]( const Gaudi::Details::PropertyBase* prop ) {
                     return Gaudi::Utils::iequal( prop->name(), name );
                   } ) ||
           any_of( begin( m_remoteProperties ), end( m_remoteProperties ),
                   [&name]( const auto& prop ) { return Gaudi::Utils::iequal( prop.name, name ); } );
  }
  // ==========================================================================
  /// \fixme property and bindPropertiesTo should be protected
  // get local or remote property by name
  Gaudi::Details::PropertyBase* property( const std::string& name ) const {
    // local property ?
    Gaudi::Details::PropertyBase* lp = property( name, m_properties );
    if ( lp ) return lp;
    // look for remote property
    for ( const auto& it : m_remoteProperties ) {
      if ( !Gaudi::Utils::iequal( it.name, name ) ) continue;
      const IProperty* p = it.owner;
      if ( !p ) continue;
      return property( it.remName, p->getProperties() );
    }
    return nullptr; // RETURN
  }

  void bindPropertiesTo( Gaudi::Interfaces::IOptionsSvc& optsSvc ) {
    auto set_prop = [&optsSvc, this]( auto prop ) { optsSvc.bind( this->name(), prop ); };
    std::for_each( begin( m_properties ), end( m_properties ), set_prop );
    std::for_each( begin( m_remoteProperties ), end( m_remoteProperties ), [&set_prop, this]( auto& rem ) {
      if ( rem.owner ) set_prop( this->property( rem.remName, rem.owner->getProperties() ) );
    } );
  }

private:
  /// get the property by name form the proposed list
  Gaudi::Details::PropertyBase* property( const std::string&                                name,
                                          const std::vector<Gaudi::Details::PropertyBase*>& props ) const {
    auto it = std::find_if( props.begin(), props.end(), [&name]( Gaudi::Details::PropertyBase* p ) {
      return p && Gaudi::Utils::iequal( p->name(), name );
    } );
    return ( it != props.end() ) ? *it : nullptr; // RETURN
  }

  /// Issue a runtime warning if the name is already present in the
  /// list of properties (see <a href="https://its.cern.ch/jira/browse/GAUDI-1023">GAUDI-1023</a>).
  void assertUniqueName( const std::string& name ) const {
    if ( UNLIKELY( hasProperty( name ) ) ) {
      auto msgSvc = Gaudi::svcLocator()->service<IMessageSvc>( "MessageSvc" );
      if ( !msgSvc ) std::cerr << "error: cannot get MessageSvc!" << std::endl;
      MsgStream log( msgSvc, this->name() );
      log << MSG::WARNING << "duplicated property name '" << name << "', see https://its.cern.ch/jira/browse/GAUDI-1023"
          << endmsg;
    }
  }

  typedef std::vector<Gaudi::Details::PropertyBase*> Properties;
  struct RemProperty {
    std::string name;
    IProperty*  owner = nullptr;
    std::string remName;
  };
  typedef std::vector<RemProperty> RemoteProperties;

  /// Collection of all declared properties.
  Properties m_properties;
  /// Collection of all declared remote properties.
  RemoteProperties m_remoteProperties;
  /// Properties owned by PropertyHolder, to be deleted.
  std::vector<std::unique_ptr<Gaudi::Details::PropertyBase>> m_todelete;
};
#endif
