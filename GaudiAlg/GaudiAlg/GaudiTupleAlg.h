#ifndef GAUDIALG_GAUDITUPLEALG_H
#define GAUDIALG_GAUDITUPLEALG_H 1
// ============================================================================
// Include files
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiHistoAlg.h"
#include "GaudiAlg/GaudiTuples.h"
// ============================================================================

// ============================================================================
/* @file GaudiTupleAlg.h
 *
 * Header file for class : GaudiTupleAlg
 *
 * @date 2004-01-23
 * @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 */
// ============================================================================

// ============================================================================
/** @class GaudiTupleAlg GaudiTupleAlg.h GaudiAlg/GaudiTupleAlg.h
 *
 *  Simple class to extend the functionality of class GaudiHistoAlg.
 *
 *  Class is instrumented with few methods
 *  for dealing with N-tuples and Event Tag collections
 *
 *  @attention
 *  See the class GaudiTuples, which implements the common functionality
 *  between GaudiTupleTool and GaudiTupleAlg
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2004-01-23
 */
// ============================================================================

struct GAUDI_API GaudiTupleAlg : GaudiTuples<GaudiHistoAlg> {
  using GaudiTuples<GaudiHistoAlg>::GaudiTuples;
  using GaudiTuples<GaudiHistoAlg>::initialize;
  using GaudiTuples<GaudiHistoAlg>::finalize;

  GaudiTupleAlg()                       = delete;
  GaudiTupleAlg( const GaudiTupleAlg& ) = delete;
  GaudiTupleAlg& operator=( const GaudiTupleAlg& ) = delete;
};

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIALG_GAUDITUPLEALG_H
