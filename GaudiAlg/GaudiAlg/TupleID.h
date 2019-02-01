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
namespace Tuples {
  /// the actual type for N-Tuple identifier (HBOOK-style)
  typedef GaudiAlg::ID TupleID;
  inline std::size_t   hash_value( TupleID const& b ) { return b.hash(); }
} // namespace Tuples
// ============================================================================

// ============================================================================
namespace GaudiAlg {
  /// the actual type for N-Tuple identifier
  typedef Tuples::TupleID TupleID;
  inline std::size_t      hash_value( TupleID const& b ) { return b.hash(); }
} // namespace GaudiAlg
// ============================================================================

#endif // GAUDIALG_TUPLEID_H
