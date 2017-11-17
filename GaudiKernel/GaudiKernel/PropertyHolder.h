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
#include "GaudiKernel/DataObjectHandleProperty.h"
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Property.h"

// ============================================================================

// pre-declaration of GaudiHandles is sufficient
template <class T>
class ToolHandle;
template <class T>
class ServiceHandle;
template <class T>
class ToolHandleArray;
template <class T>
class ServiceHandleArray;
template <class T>
class DataObjectHandle;

namespace Gaudi
{
  namespace Utils
  {
    /// Helper for case insensitive string comparison.
    inline bool iequal( const std::string& v1, const std::string& v2 )
    {
      return v1.size() == v2.size() && std::equal( std::begin( v1 ), std::end( v1 ), std::begin( v2 ),
                                                   []( char c1, char c2 ) { return toupper( c1 ) == toupper( c2 ); } );
    }
  }
}
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
class GAUDI_API PropertyHolder : public BASE
{
  static_assert( std::is_base_of<IProperty, BASE>::value && std::is_base_of<INamedInterface, BASE>::value,
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
  inline Gaudi::Details::PropertyBase& declareProperty( Gaudi::Details::PropertyBase& prop )
  {
    assertUniqueName( prop.name() );
    m_properties.push_back( &prop );
    return prop;
  }

  /// Helper to wrap a regular data member and use it as a regular property.
  /// \deprecated Prefer the signatures using a a fully initialized PropertyBase instance.
  template <class TYPE, typename = std::enable_if_t<!std::is_base_of<GaudiHandleBase, TYPE>::value &&
                                                    !std::is_base_of<GaudiHandleArrayBase, TYPE>::value &&
                                                    !std::is_base_of<DataObjectHandleBase, TYPE>::value>>
  Gaudi::Details::PropertyBase* declareProperty( const std::string& name, TYPE& value, const std::string& doc = "none" )
  {
    assertUniqueName( name );
    m_todelete.emplace_back( new Gaudi::Property<TYPE&>( name, value ) );
    Gaudi::Details::PropertyBase* p = m_todelete.back().get();
    p->setDocumentation( doc );
    m_properties.push_back( p );
    return p;
  }

  /// \deprecated Kept for backward compatibility, use the non-const version instead, will be removed in v29r0.
  template <class TYPE, typename = std::enable_if_t<!std::is_base_of<GaudiHandleBase, TYPE>::value &&
                                                    !std::is_base_of<GaudiHandleArrayBase, TYPE>::value &&
                                                    !std::is_base_of<DataObjectHandleBase, TYPE>::value>>
  [[deprecated(
      "Kept for backward compatibility, use the non-const version instead, will be removed in v29r0" )]] Gaudi::
      Details::PropertyBase*
      declareProperty( const std::string& name, TYPE& value, const std::string& doc = "none" ) const
  {
    return const_cast<PropertyHolder*>( this )->declareProperty<TYPE>( name, value, doc );
  }

  /// Declare a PropertyBase instance setting name and documentation.
  /// \deprecated Prefer the signatures using a fully initialized PropertyBase instance.
  template <class TYPE, class VERIFIER, class HANDLERS>
  Gaudi::Details::PropertyBase* declareProperty( const std::string& name,
                                                 Gaudi::Property<TYPE, VERIFIER, HANDLERS>& prop,
                                                 const std::string& doc = "none" )
  {
    assertUniqueName( name );
    Gaudi::Details::PropertyBase* p = &prop;
    p->setName( name );
    p->setDocumentation( doc );
    m_properties.push_back( p );
    return p;
  }

  /// Declare a remote property.
  /// Bind \c name to the property \c rname of \c rsvc.
  Gaudi::Details::PropertyBase* declareRemoteProperty( const std::string& name, IProperty* rsvc,
                                                       const std::string& rname = "" )
  {
    if ( !rsvc ) {
      return nullptr;
    }
    const std::string& nam          = rname.empty() ? name : rname;
    Gaudi::Details::PropertyBase* p = property( nam, rsvc->getProperties() );
    m_remoteProperties.emplace_back( name, std::make_pair( rsvc, nam ) );
    return p;
  }

  /// Specializations for various GaudiHandles
  /// \{
  Gaudi::Details::PropertyBase* declareProperty( const std::string& name, GaudiHandleBase& ref,
                                                 const std::string& doc = "none" )
  {
    assertUniqueName( name );
    m_todelete.emplace_back( new GaudiHandleProperty( name, ref ) );
    Gaudi::Details::PropertyBase* p = m_todelete.back().get();

    p->setDocumentation( doc );
    m_properties.push_back( p );

    return p;
  }
  Gaudi::Details::PropertyBase* declareProperty( const std::string& name, GaudiHandleArrayBase& ref,
                                                 const std::string& doc = "none" )
  {
    assertUniqueName( name );
    m_todelete.emplace_back( new GaudiHandleArrayProperty( name, ref ) );
    Gaudi::Details::PropertyBase* p = m_todelete.back().get();

    p->setDocumentation( doc );
    m_properties.push_back( p );

    return p;
  }
  Gaudi::Details::PropertyBase* declareProperty( const std::string& name, DataObjectHandleBase& ref,
                                                 const std::string& doc = "none" )
  {
    assertUniqueName( name );
    m_todelete.emplace_back( new DataObjectHandleProperty( name, ref ) );
    Gaudi::Details::PropertyBase* p = m_todelete.back().get();

    p->setDocumentation( doc );
    m_properties.push_back( p );

    return p;
  }
  /// \}

  // ==========================================================================
  // IProperty implementation
  // ==========================================================================
  /** set the property form another property
   *  @see IProperty
   */
  StatusCode setProperty( const Gaudi::Details::PropertyBase& p ) override
  {
    Gaudi::Details::PropertyBase* pp = property( p.name() );
    try {
      if ( pp && pp->assign( p ) ) {
        return StatusCode::SUCCESS;
      }
    } catch ( ... ) {
    }
    return StatusCode::FAILURE;
  }
  // ==========================================================================
  /** set the property from the formatted string
   *  @see IProperty
   */
  StatusCode setProperty( const std::string& s ) override
  {
    std::string name;
    std::string value;
    StatusCode sc = Gaudi::Parsers::parse( name, value, s );
    if ( sc.isFailure() ) {
      return sc;
    }
    return setProperty( name, value );
  }
  // ==========================================================================
  /** set the property from name and the value
   *  @see IProperty
   */
  StatusCode setProperty( const std::string& n, const std::string& v ) override
  {
    Gaudi::Details::PropertyBase* p = property( n );
    return ( p && p->fromString( v ) ) ? StatusCode::SUCCESS : StatusCode::FAILURE;
  }
  /** set the property form the value
   *
   *  @code
   *
   *  std::vector<double> data = ... ;
   *  setProperty( "Data" , data ) ;
   *
   *  std::map<std::string,double> cuts = ... ;
   *  setProperty( "Cuts" , cuts ) ;
   *
   *  std::map<std::string,std::string> dict = ... ;
   *  setProperty( "Dictionary" , dict ) ;
   *
   *  @endcode
   *
   *  Note: the interface IProperty allows setting of the properties either
   *        directly from other properties or from strings only
   *
   *  This is very convenient in resetting of the default
   *  properties in the derived classes.
   *  E.g. without this method one needs to convert
   *  everything into strings to use IProperty::setProperty
   *
   *  @code
   *
   *    setProperty ( "OutputLevel" , "1"    ) ;
   *    setProperty ( "Enable"      , "True" ) ;
   *    setProperty ( "ErrorMax"    , "10"   ) ;
   *
   *  @endcode
   *
   *  For simple cases it is more or less ok, but for complicated properties
   *  it is just ugly..
   *
   *  @param name      name of the property
   *  @param value     value of the property
   *  @see Gaudi::Utils::setProperty
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date 2007-05-13
   */
  template <class TYPE>
  StatusCode setProperty( const std::string& name, const TYPE& value )
  {
    return Gaudi::Utils::setProperty( this, name, value );
  }
  // ==========================================================================
  /** get the property
   *  @see IProperty
   */
  StatusCode getProperty( Gaudi::Details::PropertyBase* p ) const override
  {
    try {
      const Gaudi::Details::PropertyBase* pp = property( p->name() );
      if ( pp && pp->load( *p ) ) return StatusCode::SUCCESS;
    } catch ( ... ) {
    }
    return StatusCode::FAILURE;
  }
  // ==========================================================================
  /** get the property by name
   *  @see IProperty
   */
  const Gaudi::Details::PropertyBase& getProperty( const std::string& name ) const override
  {
    const Gaudi::Details::PropertyBase* p = property( name );
    if ( !p ) throw std::out_of_range( "Property " + name + " not found." );
    return *p;
  }
  // ==========================================================================
  /** convert the property to the string
   *  @see IProperty
   */
  StatusCode getProperty( const std::string& n, std::string& v ) const override
  {
    // get the property
    const Gaudi::Details::PropertyBase* p = property( n );
    if ( !p ) {
      return StatusCode::FAILURE;
    }
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
  bool hasProperty( const std::string& name ) const override
  {
    return any_of( begin( m_properties ), end( m_properties ), [&name]( const Gaudi::Details::PropertyBase* prop ) {
      return Gaudi::Utils::iequal( prop->name(), name );
    } );
  }
  // ==========================================================================
protected:
  // get local or remote property by name
  Gaudi::Details::PropertyBase* property( const std::string& name ) const
  {
    // local property ?
    Gaudi::Details::PropertyBase* lp = property( name, m_properties );
    if ( lp ) {
      return lp;
    }
    // look for remote property
    for ( const auto& it : m_remoteProperties ) {
      if ( !Gaudi::Utils::iequal( it.first, name ) ) {
        continue;
      }
      const IProperty* p = it.second.first;
      if ( !p ) {
        continue;
      }
      return property( it.second.second, p->getProperties() );
    }
    return nullptr; // RETURN
  }

private:
  /// get the property by name form the proposed list
  Gaudi::Details::PropertyBase* property( const std::string& name,
                                          const std::vector<Gaudi::Details::PropertyBase*>& props ) const
  {
    auto it = std::find_if( props.begin(), props.end(), [&name]( Gaudi::Details::PropertyBase* p ) {
      return p && Gaudi::Utils::iequal( p->name(), name );
    } );
    return ( it != props.end() ) ? *it : nullptr; // RETURN
  }

  /// Issue a runtime warning if the name is already present in the
  /// list of properties (see <a href="https://its.cern.ch/jira/browse/GAUDI-1023">GAUDI-1023</a>).
  void assertUniqueName( const std::string& name ) const
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

  typedef std::vector<Gaudi::Details::PropertyBase*> Properties;
  typedef std::pair<std::string, std::pair<IProperty*, std::string>> RemProperty;
  typedef std::vector<RemProperty> RemoteProperties;

  /// Collection of all declared properties.
  Properties m_properties;
  /// Collection of all declared remote properties.
  RemoteProperties m_remoteProperties;
  /// Properties owned by PropertyHolder, to be deleted.
  std::vector<std::unique_ptr<Gaudi::Details::PropertyBase>> m_todelete;
};
#endif
