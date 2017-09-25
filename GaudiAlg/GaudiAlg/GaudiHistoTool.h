#ifndef GAUDIALG_GAUDIHISTOTOOL_H
#define GAUDIALG_GAUDIHISTOTOOL_H 1
// ============================================================================
/** @file GaudiHistoTool.h
 *
 *  Header file for class : GaudiHistoTool
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date 2004-06-28
 */
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistos.h"
#include "GaudiAlg/GaudiTool.h"
// ============================================================================

// ============================================================================
/** @class GaudiHistoTool GaudiHistoTool.h GaudiTools/GaudiHistoTool.h
 *
 *  Simple class to extend the functionality of class GaudiTool.
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
 *  @date   2004-06-28
 */
// ============================================================================

struct GAUDI_API GaudiHistoTool : GaudiHistos<GaudiTool> {

  using GaudiHistos<GaudiTool>::GaudiHistos;
  using GaudiHistos<GaudiTool>::initialize;
  using GaudiHistos<GaudiTool>::finalize;

  // delete default/copy constructor and assignment
  GaudiHistoTool()                        = delete;
  GaudiHistoTool( const GaudiHistoTool& ) = delete;
  GaudiHistoTool& operator=( const GaudiHistoTool& ) = delete;
};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDIHISTOTOOL_H
// ============================================================================
