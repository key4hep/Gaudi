#ifndef GAUDIKERNEL_PROPERTYMGR_H
#define GAUDIKERNEL_PROPERTYMGR_H
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <iostream>
#include <vector>
#include <utility>
#include <stdexcept>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Property.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/DataObjectHandleProperty.h"

// ============================================================================

// pre-declaration of GaudiHandles is sufficient
template< class T> class ToolHandle;
template< class T> class ServiceHandle;
template< class T> class ToolHandleArray;
template< class T> class ServiceHandleArray;
template< class T> class DataObjectHandle;

/** @class PropertyMgr PropertyMgr.h GaudiKernel/PropertyMgr.h
 *
 *  Property manager helper class. This class is used by algorithms and services
 *  for helping to manage its own set of properties. It implements the IProperty
 *  interface.
 *
 *  @author Paul Maley
 *  @author David Quarrie
 *
 *  \deprecated{will be removed in v28r1, consider using PropertyHolder instead}
 */
class GAUDI_API
[[deprecated("will be removed in v28r1, consider using PropertyHolder instead")]]
PropertyMgr : public implements<IProperty>
{
public:
  /// constructor from the interface
  PropertyMgr ( IInterface* iface = nullptr );
  // copy constructor
  PropertyMgr ( const PropertyMgr& )  = delete;
  // assignment operator
  PropertyMgr& operator=( const PropertyMgr& ) = delete;
  /// virtual destructor
  ~PropertyMgr() override = default;
public:
  /// Declare a property (templated)
  template<class TYPE>
  Property* declareProperty
  ( const std::string&       name  ,
    TYPE&                    value,
    const std::string&       doc = "none" ) ;
  /// Declare a property (specialization)
  template <class TYPE>
  Property* declareProperty
  ( const std::string&       name  ,
    SimpleProperty<TYPE>&    prop,
    const std::string&       doc = "none") ;
  /// Declare a property (specialization)
  template <class TYPE>
  Property* declareProperty
  ( const std::string&       name  ,
    SimplePropertyRef<TYPE>& prop,
    const std::string&       doc = "none") ;
  // partial specializations for various GaudiHandles
  /// Declare a property (specialization)
  template<class TYPE>
  Property* declareProperty
  ( const std::string& name,
    ToolHandle<TYPE>& ref,
    const std::string& doc = "none" ) ;
  /// Declare a property (specialization)
  template<class TYPE>
  Property* declareProperty
  ( const std::string& name,
    ServiceHandle<TYPE>& ref,
    const std::string& doc = "none" ) ;
  /// Declare a property (specialization)
  template<class TYPE>
  Property* declareProperty
  ( const std::string& name,
    ToolHandleArray<TYPE>& ref,
    const std::string& doc = "none" ) ;
  /// Declare a property (specialization)
  template<class TYPE>
  Property* declareProperty
  ( const std::string& name,
    ServiceHandleArray<TYPE>& ref,
    const std::string& doc = "none" ) ;
  /// Declare a property (specialization)
   template<class TYPE>
  Property* declareProperty
  ( const std::string& name,
     DataObjectHandle<TYPE>& ref,
    const std::string& doc = "none" ) ;
  /// Declare a remote property
  Property* declareRemoteProperty
  ( const std::string& name       ,
    IProperty*         rsvc       ,
    const std::string& rname = "" ) ;
  // ==========================================================================
  // IProperty implementation
  // ==========================================================================
  /** set the property form another property
   *  @see IProperty
   */
  StatusCode setProperty(const Property& p) override;
  // ==========================================================================
  /** set the property from the property formatted string
   *  @see IProperty
   */
  StatusCode setProperty( const std::string& s ) override;
  // ==========================================================================
  /** set the property from name and the value
   *  @see IProperty
   */
  StatusCode setProperty( const std::string& n, const std::string& v) override;
  // ==========================================================================
  /** get the property
   *  @see IProperty
   */
  StatusCode getProperty(Property* p) const override;
  // ==========================================================================
  /** get the property by name
   *  @see IProperty
   */
  const Property& getProperty( const std::string& name) const override;
  // ==========================================================================
  /** convert the property to the string
   *  @see IProperty
   */
  StatusCode getProperty( const std::string& n, std::string& v ) const override;
  // ==========================================================================
  /** get all properties
   *  @see IProperty
   */
  const std::vector<Property*>& getProperties( ) const override;
  // ==========================================================================
  /** Return true if we have a property with the given name.
   *  @see IProperty
   */
  bool hasProperty(const std::string& name) const override;
  // ==========================================================================
  // IInterface implementation
  StatusCode queryInterface(const InterfaceID& iid, void** pinterface) override;
  // ==========================================================================
protected:

  // get local or remote property by name
  Property* property       ( const std::string& name  ) const ;

private:

  /// get the property by name form the proposed list
  Property* property
  ( const std::string&             name  ,
    const std::vector<Property*>&  props ) const ;

  /// Throw an exception if the name is already present in the
  /// list of properties (see GAUDI-1023).
  void assertUniqueName(const std::string& name) const;

  // Some typedef to simply typing
  typedef std::vector<Property*>   Properties       ;
  typedef std::pair<std::string,
                    std::pair<IProperty*, std::string> > RemProperty;
  typedef std::vector<RemProperty> RemoteProperties ;

  /// Collection of all declared properties
  Properties             m_properties      ;  // local  properties
  /// Collection of all declared remote properties
  RemoteProperties       m_remoteProperties;  // Remote properties
  /// Properties to be deleted
  std::vector<std::unique_ptr<Property>> m_todelete ;  // properties to be deleted
  /// Interface hub reference (ApplicationMgr)
  IInterface*            m_pOuter  ;  // Interface hub reference
};
// ============================================================================
/// Declare a property (templated)
// ============================================================================
template<class TYPE>
inline Property*
PropertyMgr::declareProperty
( const std::string& name  ,
  TYPE&              value,
  const std::string& doc )
{
  assertUniqueName(name);
  m_todelete.emplace_back( new SimplePropertyRef<TYPE> ( name , value ) );
  Property* p = m_todelete.back().get();
  //
  p->setDocumentation( doc );
  m_properties .push_back( p ) ;
  //
  return p ;
}
// ============================================================================
/// Declare a property (templated)
// ============================================================================
template <class TYPE>
inline Property*
PropertyMgr::declareProperty
( const std::string&       name ,
  SimpleProperty<TYPE>&    prop,
  const std::string&       doc )
{
  assertUniqueName(name);
  Property* p = &prop ;
  //
  p -> setName           ( name  ) ;
  p -> setDocumentation  ( doc   ) ;
  m_properties.push_back ( p     ) ;
  //
  return p ;
}
// ============================================================================
/// Declare a property (templated)
// ============================================================================
template <class TYPE>
inline Property*
PropertyMgr::declareProperty
( const std::string&       name ,
  SimplePropertyRef<TYPE>& prop,
  const std::string&       doc )
{
  assertUniqueName(name);
  Property* p = &prop ;
  //
  p -> setName             ( name  ) ;
  p -> setDocumentation    ( doc   ) ;
  m_properties.push_back   ( p     ) ;
  //
  return p ;
}
// ============================================================================
// Declare a property (templated)
// ============================================================================
template<class TYPE>
inline Property*
PropertyMgr::declareProperty
( const std::string& name,
  ToolHandle<TYPE>& ref,
  const std::string& doc )
{
  assertUniqueName(name);
  m_todelete   . emplace_back ( new GaudiHandleProperty( name, ref ) );
  Property* p = m_todelete.back().get();
  //
  p -> setDocumentation    ( doc ) ;
  m_properties . push_back ( p   ) ;
  //
  return p ;
}
// ============================================================================
template<class TYPE>
inline Property*
PropertyMgr::declareProperty
( const std::string& name,
  ServiceHandle<TYPE>& ref,
  const std::string& doc )
{
  assertUniqueName(name);
  m_todelete   . emplace_back (new GaudiHandleProperty( name, ref ));
  Property* p = m_todelete.back().get();
  //
  p -> setDocumentation    ( doc ) ;
  m_properties . push_back ( p   ) ;
  //
  return p ;
}
// ============================================================================
template<class TYPE>
inline Property*
PropertyMgr::declareProperty
( const std::string& name,
  ToolHandleArray<TYPE>& ref,
  const std::string& doc )
{
  assertUniqueName(name);
  m_todelete   . emplace_back ( new GaudiHandleArrayProperty( name, ref ) );
  Property* p = m_todelete.back().get();
  //
  p -> setDocumentation    ( doc ) ;
  m_properties . push_back ( p   ) ;
  //
  return p ;
}
// ============================================================================
template<class TYPE>
inline Property*
PropertyMgr::declareProperty
( const std::string& name,
  ServiceHandleArray<TYPE>& ref,
  const std::string& doc )
{
  assertUniqueName(name);
  m_todelete   . emplace_back ( new GaudiHandleArrayProperty( name, ref ) );
  Property* p = m_todelete.back().get();
  //
  p -> setDocumentation    ( doc ) ;
  m_properties . push_back ( p   ) ;
  //
  return p ;
}

// ============================================================================
template<class TYPE>
inline Property*
PropertyMgr::declareProperty
( const std::string& name,
  DataObjectHandle<TYPE>& ref,
  const std::string& doc )
{
  assertUniqueName(name);
  Property* p = new DataObjectHandleProperty( name, ref );
  //
  p -> setDocumentation    ( doc ) ;
  m_properties . push_back ( p   ) ;
  //
  return p ;
}

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_PROPERTYMGR_H
// ============================================================================
