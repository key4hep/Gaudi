// $Id: Constant.h,v 1.1 2003/11/19 17:09:39 mato Exp $
// ============================================================================
#ifndef GAUDIMATH_CONSTANT_H
#define GAUDIMATH_CONSTANT_H 1
// ============================================================================
// Include files
// ============================================================================
// CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/AbsFunction.hh"
// ============================================================================
#include "GaudiKernel/Kernel.h"

namespace Genfun // due to CLHEP
{
  namespace GaudiMathImplementation
  {
    /** @class Constant Constant.h GaudiMath/Constant.h
     *
     *  Fixed constant function
     *
     *  @author Vanya BELYAEV  Ivan.Belyaev@itep.ru
     *  @date   2003-08-31
     */
    class GAUDI_API Constant: public AbsFunction
    {
    public:

      /// From CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( Constant )

    public:

      /** Standard constructor
       *  @param value of the function
       *  @param dim   dimensionality
       */
      Constant ( const double value ,
                 const size_t dim   ) ;

      /// copy constructor
      Constant ( const Constant& right ) ;

      /// destructor
      virtual ~Constant();

    public:

      /// dimensionality of the problem
      virtual unsigned int dimensionality         () const { return m_DIM   ; }
      /// Does this function have an analytic derivative?
      virtual bool  hasAnalyticDerivative         () const { return true    ; }
      /// Function value
      virtual double operator()  ( double          ) const { return m_value ; }
      /// Function value
      virtual double operator()  ( const Argument& ) const { return m_value ; }
      /// Derivatives
      virtual Derivative partial ( unsigned int i  ) const ;

    private:

      // the default constructor  is disabled
      Constant() ;
      // the assignement operator is disabled
      Constant& operator=( const Constant& );

    private:

      double m_value ;
      size_t m_DIM   ;

    };

  } // end of namespace GaudiMathImplementation
} // end of namespace Genfun

// ============================================================================
// The END
// ============================================================================
#endif // GAUDIMATH_CONSTANT_H
// ============================================================================
