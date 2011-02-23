// $Id: GaudiCommonConstructors.cpp,v 1.1 2005/09/23 16:14:20 hmd Exp $

// ============================================================================
/* @file GaudiCommonConstructors.cpp
 *
 *  Specialised constructors for GaudiCommon
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2005-08-08
 */
// ============================================================================

// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/AlgTool.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiCommon.h"
// ============================================================================

//=============================================================================
// Null algorithm like constructor for AlgTool
// should NEVER be used...
//=============================================================================
template <>
GaudiCommon<AlgTool>::GaudiCommon( const std::string & /* name */,
                                   ISvcLocator * /* pSvcLocator */ )
  : AlgTool ( "ERROR", "ERROR", 0 )
{
  throw GaudiException( "Invalid GaudiCommon<AlgTool> constructor", 
                        "GaudiCommon", StatusCode::FAILURE );
}
//=============================================================================

//=============================================================================
// Null tool like constructor for Algorithm
// should NEVER be used...
//=============================================================================
template <>
GaudiCommon<Algorithm>::GaudiCommon( const std::string& /* type */  ,
                                     const std::string& /* name */  ,
                                     const IInterface*  /* parent */ )
  : Algorithm ( "ERROR", 0 )
{
  throw GaudiException( "Invalid GaudiCommon<Algorithm> constructor", 
                        "GaudiCommon", StatusCode::FAILURE );
}
//=============================================================================

//=============================================================================
// Standard algorithm constructor, initializes variables
//=============================================================================
template <>
GaudiCommon<Algorithm>::GaudiCommon( const std::string & name,
                                     ISvcLocator * pSvcLocator )
  : Algorithm ( name, pSvcLocator )
{
  initGaudiCommonConstructor();
}
//=============================================================================

//=============================================================================
// Standard tool constructor, initializes variables
//=============================================================================
template <>
GaudiCommon<AlgTool>::GaudiCommon( const std::string& type   ,
                                   const std::string& name   ,
                                   const IInterface*  parent )
  : AlgTool ( type , name , parent )
{
  initGaudiCommonConstructor(parent);
}
//=============================================================================

// ============================================================================
// The END
// ============================================================================



