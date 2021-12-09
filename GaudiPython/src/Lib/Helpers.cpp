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
// Python must always be the first.
#include "Python.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/SmartIF.h"
#include <Gaudi/Property.h>
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
DataObject* GaudiPython::Helper::findobject( IDataProviderSvc* dpsvc, const std::string& path ) {
  DataObject* o = nullptr;
  if ( !dpsvc ) { return nullptr; }             // RETURN
  StatusCode sc = dpsvc->findObject( path, o ); // NB!
  return sc.isSuccess() ? o : nullptr;          // RETURN
}
// ===========================================================================
/// Anonymous namespace to hide local class
namespace {
  // ==========================================================================
  /** the property of DataService
   *  @see DataSvc
   */
  const std::string s_NAME = "EnableFaultHandler";
  // ======================================================================
  class Disabler {
  public:
    // =========================================================================
    /// constructor from the interface  and flag
    Disabler( IInterface* svc, bool disable ) : m_svc( svc ), m_old( s_NAME, true ), m_enable( !disable ) {
      if ( !m_svc ) {
        m_code = StatusCode::FAILURE;
      } else if ( m_enable ) { /* no action here!! */
        ;
      } // No action!
      else {
        const Gaudi::Details::PropertyBase* property = Gaudi::Utils::getProperty( m_svc.get(), s_NAME );
        if ( !property || !m_old.assign( *property ) ) {
          m_code = StatusCode::FAILURE;
        } else if ( m_old.value() != m_enable ) {
          m_code = Gaudi::Utils::setProperty( m_svc.get(), s_NAME, m_enable );
        }
      }
    }
    // =========================================================================
    /// destructor: restore the property
    ~Disabler() {
      if ( m_enable ) { /* no action here! */
      }                 // no action here
      else if ( code().isSuccess() && m_old.value() != m_enable ) {
        m_code = Gaudi::Utils::setProperty( m_svc.get(), s_NAME, m_old );
      }
      m_code.ignore();
    }
    // ========================================================================
    StatusCode code() const { return m_enable ? StatusCode::SUCCESS : m_code; }
    // ========================================================================
  private:
    // ========================================================================
    /// the property interface
    SmartIF<IProperty>    m_svc; // the property interface
    Gaudi::Property<bool> m_old = { s_NAME, true };
    bool                  m_enable;
    StatusCode            m_code = StatusCode::SUCCESS; // status code
    // ========================================================================
  };
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
DataObject* GaudiPython::Helper::getobject( IDataProviderSvc* dpsvc, const std::string& path, const bool retrieve,
                                            const bool disable ) {
  if ( !dpsvc ) { return nullptr; } // RETURN 0
  // create the sentry:
  Disabler sentry( dpsvc, disable );
  //
  DataObject* result = nullptr;
  //
  StatusCode sc = retrieve ? dpsvc->retrieveObject( path, result ) : dpsvc->findObject( path, result );
  //
  return sc.isSuccess() ? result : nullptr;
}
// ============================================================================
// The END
// ============================================================================
