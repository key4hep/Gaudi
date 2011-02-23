// $Id: TupleDef.h,v 1.3 2007/05/24 14:36:37 hmd Exp $
// ============================================================================
// CVS Tag $Name:  $ , version $Revision: 1.3 $ 
// ============================================================================
#ifndef TUPLEEX_TUPLEDEF_H 
#define TUPLEEX_TUPLEDEF_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
#include <utility>
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/Tuple.h"
// ============================================================================

/** @class TupleDef TupleDef.h TupleEx/TupleDef.h
 *
 *  Simple example of partical specialization of 
 *  N-Tuple for different types, in particular for 
 *
 *  std::pair<double,double> and 
 *  std::pair<std::pair<double,double>,double> and 
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2005-11-29
 */

/** @namespace TupleExample 
 *  helper namespace for TupleEx example
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2005-11-29
 */ 
namespace TupleExample
{
  typedef std::pair<double,double> Dublet  ;
  typedef std::pair<Dublet,double> Triplet ;
  
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
    const Dublet&      p ) ;  
  
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
    const Triplet&     p ) ;  
} // end of namespace TupleExample 


// ============================================================================
// define corresponding partial specializations: 
// ============================================================================
template <> 
inline Tuples::Tuple& operator<< 
  ( Tuples::Tuple&                                    tuple , 
    const Tuples::TupleColumn<TupleExample::Dublet>&  item  ) 
{
  return TupleExample::fillNTuple ( tuple         , 
                                    item.name  () , 
                                    item.value () ) ;
}
// ============================================================================
template <> 
inline Tuples::Tuple& operator<< 
  ( Tuples::Tuple&                                    tuple , 
    const Tuples::TupleColumn<TupleExample::Triplet>& item  ) 
{
  return TupleExample::fillNTuple ( tuple         , 
                                    item.name  () , 
                                    item.value () ) ;
}
// ============================================================================


// ============================================================================
// The END 
// ============================================================================
#endif // TUPLEEX_TUPLEDEF_H
// ============================================================================
