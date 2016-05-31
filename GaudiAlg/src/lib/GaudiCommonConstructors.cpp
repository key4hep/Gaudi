
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
GaudiCommon<AlgTool>::GaudiCommon( const std::string&  /* name */,
                                   ISvcLocator * /* pSvcLocator */ )
  : base_class ( "ERROR", "ERROR", nullptr )
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
  : base_class ( "ERROR", nullptr )
{
  throw GaudiException( "Invalid GaudiCommon<Algorithm> constructor", 
                        "GaudiCommon", StatusCode::FAILURE );
}
//=============================================================================

//=============================================================================
// Standard algorithm constructor, initializes variables
//=============================================================================
template <>
GaudiCommon<Algorithm>::GaudiCommon( const std::string& name,
                                     ISvcLocator * pSvcLocator )
  : base_class ( name, pSvcLocator )
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
                                   const IInterface*  parnt )
  : base_class ( type , name , parnt )
{
  initGaudiCommonConstructor(parent());
}
//=============================================================================

// ============================================================================
// The END
// ============================================================================
