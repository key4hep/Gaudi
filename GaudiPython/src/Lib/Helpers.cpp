// Python must always be the first.
#include "Python.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/DataObject.h"
// ============================================================================
// GaudiPython
// ============================================================================
#include "GaudiPython/Helpers.h"
// ============================================================================
/** @file
 *  Implementation file for some functions from class GaudiPython::Helpers
 *  @see GaudiPython::Helpers
 *  @see GaudiPython::Helpers::findobject
 *  @see GaudiPython::Helpers::getobject
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-10-09
 */
// ============================================================================
/*  Simple wrapper for IDataProviderSvc::findObject
 *  The method does NOT trigger the loading
 *  the object from tape or Data-On-Demand action
 *  @see IDataProviderSvc
 *  @see IDataProviderSvc::findObject
 *  @param dpsvc (INPUT) pointer to Data Provider Service
 *  @param oath full path in TES
 *  @return the object
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-10-09
 */
// ===========================================================================
DataObject* GaudiPython::Helper::findobject
( IDataProviderSvc*  dpsvc ,
  const std::string& path  )
{
  DataObject* o = 0 ;
  if ( 0 == dpsvc     ) { return 0 ; }                              // RETURN
  StatusCode sc = dpsvc -> findObject ( path , o ) ;  // NB!
  if ( sc.isFailure() ) { return 0 ; }                              // RETURN
  return o ;                                                        // RETURN
}
// ===========================================================================
/// Anonymous namespace to hide local class
namespace
{
  // ==========================================================================
  /** the property of DataService
   *  @see DataSvc
   */
  const std::string s_NAME = "EnableFaultHandler" ;
  // ======================================================================
  class Disabler
  {
  public:
    // =========================================================================
    /// constructor from the interface  and flag
    Disabler ( IInterface* svc     ,
               const bool  disable )
      : m_svc     ( svc     )
      , m_old     (  s_NAME , true      )
      , m_enable  ( !disable            )
      , m_code    ( StatusCode::SUCCESS )
    {
      if      ( !m_svc   ) { m_code = StatusCode::FAILURE ; }
      else if ( m_enable ) { /* no action here!! */    ; }  // No action!
      else
      {
        const Property* property =
          Gaudi::Utils::getProperty ( m_svc.get() , s_NAME ) ;
        if ( 0 == property || !m_old.assign ( *property ) )
        { m_code = StatusCode::FAILURE ; }
        else if ( m_old.value() != m_enable )
        {
          m_code = Gaudi::Utils::setProperty
            ( m_svc.get() , s_NAME , m_enable ) ;
        }
      }
    }
    // =========================================================================
    /// destructor: restore the property
    ~Disabler()
    {
      if      ( m_enable ) { /* no action here! */ } // no action here
      else if ( code().isSuccess() && m_old.value() != m_enable ) {
        // This line results in an ambiguous overload resolution on g++ 3.4
        //   m_code = Gaudi::Utils::setProperty ( m_svc.get() , s_NAME , m_old );
        // The problem is that m_old could be any of:
        //  - const TYPE &
        //  - const Property &
        //  - const SimpleProperty<TYPE, BoundedVerifier<TYPE> >&
        // So we force the the template argument to help the compiler
        m_code = Gaudi::Utils::setProperty<bool>(m_svc.get(), s_NAME, m_old);
      }
      m_code.ignore() ;
    }
    // ========================================================================
    StatusCode code () const
    {
      if ( m_enable ) { return StatusCode::SUCCESS ; }
      return m_code ;
    }
    // ========================================================================
  private:
    // ========================================================================
    /// the property interface
    SmartIF<IProperty> m_svc      ; // the property interface
    BooleanProperty    m_old      ;
    bool               m_enable   ;
    StatusCode         m_code     ; // status code
    // ========================================================================
  } ;
  // ==========================================================================
} //                                                 end of anonymous namespace
// ===========================================================================
/*  the generic function to get object from TES
 *  @see IDataProviderSvc
 *  @see IDataProviderSvc::findObject
 *  @see IDataProviderSvc::retriveObject
 *  @param dpsvc (INPUT) pointer to Data Provider Service
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2009-10-09
 */
// ===========================================================================
DataObject* GaudiPython::Helper::getobject
( IDataProviderSvc*  dpsvc    ,
  const std::string& path     ,
  const bool         retrieve ,
  const bool         disable  )
{
  if ( 0 == dpsvc ) { return 0 ; }  // RETURN 0
  // create the sentry:
  Disabler sentry ( dpsvc , disable ) ;
  //
  DataObject * result = 0 ;
  //
  StatusCode sc =
    retrieve ?
    dpsvc -> retrieveObject ( path , result ) :
    dpsvc -> findObject     ( path , result ) ;
  //
  if ( sc.isFailure() ) { return 0 ; }                               // RETURN
  //
  return result ;                                                    // RETURN
}
// ============================================================================
// The END
// ============================================================================
