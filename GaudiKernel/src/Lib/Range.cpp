// ============================================================================
// Include files
// ============================================================================
// GauduKernel
// ============================================================================
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/Range.h"
// ============================================================================
/** @file
 *
 *  This file has been imported from
 *  <a href="http://savannah.cern.ch/projects/loki">LoKi project</a>
 *  <a href="http://cern.ch/lhcb-comp/Analysis/Loki">
 *  "C++ ToolKit  for Smart and Friendly Physics Analysis"</a>
 *
 *  The package has been designed with the kind help from
 *  Galina PAKHLOVA and Sergey BARSUK.  Many bright ideas,
 *  contributions and advices from G.Raven, J.van Tilburg,
 *  A.Golutvin, P.Koppenburg have been used in the design.
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
 *  @date 2001-01-23
 */
// ============================================================================
/*  Helpful function to throw an out-of-range exception for class Range_
 *  @param index invalid index
 *  @param size  range size
 */
// ============================================================================
void Gaudi::details::rangeException
( const long   /* index */ ,
  const size_t /* size  */ )
{
  throw GaudiException
    ( "Out-of-Range for class Gaudi::Range_" , "RangeError", StatusCode::FAILURE ) ;
}
// ============================================================================
// The END
// ============================================================================
