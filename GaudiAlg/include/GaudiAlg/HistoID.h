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
#ifndef GAUDIALG_HISTOID_H
#define GAUDIALG_HISTOID_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlg.h"
#include "GaudiAlg/GaudiHistoID.h"
#include <vector>
// ============================================================================

// ============================================================================
/** @namespace Histos
 *  helper namespace to collect useful definitions, types, constants
 *  and functions, related to manipulations with histograms
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date 2005-08-06
 */
// ============================================================================
namespace Histos {
  /// The actual type for histogram identifier
  typedef GaudiAlg::ID HistoID;
} // namespace Histos
// ============================================================================

// ============================================================================
namespace GaudiAlg {
  /// The actual type for histogram identifier
  typedef Histos::HistoID HistoID;
  /// Type for bin edges for variable binning histograms
  typedef std::vector<double> HistoBinEdges;
} // namespace GaudiAlg
// ============================================================================

#endif // GAUDIALG_HISTOID_H
