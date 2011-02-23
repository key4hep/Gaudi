// $Id: PropertyMgr.h,v 1.22 2008/04/03 17:27:01 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.22 $
// ============================================================================
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
// ============================================================================

// pre-declaration of GaudiHandles is sufficient
template< class T> class ToolHandle;
template< class T> class ServiceHandle;
template< class T> class ToolHandleArray;
template< class T> class ServiceHandleArray;

/** @class PropertyMgr PropertyMgr.h GaudiKernel/PropertyMgr.h
 *
 *  Property manager helper class. This class is used by algorithms and services
 *  for helping to manage its own set of properties. It implements the IProperty
 *  interface.
 *
 *  @author Paul Maley
 *  @author David Quarrie
 */
class GAUDI_API PropertyMgr : public implements1<IProperty>
{
public:
  /// constructor from the interface
  PropertyMgr ( IInterface* iface = 0 );
  // copy constructor
  PropertyMgr ( const PropertyMgr& ) ;
  /// virtual destructor
  virtual ~PropertyMgr();
  // assignment operator
  PropertyMgr& operator=( const PropertyMgr& ) ;
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
	StatusCode setProperty(const Property& p);
  // ==========================================================================
  /** set the property from the property formatted string
   *  @see IProperty
   */
  StatusCode setProperty( const std::string& s );
  // ==========================================================================
  /** set the property from name and the value
   *  @see IProperty
   */
  StatusCode setProperty( const std::string& n, const std::string& v);
  // ==========================================================================
  /** get the property
   *  @see IProperty
   */
	StatusCode getProperty(Property* p) const;
  // ==========================================================================
  /** get the property by name
   *  @see IProperty
   */
  const Property& getProperty( const std::string& name) const;
  // ==========================================================================
  /** convert the property to the string
   *  @see IProperty
   */
  StatusCode getProperty( const std::string& n, std::string& v ) const;
  // ==========================================================================
  /** get all properties
   *  @see IProperty
   */
  const std::vector<Property*>& getProperties( ) const;
  // ==========================================================================
  // IInterface implementation
  StatusCode queryInterface(const InterfaceID& iid, void** pinterface);
  // ==========================================================================
protected:

  // get local or remote property by name
  Property* property       ( const std::string& name  ) const ;

private:
  /// get the property by name form the proposed list
  Property* property
  ( const std::string&             name  ,
    const std::vector<Property*>&  props ) const ;

private:

  // Some typedef to simply typing
  typedef std::vector<Property*>   Properties       ;
  typedef std::pair<std::string,
                    std::pair<IProperty*, std::string> > RemProperty;
  typedef std::vector<RemProperty> RemoteProperties ;

private:

  /// Collection of all declared properties
  Properties             m_properties      ;  // local  properties
  /// Collection of all declared remote properties
  RemoteProperties       m_remoteProperties;  // Remote properties
  /// Properties to be deleted
  Properties             m_todelete        ;  // properties to be deleted
  ///  Flag to decide to delete or not a propertyRef
  std::vector<bool>      m_isOwned         ;  // flag to delete
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
  Property* p = new SimplePropertyRef<TYPE> ( name , value ) ;
  //
  p->setDocumentation( doc );
  m_properties .push_back( p ) ;
  m_todelete   .push_back( p ) ;
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
  Property* p = new GaudiHandleProperty( name, ref );
  //
  p -> setDocumentation    ( doc ) ;
  m_properties . push_back ( p   ) ;
  m_todelete   . push_back ( p   ) ;
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
  Property* p = new GaudiHandleProperty( name, ref );
  //
  p -> setDocumentation    ( doc ) ;
  m_properties . push_back ( p   ) ;
  m_todelete   . push_back ( p   ) ;
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
  Property* p = new GaudiHandleArrayProperty( name, ref );
  //
  p -> setDocumentation    ( doc ) ;
  m_properties . push_back ( p   ) ;
  m_todelete   . push_back ( p   ) ;
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
  Property* p = new GaudiHandleArrayProperty( name, ref );
  //
  p -> setDocumentation    ( doc ) ;
  m_properties . push_back ( p   ) ;
  m_todelete   . push_back ( p   ) ;
  //
  return p ;
}

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIKERNEL_PROPERTYMGR_H
// ============================================================================

