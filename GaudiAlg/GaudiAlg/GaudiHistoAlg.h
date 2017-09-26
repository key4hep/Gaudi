#ifndef GAUDIALG_GAUDIHISTOALG_H
#define GAUDIALG_GAUDIHISTOALG_H 1
// ============================================================================
/* @file GaudiHistoAlg.h
 *
 * Header file for class : GaudiHistoAlg
 *
 * @date 30/06/2001
 * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 */
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/GaudiHistos.h"
// ============================================================================
/** @class GaudiHistoAlg GaudiHistoAlg.h GaudiAlg/GaudiHistoAlg.h
 *
 *  Simple class to extend the functionality of class GaudiAlgorithm.
 *
 *  Class is instrumented with a selection of plotting functions for
 *  easy creation of histograms.
 *
 *  @attention
 *  See the class GaudiHistos, which implements the common functionality
 *  between GaudiHistoTool and GaudiHistoAlg
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2003-12-11
 */
// ============================================================================

struct GAUDI_API GaudiHistoAlg : GaudiHistos<GaudiAlgorithm> {
  using GaudiHistos<GaudiAlgorithm>::GaudiHistos;
  using GaudiHistos<GaudiAlgorithm>::initialize;
  using GaudiHistos<GaudiAlgorithm>::finalize;

  // delete  default/copy constructor and assignment
  GaudiHistoAlg()                       = delete;
  GaudiHistoAlg( const GaudiHistoAlg& ) = delete;
  GaudiHistoAlg& operator=( const GaudiHistoAlg& ) = delete;

}; // end of class GaudiHistoAlg

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDIHISTOAL_H
// ============================================================================
