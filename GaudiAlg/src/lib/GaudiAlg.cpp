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
// Boots
// ============================================================================
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   coming from boost/lexical_cast.hpp
#pragma warning(disable:2259)
#endif
#include "boost/lexical_cast.hpp"
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
{ return boost::lexical_cast<std::string>( number ) ; }
// ============================================================================
