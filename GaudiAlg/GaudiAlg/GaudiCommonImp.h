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
#ifndef GAUDIALG_GAUDICOMMONIMP_H
#define GAUDIALG_GAUDICOMMONIMP_H 1
// ============================================================================
// Include files
// ============================================================================
#include <algorithm>
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiCommon.h"
#include "GaudiAlg/GetData.h"
// ============================================================================
/** @file
 *  The implementation of templated methods for class GaudiCommon
 *  @see    GaudiCommon
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2004-01-19
 */
// ============================================================================
// Templated access to the data in Gaudi Transient Store
// ============================================================================
template <class PBASE>
template <class TYPE>
typename Gaudi::Utils::GetData<TYPE>::return_type
GaudiCommon<PBASE>::get( IDataProviderSvc* service, const std::string& location, const bool useRootInTES ) const {
  // check the environment
  Assert( service, "get():: IDataProvider* points to NULL!" );
  // get the helper object:
  Gaudi::Utils::GetData<TYPE> getter;
  return getter( *this, service, this->fullTESLocation( location, useRootInTES ) );
}
// ============================================================================
// Templated access to the data in Gaudi Transient Store, no check on the content
// ============================================================================
template <class PBASE>
template <class TYPE>
typename Gaudi::Utils::GetData<TYPE>::return_type
GaudiCommon<PBASE>::getIfExists( IDataProviderSvc* service, const std::string& location,
                                 const bool useRootInTES ) const {
  // check the environment
  Assert( service, "get():: IDataProvider* points to NULL!" );
  // get the helper object:
  Gaudi::Utils::GetData<TYPE> getter;
  return getter( *this, service, this->fullTESLocation( location, useRootInTES ), false );
}
// ============================================================================
// check the existence of objects in Gaudi Transient Store
// ============================================================================
template <class PBASE>
template <class TYPE>
bool GaudiCommon<PBASE>::exist( IDataProviderSvc* service, const std::string& location,
                                       const bool useRootInTES ) const {
  // check the environment
  Assert( service, "exist():: IDataProvider* points to NULL!" );
  // check the data object
  Gaudi::Utils::CheckData<TYPE> checker;
  return checker( service, this->fullTESLocation( location, useRootInTES ) );
}
// ============================================================================
// get the existing object from Gaudi Event Transient store
// or create new object register in in TES and return if object
// does not exist
// ============================================================================
template <class PBASE>
template <class TYPE, class TYPE2>
typename Gaudi::Utils::GetData<TYPE>::return_type
GaudiCommon<PBASE>::getOrCreate( IDataProviderSvc* service, const std::string& location,
                                 const bool useRootInTES ) const {
  // check the environment
  Assert( service, "getOrCreate():: svc points to NULL!" );
  // get the helper object
  Gaudi::Utils::GetOrCreateData<TYPE, TYPE2> getter;
  return getter( *this, service, this->fullTESLocation( location, useRootInTES ), location );
}
// ============================================================================
// the useful method for location of tools.
// ============================================================================
template <class PBASE>
template <class TOOL>
TOOL* GaudiCommon<PBASE>::tool( const std::string& type, const std::string& name, const IInterface* parent,
                                       bool create ) const {
  // for empty names delegate to another method
  if ( name.empty() ) return tool<TOOL>( type, parent, create );
  Assert( this->toolSvc(), "tool():: IToolSvc* points to NULL!" );
  // get the tool from Tool Service
  TOOL*            Tool = nullptr;
  const StatusCode sc   = this->toolSvc()->retrieveTool( type, name, Tool, parent, create );
  if ( sc.isFailure() ) { Exception( "tool():: Could not retrieve Tool '" + type + "'/'" + name + "'", sc ); }
  if ( !Tool ) { Exception( "tool():: Could not retrieve Tool '" + type + "'/'" + name + "'" ); }
  // insert tool into list of tools
  PBASE::registerTool( Tool );
  m_managedTools.push_back( Tool );
  // return *VALID* located tool
  return Tool;
}
// ============================================================================
// the useful method for location of tools.
// ============================================================================
template <class PBASE>
template <class TOOL>
TOOL* GaudiCommon<PBASE>::tool( const std::string& type, const IInterface* parent, bool create ) const {
  // check the environment
  Assert( PBASE::toolSvc(), "IToolSvc* points to NULL!" );
  // retrieve the tool from Tool Service
  TOOL*            Tool = nullptr;
  const StatusCode sc   = this->toolSvc()->retrieveTool( type, Tool, parent, create );
  if ( sc.isFailure() ) { Exception( "tool():: Could not retrieve Tool '" + type + "'", sc ); }
  if ( !Tool ) { Exception( "tool():: Could not retrieve Tool '" + type + "'" ); }
  // add the tool into the list of known tools to be properly released
  PBASE::registerTool( Tool );
  m_managedTools.push_back( Tool );
  // return *VALID* located tool
  return Tool;
}
// ============================================================================
// the useful method for location of services
// ============================================================================
template <class PBASE>
template <class SERVICE>
SmartIF<SERVICE> GaudiCommon<PBASE>::svc( const std::string& name, const bool create ) const {
  Assert( this->svcLoc(), "ISvcLocator* points to NULL!" );
  SmartIF<SERVICE> s;
  // check if we already have this service
  auto it = std::lower_bound( std::begin( m_services ), std::end( m_services ), name, GaudiCommon_details::svc_lt );
  if ( it != std::end( m_services ) && GaudiCommon_details::svc_eq( *it, name ) ) {
    // Try to get the requested interface
    s = *it;
    // check the results
    if ( !s ) { Exception( "svc():: Could not retrieve Svc '" + name + "'", StatusCode::FAILURE ); }
  } else {
    auto baseSvc = this->svcLoc()->service( name, create );
    // Try to get the requested interface
    s = baseSvc;
    // check the results
    if ( !baseSvc || !s ) { Exception( "svc():: Could not retrieve Svc '" + name + "'", StatusCode::FAILURE ); }
    // add the tool into list of known tools, to be properly released
    addToServiceList( baseSvc );
  }
  // return *VALID* located service
  return s;
}
// ============================================================================
// Short-cut  to get a pointer to the UpdateManagerSvc
// ============================================================================
template <class PBASE>
IUpdateManagerSvc* GaudiCommon<PBASE>::updMgrSvc() const {
  if ( !m_updMgrSvc ) { m_updMgrSvc = svc<IUpdateManagerSvc>( "UpdateManagerSvc", true ); }
  return m_updMgrSvc;
}
// ============================================================================
// Assertion - throw exception, if condition is not fulfilled
// ============================================================================
template <class PBASE>
void GaudiCommon<PBASE>::Assert( const bool ok, const std::string& msg, const StatusCode sc ) const {
  if ( !ok ) Exception( msg, sc );
}
// ============================================================================
// Assertion - throw exception, if condition is not fulfilled
// ============================================================================
template <class PBASE>
void GaudiCommon<PBASE>::Assert( const bool ok, const char* msg, const StatusCode sc ) const {
  if ( !ok ) Exception( msg, sc );
}
// ============================================================================
/** @def ALG_ERROR
 *  Small and simple macro to add into error message the file name
 *  and the line number for easy location of the problematic lines.
 *
 *  @code
 *
 *  if ( a < 0 ) { return ALG_ERROR( "'a' is negative" , 301 ) ; }
 *
 *  @endcode
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date 2002-10-29
 */
// ============================================================================
#define ALG_ERROR( message, code )                                                                                     \
  ( Error( message + std::string( " [ at line " ) + std::to_string( __LINE__ ) + std::string( " in file '" ) +         \
               std::string( __FILE__ ) + "']",                                                                         \
           code ) )

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDICOMMONIMP_H
