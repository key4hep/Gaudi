// $Id: GaudiAlg.cpp,v 1.4 2005/11/18 17:28:32 mato Exp $
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <string>
// ============================================================================
// GaudiAlg 
// ============================================================================
#include "GaudiAlg/GaudiAlg.h"
// ============================================================================

/** @file
 *  Implementation file for functions from namespace GaudiAlg
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr 
 *  @date 2005-08-06 
 */

// ============================================================================
/** convers number into the string 
 *  (remove the actual code duplication from namespaces 
 *  GaudiAlgLocal and GaudiToolLocal 
 *  @param number value 
 *  @return string representation
 */
// ============================================================================
std::string fileLine( const int number ) 
{ return std::to_string( number ) ; }
// ============================================================================
