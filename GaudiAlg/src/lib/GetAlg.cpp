// $Id: GetAlg.cpp,v 1.1 2007/09/25 16:12:41 marcocle Exp $
// ============================================================================
// Include files 
// ============================================================================
// GaudiKernel 
// ============================================================================
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IAlgContextSvc.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GetAlg.h"
// ============================================================================
/** @file 
 *  Implementation file for class  Gaudi::Utils::AlgSelector 
 *  and functions Gaudi::Utils::getAlgorithm 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ============================================================================
// virtual destructor 
// ============================================================================
Gaudi::Utils::AlgSelector::~AlgSelector(){}
// ============================================================================
// simple function to get the algorithm from Context Service
// ============================================================================
IAlgorithm* 
Gaudi::Utils::getAlgorithm  
( const IAlgContextSvc* svc , 
  const AlgSelector&    sel ) 
{
  if ( 0 == svc ) { return  0 ; } // RETURN 
  return getAlgorithm ( svc->algorithms() , sel ) ;
}
// ============================================================================
// simple function to get the algorithm from Context Service
// ============================================================================
IAlgorithm* 
Gaudi::Utils::getAlgorithm  
( const std::vector<IAlgorithm*>& lst , 
  const AlgSelector&              sel ) 
{
  for ( std::vector<IAlgorithm*>::const_reverse_iterator it = lst.rbegin() ; 
        lst.rend() != it ; ++it ) 
  {
    // use the selector:
    if ( sel ( *it ) ) { return *it ; }  // return 
  }
  return 0 ;
}
// ============================================================================


// ============================================================================
// The END 
// ============================================================================
