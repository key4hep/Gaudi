// $Id: HistoID.h,v 1.5 2006/12/10 19:11:56 leggett Exp $
// ============================================================================
#ifndef GAUDIALG_HISTOID_H 
#define GAUDIALG_HISTOID_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlg.h"
#include "GaudiAlg/GaudiHistoID.h"
// ============================================================================

// ============================================================================
/** @namespace Histos
 *  helper namespace to collect useful definitions, types, constants 
 *  and functions, related to manipulations with histograms
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date 2005-08-06
 */
// ============================================================================
namespace Histos 
{
  /// The actual type for histogram identifier
  typedef GaudiAlg::ID HistoID;
}
// ============================================================================

// ============================================================================
namespace GaudiAlg
{
  /// The actual type for histogram identifier
  typedef Histos::HistoID HistoID ;
  /// Type for bin edges for variable binning histograms
  typedef std::vector<double> HistoBinEdges;
}
// ============================================================================

#endif // GAUDIALG_HISTOID_H
