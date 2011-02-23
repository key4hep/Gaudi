// $Id: Tuples.h,v 1.5 2006/03/09 10:28:15 hmd Exp $
// ============================================================================
#ifndef GAUDIALG_TUPLES_H 
#define GAUDIALG_TUPLES_H 1
// ============================================================================
// GaudiAlg 
// ============================================================================
#include "GaudiAlg/TupleObj.h"
// ============================================================================

/** @namespace Tuples Tuples.h GaudiAlg/Tuples.h
 *  helper namespace to collect useful definitions, types, 
 *  constants and functions, related to manipulations 
 *  with N-Tuples and event tag collections
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-01-23
 */
namespace Tuples 
{
  class Tuple    ;
  class TupleObj ;
} // end of namespace Tuples 


namespace Tuples 
{
  /** @class _TupAdapter
   *  Trivial adapter to avoid the GCC problem 
   *  (pointer to the function is never evaluated as  
   *   "const TYPE&" )
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-05-01
   */
  template <class OBJECT,class ARGUMENT=double, class RESULT=double>
  class _TupAdapter : public std::unary_function<ARGUMENT,RESULT>
  {
  public:
    // constructor
    _TupAdapter ( OBJECT object ) : m_object ( object ) {} ;
  public:
    /// the only one essential menthod 
    inline RESULT operator() ( ARGUMENT arg ) const 
    { return m_object( arg )  ; }
  private:
    // default constructor is disabled 
    _TupAdapter();
  private:
    OBJECT m_object ;
  };
  
  /** trivial helper function to convert on flight 
   *  pointer to function into "functor"
   *  It is especially useful in conjunction with 
   *  N-Tuples fills
   *
   *  @code
   *   
   *   using namespace Tuples ;
   *  
   *  typedef std::vector<double> Array ;
   *  Array arr = ... ;
   *  
   *   tuple->farray ( "sinarr"        , 
   *                   useFun  ( sin ) ,
   *                   "cosarr"        , 
   *                   useFun  ( cos ) , 
   *                   arr.begin ()    , 
   *                   arr.end   ()    , 
   *                   "Length"  , 100 ) ;
   *
   *  @endcode 
   *
   *  @see GaudiTupleAlg
   *  @see Tuples::TupleObj
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
   *  @date 2005-05-01
   */
  template <class OBJECT>
  inline _TupAdapter<OBJECT,double,double> useFun ( OBJECT object ) 
  { return _TupAdapter<OBJECT,double,double>( object ) ; }
  
} // end of namespace Tuples 

#endif // GAUDIALG_TUPLES_H
