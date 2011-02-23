// $Id: GaudiAlg.h,v 1.4 2006/03/09 10:28:14 hmd Exp $
// ============================================================================
#ifndef GAUDIALG_GAUDIALG_H
#define GAUDIALG_GAUDIALG_H 1
// ============================================================================
// Include files
// ============================================================================
// Gaudi
// ============================================================================
#include "GaudiKernel/Kernel.h"
// ============================================================================
// STD & STL
// ============================================================================
#include <string>
// ============================================================================

/** @namespace GaudiAlg GaudiAlg.h GaudiAlg/GaudiAlg.h
 *  Namespace with definition of useful constants, types and function,
 *  common for all classes from GaudiAlg package
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date   2005-08-06
 */
namespace GaudiAlg
{
  /** convers number into the string
   *  (remove the actual code duplication from namespaces
   *  GaudiAlgLocal and GaudiToolLocal
   *  @param number value
   *  @return string representation
   */
  GAUDI_API std::string fileLine( const int number ) ;
} // end of namespace GaudiAlg

#endif // GAUDIALG_GAUDIALG_H
