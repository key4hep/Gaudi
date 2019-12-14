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
IAlgorithm* Gaudi::Utils::getAlgorithm( const IAlgContextSvc* svc, const AlgSelector& sel ) {
  return svc ? getAlgorithm( svc->algorithms(), sel ) : nullptr;
}
// ============================================================================
// simple function to get the algorithm from Context Service
// ============================================================================
IAlgorithm* Gaudi::Utils::getAlgorithm( const std::vector<IAlgorithm*>& lst, const AlgSelector& sel ) {
  auto it = std::find_if( lst.rbegin(), lst.rend(), std::cref( sel ) );
  return it != lst.rend() ? *it : nullptr;
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
