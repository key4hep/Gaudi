// $Id: TupleID.h,v 1.5 2006/03/09 10:28:14 hmd Exp $
// ============================================================================
#ifndef GAUDIALG_TUPLEID_H 
#define GAUDIALG_TUPLEID_H 1
// ============================================================================
// include files 
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlg.h"
#include "GaudiAlg/GaudiHistoID.h"
// ============================================================================

// ============================================================================
/** @namespace Tuples
 *  helper namespace to collect useful definitions, types, constants 
 *  and functions, related to manipulations with N-Tuples
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date 2005-08-06
 */
// ============================================================================
namespace Tuples 
{
  /// the actual type for N-Tuple identifier (HBOOK-style)
  typedef GaudiAlg::ID TupleID;
}
// ============================================================================

// ============================================================================
namespace GaudiAlg
{
  /// the actual type for N-Tuple identifier
  typedef Tuples::TupleID TupleID ;
}
// ============================================================================

#endif // GAUDIALG_TUPLEID_H

