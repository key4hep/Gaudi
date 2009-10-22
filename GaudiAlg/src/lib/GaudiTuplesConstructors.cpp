// $Id: GaudiTuplesConstructors.cpp,v 1.1 2005/09/23 16:14:20 hmd Exp $

// ============================================================================
/* @file GaudiTuplesConstructors.cpp
 *
 *  Specialised constructors for GaudiTuples
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2005-08-08
 */
// ============================================================================

// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoAlg.h"
#include "GaudiAlg/GaudiHistoTool.h"
#include "GaudiAlg/GaudiTuples.h"

//=============================================================================
// Null algorithm like constructor for tool
// should NEVER be used...
//=============================================================================
template <>
GaudiTuples<GaudiHistoTool>::GaudiTuples( const std::string & /* name */,
                                          ISvcLocator * /* pSvcLocator */ )
  : GaudiHistoTool ( "ERROR" , "ERROR" , 0 )
{
  throw GaudiException( "Invalid GaudiTuples<GaudiTool> constructor",
                        "GaudiTuples", StatusCode::FAILURE );
}
//=============================================================================

//=============================================================================
// Null tool like constructor for Algorithm
// should NEVER be used...
//=============================================================================
template <>
GaudiTuples<GaudiHistoAlg>::GaudiTuples( const std::string& /* type */  ,
                                         const std::string& /* name */  ,
                                         const IInterface*  /* parent */ )
  : GaudiHistoAlg ( "ERROR" , 0 )
{
  throw GaudiException( "Invalid GaudiTuples<GaudiAlgorithm> constructor",
                        "GaudiTuples", StatusCode::FAILURE );
}
//=============================================================================

//=============================================================================
// Standard algorithm constructor, initializes variables
//=============================================================================
template <>
GaudiTuples<GaudiHistoAlg>::GaudiTuples( const std::string & name,
                                         ISvcLocator * pSvcLocator )
  : GaudiHistoAlg ( name , pSvcLocator )
{
  initGaudiTuplesConstructor();
}
//=============================================================================

//=============================================================================
// Standard tool constructor, initializes variables
//=============================================================================
template <>
GaudiTuples<GaudiHistoTool>::GaudiTuples( const std::string& type   ,
                                          const std::string& name   ,
                                          const IInterface*  parent )
  : GaudiHistoTool ( type , name , parent )
{
  initGaudiTuplesConstructor();
}
//=============================================================================

// ============================================================================
// The END
// ============================================================================
