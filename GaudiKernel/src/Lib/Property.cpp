// $Id: Property.cpp,v 1.24 2007/09/25 10:31:43 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, verison $Revision: 1.24 $
// ============================================================================
// Include files
// ============================================================================
// STD & STL
// ============================================================================
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <utility>
#include <map>
#include <algorithm>
#include <functional>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/GaudiHandle.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/algorithm/string/compare.hpp"
// ============================================================================
/** @file
 *  The implementation file for the class Property ans related classes
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2006-02-27
 */
// ============================================================================
// The output operator for friendly printout
// ============================================================================
std::ostream& operator<<( std::ostream&   stream ,
                          const Property& prop   )
{ return prop.fillStream ( stream ) ; }
// ============================================================================
/*  constructor from the property name and the type
 *  @param name proeprty name
 *  @param type property C++/RTTI type
 */
// ============================================================================
Property::Property
( const std::type_info&  type ,
  std::string     name )
  : m_name            ( std::move(name) )
  , m_documentation   ( m_name )
  , m_typeinfo        ( &type )
{}
// ============================================================================
/*  constructor from the property name and the type
 *  @param type property C++/RTTI type
 *  @param name proeprty name
 */
// ============================================================================
Property::Property
( std::string     name ,
  const std::type_info&  type )
  : m_name            ( std::move(name) )
  , m_documentation   ( m_name )
  , m_typeinfo        ( &type )
{}
// ============================================================================
// set new callback for reading
// ============================================================================
void  Property::declareReadHandler( std::function<void(Property&)> fun )
{
  m_readCallBack = std::move(fun);
}
// ============================================================================
// set new callback for update
// ============================================================================
void  Property::declareUpdateHandler ( std::function<void(Property&)> fun )
{
  m_updateCallBack = std::move(fun);
}
// ============================================================================
// use the call-back function at reading
// ============================================================================
void Property::useReadHandler   () const
{
  if ( !m_readCallBack ) { return ; }               // RETURN
  const Property& p = *this ;
  // avoid infinite loop
  std::function<void(Property&)> theCallBack;
  theCallBack.swap(m_readCallBack);
  theCallBack( const_cast<Property&>(p) ) ;
  m_readCallBack.swap(theCallBack);
}
// ============================================================================
// use the call-back function at update
// ============================================================================
bool Property::useUpdateHandler ()
{
  bool sc(true);
  if ( !m_updateCallBack ) { return sc; }  // RETURN
  // avoid infinite loop
  std::function<void(Property&)> theCallBack;
  theCallBack.swap(m_updateCallBack);
  try {
    theCallBack( *this ) ;
  } catch(...) {
    sc = false;
  }
  m_updateCallBack.swap(theCallBack);
  return sc;
}
// ============================================================================
// the printout of the property value
// ============================================================================
std::ostream&
Property::fillStream ( std::ostream& stream ) const
{ return stream << " '" <<name() << "':" << toString() ; }
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
bool Gaudi::Utils::hasProperty
( const IInterface*  p    ,
  const std::string& name )
{
  // trivial check
  if ( !p ) { return false ; }                                // RETURN
  // gelegate to another method
  return getProperty ( p , name ) ;
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
bool Gaudi::Utils::hasProperty
( const IProperty*   p    ,
  const std::string& name )
{
  if ( !p ) { return false ; }
  // delegate the actual work to another method ;
  return getProperty ( p , name ) ;
}
// ============================================================================
//
// GaudiHandleProperty implementation
//
GaudiHandleProperty::GaudiHandleProperty
( const std::string& name, GaudiHandleBase& ref )
  : Property( name, typeid( GaudiHandleBase ) ), m_pValue( &ref )
{
  m_pValue->setPropertyName( name );
}

bool GaudiHandleProperty::setValue( const GaudiHandleBase& value ) {
  m_pValue->setTypeAndName( value.typeAndName() );
  return useUpdateHandler();
}

std::string GaudiHandleProperty::toString( ) const {
  useReadHandler();
  return m_pValue->typeAndName();
}

void GaudiHandleProperty::toStream(std::ostream& out) const {
  useReadHandler();
  out << m_pValue->typeAndName();
}

StatusCode GaudiHandleProperty::fromString( const std::string& s) {
  m_pValue->setTypeAndName( s );
  return useUpdateHandler()?StatusCode::SUCCESS:StatusCode::FAILURE;
}


//
// GaudiHandlePropertyArray implementation
//
GaudiHandleArrayProperty::GaudiHandleArrayProperty( const std::string& name, GaudiHandleArrayBase& ref )
  : Property( name, typeid( GaudiHandleArrayBase ) ), m_pValue( &ref )
{
  m_pValue->setPropertyName( name );
}

bool GaudiHandleArrayProperty::setValue( const GaudiHandleArrayBase& value ) {
  m_pValue->setTypesAndNames( value.typesAndNames() );
  return useUpdateHandler();
}

std::string GaudiHandleArrayProperty::toString() const {
  // treat as if a StringArrayProperty
  useReadHandler();
  return Gaudi::Utils::toString( m_pValue->typesAndNames() );
}

void GaudiHandleArrayProperty::toStream(std::ostream &out) const {
  // treat as if a StringArrayProperty
  useReadHandler();
  Gaudi::Utils::toStream( m_pValue->typesAndNames(), out );
}

StatusCode GaudiHandleArrayProperty::fromString( const std::string& source ) {
  // treat as if a StringArrayProperty
  std::vector< std::string > tmp;
  StatusCode sc = Gaudi::Parsers::parse ( tmp , source );
  if ( sc.isFailure() ) return sc;
  if ( !m_pValue->setTypesAndNames( std::move(tmp) ) ) return StatusCode::FAILURE;
  return useUpdateHandler()?StatusCode::SUCCESS:StatusCode::FAILURE;
}



// ============================================================================
namespace
{
  template <typename C, typename BinaryPredicate>
  bool equal_(const C& c1,  const C& c2, BinaryPredicate&& p) {
    return c1.size() == c2.size() &&
           std::equal(std::begin(c1), std::end(c1), std::begin(c2),
                      std::forward<BinaryPredicate>(p) );
  }

  // match (case insensitive) property by name
  struct is_iByName
  {
    /// constructor from name
    is_iByName ( const std::string& name ) : m_name ( name ) {}
    /// the most essential method:
    bool operator () ( const Property* p ) const
    {
      return p && equal_(m_name,p->name(),boost::algorithm::is_iequal{});
    } ;
  private:
    const std::string& m_name ;
  } ;
}
// ============================================================================
/*  simple function which gets the property with given name
 *  from the component
 *
 *  @code
 *
 *  const IProperty* p = ... ;
 *
 *  const Property* pro = getProperty( p , "Context" ) ;
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
Property* Gaudi::Utils::getProperty
( const IProperty*   p    ,
  const std::string& name )
{
  // trivial check
  if ( !p      ) { return nullptr ; }                          // RETURN
  // get all properties
  const auto& props = p->getProperties() ;
  // comparison criteria:
  auto ifound = std::find_if ( props.begin() , props.end() , is_iByName{ name } ) ;
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
 *  const Property* pro = getProperty( p , "Context" ) ;
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
Property* Gaudi::Utils::getProperty
( const IInterface*   p , const std::string& name )
{
  // trivial check
  if ( !p        ) { return nullptr ; }                                // RETURN
  // remove const-qualifier
  IInterface* _i = const_cast<IInterface*>( p ) ;
  if ( !_i        ) { return nullptr ; }                                // RETURN
  SmartIF<IProperty> property ( _i ) ;
  return property ? getProperty ( property , name ) : nullptr;
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
bool Gaudi::Utils::hasProperty
( const std::vector<const Property*>* p    ,
  const std::string&                  name )
{
  // delegate to another method
  return getProperty ( p , name ) ;
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
 *  const Property* context =
 *      getProperty ( svc->getProperties( client ) , "Context" )
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
const Property* Gaudi::Utils::getProperty
( const std::vector<const Property*>* p    ,
  const std::string&                  name )
{
  // trivial check
  if ( !p             ) { return nullptr ; }                 // RETURN
  auto ifound = std::find_if ( p->begin() , p->end() , is_iByName{ name } ) ;
  return p->end() != ifound ? *ifound : nullptr ; // RETURN
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
StatusCode Gaudi::Utils::setProperty
( IProperty*         component ,
  const std::string& name      ,
  const char*        value     ,
  const std::string& doc       )
{
  return Gaudi::Utils::setProperty ( component , name , std::string{value} , doc ) ;
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
StatusCode Gaudi::Utils::setProperty
( IProperty*         component ,
  const std::string& name      ,
  const std::string& value     ,
  const std::string& doc       )
{
  if ( !component ) { return StatusCode::FAILURE ; }   // RETURN
  if ( !hasProperty ( component , name ) ) { return StatusCode::FAILURE ; }
  StatusCode sc = component -> setProperty ( name , value ) ;
  if ( !doc.empty() )
  {
    Property* p = getProperty( component , name ) ;
    if ( p ) { p -> setDocumentation ( doc ) ; }
  }
  sc.ignore() ;
  return sc ;
}
// ============================================================================
/*  simple function to set the property of the given object from another
 *  property
 *
 *  @code
 *
 *  IProperty* component = ... ;
 *
 *  const Property* prop = ... ;
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
StatusCode Gaudi::Utils::setProperty
( IProperty*         component ,
  const std::string& name      ,
  const Property*    property  ,
  const std::string& doc       )
{
  if ( !component || !property || !hasProperty ( component , name )  ) { return StatusCode::FAILURE ; }
  Property* p = getProperty ( component , name ) ;
  if ( !p || !p->assign ( *property )                                ) { return StatusCode::FAILURE ; }
  if ( !doc.empty()  ) { p->setDocumentation( doc ) ; }
  return StatusCode::SUCCESS ;
}
// ============================================================================
/* simple function to set the property of the given object from another
 *  property
 *
 *  @code
 *
 *  IProperty* component = ... ;
 *
 *  const Property& prop = ... ;
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
StatusCode Gaudi::Utils::setProperty
( IProperty*         component ,
  const std::string& name      ,
  const Property&    property  ,
  const std::string& doc       )
{ return setProperty ( component , name , &property , doc ) ; }
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
StatusCode Gaudi::Utils::setProperty
( IInterface*        component ,
  const std::string& name      ,
  const std::string& value     ,
  const std::string& doc       )
{
  if ( !component ) { return StatusCode::FAILURE ; }
  SmartIF<IProperty> property ( component ) ;
  if ( !property  ) { return StatusCode::FAILURE ; }
  return setProperty ( property , name , value , doc ) ;
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
StatusCode Gaudi::Utils::setProperty
( IInterface*        component ,
  const std::string& name      ,
  const char*        value     ,
  const std::string& doc       )
{
  return setProperty ( component , name , std::string{ value } , doc ) ;
}
// ============================================================================
/*  simple function to set the property of the given object from another
 *  property
 *
 *  @code
 *
 *  IInterface* component = ... ;
 *
 *  const Property* prop = ... ;
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
StatusCode Gaudi::Utils::setProperty
( IInterface*        component ,
  const std::string& name      ,
  const Property*    property  ,
  const std::string& doc       )
{
  if ( !component ) { return StatusCode::FAILURE ; }
  SmartIF<IProperty> prop  ( component ) ;
  if ( !prop          ) { return StatusCode::FAILURE ; }
  return setProperty ( prop  , name , property , doc ) ;
}
// ============================================================================
/*  simple function to set the property of the given object from another
 *  property
 *
 *  @code
 *
 *  IInterface* component = ... ;
 *
 *  const Property& prop = ... ;
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
StatusCode Gaudi::Utils::setProperty
( IInterface*        component ,
  const std::string& name      ,
  const Property&    property  ,
  const std::string& doc       )
{ return setProperty ( component , name , &property , doc ) ; }
// ============================================================================

// ============================================================================
// The END
// ============================================================================
