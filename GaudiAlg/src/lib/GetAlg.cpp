// ============================================================================
// Include files
// ============================================================================
#include <algorithm>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IAlgorithm.h"
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
// simple function to get the algorithm from Context Service
// ============================================================================
IAlgorithm* Gaudi::Utils::getAlgorithm( const IAlgContextSvc* svc, const AlgSelector& sel )
{
  return svc ? getAlgorithm( svc->algorithms(), sel ) : nullptr;
}
// ============================================================================
// simple function to get the algorithm from Context Service
// ============================================================================
IAlgorithm* Gaudi::Utils::getAlgorithm( const std::vector<IAlgorithm*>& lst, const AlgSelector& sel )
{
  auto it = std::find_if( lst.rbegin(), lst.rend(), std::cref( sel ) );
  return it != lst.rend() ? *it : nullptr;
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
