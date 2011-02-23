// $Id: GaudiHistosConstructors.cpp,v 1.1 2005/09/23 16:14:20 hmd Exp $

// ============================================================================
/* @file GaudiHistosConstructors.cpp
 *
 *  Specialised constructors for GaudiHistos
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2005-08-08
 */
// ============================================================================

// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/GaudiTool.h"
#include "GaudiAlg/GaudiHistos.h"

//=============================================================================
// Null algorithm like constructor for tool
// should NEVER be used...
//=============================================================================
template <>
GaudiHistos<GaudiTool>::GaudiHistos( const std::string & /* name */,
                                     ISvcLocator * /* pSvcLocator */ )
  : GaudiTool ( "ERROR", "ERROR", 0 )
{
  throw GaudiException( "Invalid GaudiHistos<GaudiTool> constructor",
                        "GaudiHistos", StatusCode::FAILURE );
}
//=============================================================================

//=============================================================================
// Null tool like constructor for Algorithm
// should NEVER be used...
//=============================================================================
template <>
GaudiHistos<GaudiAlgorithm>::GaudiHistos( const std::string& /* type */  ,
                                          const std::string& /* name */  ,
                                          const IInterface*  /* parent */ )
  : GaudiAlgorithm ( "ERROR", 0 )
{
  throw GaudiException( "Invalid GaudiHistos<GaudiAlgorithm> constructor",
                        "GaudiHistos", StatusCode::FAILURE );
}
//=============================================================================

//=============================================================================
// Standard algorithm constructor, initializes variables
//=============================================================================
template <>
GaudiHistos<GaudiAlgorithm>::GaudiHistos( const std::string & name,
                                          ISvcLocator * pSvcLocator )
  : GaudiAlgorithm ( name, pSvcLocator )
{
  initGaudiHistosConstructor();
}
//=============================================================================

//=============================================================================
// Standard tool constructor, initializes variables
//=============================================================================
template <>
GaudiHistos<GaudiTool>::GaudiHistos( const std::string& type   ,
                                     const std::string& name   ,
                                     const IInterface*  parent )
  : GaudiTool ( type , name , parent )
{
  initGaudiHistosConstructor();
}
//=============================================================================

// ============================================================================
// The END
// ============================================================================
