#ifndef GAUDIKERNEL_PROPERTYMGR_H
#define GAUDIKERNEL_PROPERTYMGR_H
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

/**
 *  Property manager helper class. This class is used by algorithms and services
 *  for helping to manage its own set of properties. It implements the IProperty
 *  interface.
 *
 *  @author Paul Maley
 *  @author David Quarrie
 *  @author Marco Clemencic
 */
class GAUDI_API PropertyMgr : public virtual IProperty, public virtual INamedInterface
{
private:
  /// Helpers
  /// @{
  /// case insensitive comparison of strings
  constexpr static struct NoCaseCmp_t {
    inline bool operator()( const std::string& v1, const std::string& v2 ) const
    {
      return v1.size() == v2.size() && std::equal( std::begin( v1 ), std::end( v1 ), std::begin( v2 ),
                                                   []( char c1, char c2 ) { return toupper( c1 ) == toupper( c2 ); } );
    }
  } noCaseCmp{};
  /// get the property by name
  struct PropByName {
    std::string m_name;

    inline bool operator()( const Property* p ) const { return p && noCaseCmp( p->name(), m_name ); }
  };
  /// @}
public:
  // copy constructor
  PropertyMgr() = default;
  // copy constructor
  PropertyMgr( const PropertyMgr& ) = delete;
  // assignment operator
  PropertyMgr& operator=( const PropertyMgr& ) = delete;
  /// virtual destructor
  ~PropertyMgr() override = default;

public:
  /// Declare a property (templated)
  template <class TYPE>
  Property* declareProperty( const std::string& name, TYPE& value, const std::string& doc = "none" )
  {
    assertUniqueName( name );
    m_todelete.emplace_back( new SimplePropertyRef<TYPE>( name, value ) );
    Property* p = m_todelete.back().get();
    p->setDocumentation( doc );
    m_properties.push_back( p );
    return p;
  }
  /// Declare a property (specialization)
  inline Property& declareProperty( Property& prop )
  {
    assertUniqueName( prop.name() );
    m_properties.push_back( &prop );
    return prop;
  }
  /// Declare a property (specialization)
  template <class TYPE, class VERIFIER>
  Property* declareProperty( const std::string& name, SimpleProperty<TYPE, VERIFIER>& prop,
                             const std::string& doc = "none" )
  {
    assertUniqueName( name );
    Property* p = &prop;

    p->setName( name );
    p->setDocumentation( doc );
    m_properties.push_back( p );

    return p;
  }
  /// Declare a property (specialization)
  template <class TYPE, class VERIFIER>
  Property* declareProperty( const std::string& name, SimplePropertyRef<TYPE, VERIFIER>& prop,
                             const std::string& doc = "none" )
  {
    assertUniqueName( name );
    Property* p = &prop;

    p->setName( name );
    p->setDocumentation( doc );
    m_properties.push_back( p );

    return p;
  }
  // partial specializations for various GaudiHandles
  /// Declare a property (specialization)
  template <class TYPE>
  Property* declareProperty( const std::string& name, ToolHandle<TYPE>& ref, const std::string& doc = "none" )
  {
    assertUniqueName( name );
    m_todelete.emplace_back( new GaudiHandleProperty( name, ref ) );
    Property* p = m_todelete.back().get();

    p->setDocumentation( doc );
    m_properties.push_back( p );

    return p;
  }
  /// Declare a property (specialization)
  template <class TYPE>
  Property* declareProperty( const std::string& name, ServiceHandle<TYPE>& ref, const std::string& doc = "none" )
  {
    assertUniqueName( name );
    m_todelete.emplace_back( new GaudiHandleProperty( name, ref ) );
    Property* p = m_todelete.back().get();

    p->setDocumentation( doc );
    m_properties.push_back( p );

    return p;
  }
  /// Declare a property (specialization)
  template <class TYPE>
  Property* declareProperty( const std::string& name, ToolHandleArray<TYPE>& ref, const std::string& doc = "none" )
  {
    assertUniqueName( name );
    m_todelete.emplace_back( new GaudiHandleArrayProperty( name, ref ) );
    Property* p = m_todelete.back().get();

    p->setDocumentation( doc );
    m_properties.push_back( p );

    return p;
  }
  /// Declare a property (specialization)
  template <class TYPE>
  Property* declareProperty( const std::string& name, ServiceHandleArray<TYPE>& ref, const std::string& doc = "none" )
  {
    assertUniqueName( name );
    m_todelete.emplace_back( new GaudiHandleArrayProperty( name, ref ) );
    Property* p = m_todelete.back().get();

    p->setDocumentation( doc );
    m_properties.push_back( p );

    return p;
  }
  /// Declare a property (specialization)
  template <class TYPE>
  Property* declareProperty( const std::string& name, DataObjectHandle<TYPE>& ref, const std::string& doc = "none" )
  {
    assertUniqueName( name );
    Property* p = new DataObjectHandleProperty( name, ref );

    p->setDocumentation( doc );
    m_properties.push_back( p );

    return p;
  }
  /// Declare a remote property
  Property* declareRemoteProperty( const std::string& name, IProperty* rsvc, const std::string& rname = "" )
  {
    if ( !rsvc ) {
      return nullptr;
    }
    const std::string& nam = rname.empty() ? name : rname;
    Property* p            = property( nam, rsvc->getProperties() );
    m_remoteProperties.emplace_back( name, std::make_pair( rsvc, nam ) );
    return p;
  }
  // ==========================================================================
  // IProperty implementation
  // ==========================================================================
  /** set the property form another property
   *  @see IProperty
   */
  StatusCode setProperty( const Property& p ) override
  {
    Property* pp = property( p.name() );
    try {
      if ( pp && pp->assign( p ) ) {
        return StatusCode::SUCCESS;
      }
    } catch ( ... ) {
    }
    return StatusCode::FAILURE;
  }
  // ==========================================================================
  /** set the property from the property formatted string
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
    Property* p = property( n );
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
  StatusCode getProperty( Property* p ) const override
  {
    try {
      const Property* pp = property( p->name() );
      if ( pp && pp->load( *p ) ) return StatusCode::SUCCESS;
    } catch ( ... ) {
    }
    return StatusCode::FAILURE;
  }
  // ==========================================================================
  /** get the property by name
   *  @see IProperty
   */
  const Property& getProperty( const std::string& name ) const override
  {
    const Property* p = property( name );
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
    const Property* p = property( n );
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
  const std::vector<Property*>& getProperties() const override { return m_properties; }
  // ==========================================================================
  /** Return true if we have a property with the given name.
   *  @see IProperty
   */
  bool hasProperty( const std::string& name ) const override
  {
    return any_of( begin( m_properties ), end( m_properties ),
                   [&name]( const Property* prop ) { return noCaseCmp( prop->name(), name ); } );
  }
  // ==========================================================================
protected:
  // get local or remote property by name
  Property* property( const std::string& name ) const
  {
    // local property ?
    Property* lp = property( name, m_properties );
    if ( lp ) {
      return lp;
    }
    // look for remote property
    for ( const auto& it : m_remoteProperties ) {
      if ( !noCaseCmp( it.first, name ) ) {
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
  Property* property( const std::string& name, const std::vector<Property*>& props ) const
  {
    auto it = std::find_if( props.begin(), props.end(), PropByName{name} );
    return ( it != props.end() ) ? *it : nullptr; // RETURN
  }

  /// Throw an exception if the name is already present in the
  /// list of properties (see GAUDI-1023).
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

  // Some typedef to simply typing
  typedef std::vector<Property*> Properties;
  typedef std::pair<std::string, std::pair<IProperty*, std::string>> RemProperty;
  typedef std::vector<RemProperty> RemoteProperties;

  /// Collection of all declared properties
  Properties m_properties; // local  properties
  /// Collection of all declared remote properties
  RemoteProperties m_remoteProperties; // Remote properties
  /// Properties to be deleted
  std::vector<std::unique_ptr<Property>> m_todelete; // properties to be deleted
};
#endif
