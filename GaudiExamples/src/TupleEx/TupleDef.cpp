// $Id: TupleDef.cpp,v 1.3 2007/05/24 14:36:37 hmd Exp $
// ============================================================================
// CVS tag $Name:  $ , version $Revision: 1.3 $
// ============================================================================
// Include files 
// ============================================================================
// local
// ============================================================================
#include "TupleDef.h"
// ============================================================================

/** @file 
 *  Implementation file for helper function TupleExample::fillNTuple 
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2005-11-29
 */

namespace TupleExample 
{
  
  /** helper function useful for the partial 
   *  specialization of N-Tuple fills
   *  @param t tuple 
   *  @param n base column name 
   *  @param p data dublet 
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2005-11-29
   */ 
  Tuples::Tuple& fillNTuple 
  ( Tuples::Tuple&     t , 
    const std::string& n ,
    const Dublet&      p ) 
  {
    if ( !t.valid() ) { return t ; }
    t -> column ( n + "F" , p.first  ) ;
    t -> column ( n + "S" , p.second ) ;    
    return t ;
  }
  
  /** helper function useful for the partial 
   *  specialization of N-Tuple fills
   *  @param t tuple 
   *  @param n base column name 
   *  @param p data triplet  
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2005-11-29
   */ 
  Tuples::Tuple& fillNTuple 
  ( Tuples::Tuple&     t , 
    const std::string& n ,
    const Triplet&     p ) 
  {  
    if ( !t.valid() ) { return t ; }
    t -> column ( n + "F" , p.first.first  ) ;
    t -> column ( n + "S" , p.first.second ) ;    
    t -> column ( n + "T" , p.second       ) ;    
    return t ;
  }
  
}

// ============================================================================
// The END 
// ============================================================================
