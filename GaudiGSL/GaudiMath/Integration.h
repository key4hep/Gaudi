// $Id: Integration.h,v 1.1 2003/11/19 17:09:40 mato Exp $
// ============================================================================
#ifndef GAUDIMATH_INTEGRATION_H 
#define GAUDIMATH_INTEGRATION_H 1
// ============================================================================
// Include files
// ============================================================================

namespace GaudiMath 
{
  /** @class Integration Integration.h GaudiMath/Integration.h
   *
   *  collection of common types for classes 
   *  NumericalIndefiniteIntegral and NumericalDefiniteIntegral 
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
   *  @date   2003-09-01
   */
  namespace Integration 
  {
    /// infinity  
    enum Inf          { Infinity = 0      } ;
    /// how to distinguish variable low and variable high limits 
    enum Limit        { VariableLowLimit  , 
                        VariableHighLimit } ;
    /// type of integration (for finite limits)
    enum Type         { NonAdaptive       , 
                        Adaptive          , 
                        AdaptiveSingular  , 
                        Other             } ;
    /// integration category 
    enum Category     { Finite            , 
                        Singular          , 
                        Infinite          } ;
    /// integration rule  
    enum KronrodRule  { Fixed   =       0 ,
                        Gauss15 =       1 , 
                        Gauss21 =       2 , 
                        Gauss31 =       3 , 
                        Gauss41 =       4 , 
                        Gauss51 =       5 , 
                        Gauss61 =       6 , 
                        Default = Gauss31 } ;
  } // end of namespace Integration  
} // end of namespace GaudiMath

// ============================================================================
// The END 
// ============================================================================
#endif // GAUDIMATH_INTEGRATION_H
// ============================================================================
