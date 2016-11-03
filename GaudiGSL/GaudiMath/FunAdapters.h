#ifndef GAUDIMATH_FUNADAPTERS_H
#define GAUDIMATH_FUNADAPTERS_H 1
// ============================================================================
// Include files
// ============================================================================
#include <memory>
// ============================================================================
// from CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/GenericFunctions.hh"
#include "CLHEP/GenericFunctions/Argument.hh"
#include "CLHEP/GenericFunctions/AbsFunction.hh"
// ============================================================================
#include "GaudiKernel/Kernel.h"

#if defined(__clang__) || defined(__cling__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#elif defined(__GNUC__) && __GNUC__ >= 5
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#endif

namespace AIDA { class IFunction ; }

namespace Genfun
{
  namespace GaudiMathImplementation
  {
    // ========================================================================
    /** @class AdapterIFunction Adapters.h GaudiMath/Adapters.h
     *  constructor from the IFunction ( see AIDA/IFunction.h)
     *  @see AIDA::IFunction
     *  @author Kirill Miklyaev kirillm@iris1.itep.ru
     *  @date   2003-08-03
     */
    // =========================================================================

    class GAUDI_API AdapterIFunction : public AbsFunction
    {
    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( AdapterIFunction )
    public:
      /** constructor from AIDA::Function
       *  @param fun AIDA function
       */
      AdapterIFunction ( const AIDA::IFunction& fun) ;
      /// copy constructor
      AdapterIFunction ( const AdapterIFunction& );
      /// desctructor
      ~AdapterIFunction() override;

      double operator() ( double a          ) const override;

      double operator() ( const Argument& x ) const override;

      unsigned int dimensionality () const override { return m_dim ; }

      /// Does this function have an analytic derivative?
      bool  hasAnalyticDerivative() const override { return true ; }
      /// Derivatives
      Genfun::Derivative partial( unsigned int i  ) const override;

    private:
      AdapterIFunction();
      AdapterIFunction& operator=(const AdapterIFunction&);
    private:
      const AIDA::IFunction*      m_fun ;
      size_t                      m_dim ;
      mutable std::vector<double> m_arg ;
    };
    /// mandatory macro from CLHEP/GenericFunctions
    FUNCTION_OBJECT_IMP( AdapterIFunction )

    // ========================================================================
    /** @class Adapter2DoubleFunction Adapters.h GaudiMath/Adapters.h
     *  constructor from the trivial function with two argument
     *
     *  @code
     *
     *  double func( const double , const double ) ;
     *
     *  const AbsFuction& fun = Adapter2DoubleFunction( fun ) ;
     *
     *  // evaluate the function in point (1,2):
     *  Argument arg(2) ;
     *  arg[0] = 1 ;
     *  arg[1] = 2 ;
     *
     *  double value = fun( arg ) ;
     *
     *  @endcode
     *
     *  @param func pointer to trivial function
     *  @author Kirill Miklyaev kirillm@iris1.itep.ru
     *  @author Vanya BELYAEV   Ivan.Belyaev@itep.ru
     *  @date   2003-08-03
     */
    // ========================================================================
    class GAUDI_API Adapter2DoubleFunction : public AbsFunction
    {
    public:
      /// the actual type of the function "to be adapted"
      typedef double (*Function)( const double ,
                                  const double ) ;
    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( Adapter2DoubleFunction )
    public:
      /// constructor
      Adapter2DoubleFunction ( Function func );
      /// copy coinstructor
      Adapter2DoubleFunction ( const Adapter2DoubleFunction& );
      /// destructor
      ~Adapter2DoubleFunction () override;

      double operator() (       double    x ) const override;

      double operator() ( const Argument& x ) const override;

      unsigned int dimensionality() const override { return 2    ; }
      /// Does this function have an analytic derivative?
      bool  hasAnalyticDerivative() const override { return true ; }
      /// Derivatives
      Genfun::Derivative partial( unsigned int i  ) const override;
    public:
      double operator() ( const double x , const double y ) const ;
    private:
      // default constructor is disabled
      Adapter2DoubleFunction();
      // assigmenet operator is disabled
      Adapter2DoubleFunction& operator = ( const Adapter2DoubleFunction&);
    private:
      Function                 m_func  ;
    };
    /// mandatory macro from CLHEP/GenericFunctions
    FUNCTION_OBJECT_IMP( Adapter2DoubleFunction )

    // ========================================================================
    /** @class Adapter3DoubleFunction Adapters.h GaudiMath/Adapters.h
     *  constructor from the trivial function with two argument
     *
     *  @code
     *
     *  double func( const double , const double , const double ) ;
     *
     *  const AbsFuction& fun = Adapter2DoubleFunction( fun ) ;
     *
     *  // evaluate the function in point (1,2,3):
     *  Argument arg(3) ;
     *  arg[0] = 1 ;
     *  arg[1] = 2 ;
     *  arg[2] = 3 ;
     *
     *  double value = fun( arg ) ;
     *
     *  @endcode
     *
     *  @param func pointer to trivial function
     *  @author Kirill Miklyaev kirillm@iris1.itep.ru
     *  @author Vanya BELYAEV   Ivan.Belyaev@itep.ru
     *  @date   2003-08-03
     */
    // ========================================================================
    class GAUDI_API Adapter3DoubleFunction : public AbsFunction
    {
    public:
      /// the actual type of the function "to be adapted"
      typedef double (*Function)( const double ,
                                  const double ,
                                  const double ) ;
    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( Adapter3DoubleFunction )
    public:
      /// constructor
      Adapter3DoubleFunction ( Function func );
      /// copy coinstructor
      Adapter3DoubleFunction ( const Adapter3DoubleFunction& );
      /// destructor
      ~Adapter3DoubleFunction () override ;

      double operator() (       double    x ) const override ;

      double operator() ( const Argument& x ) const override ;

      unsigned int dimensionality() const override { return 3    ; }
      /// Does this function have an analytic derivative?
      bool  hasAnalyticDerivative() const override { return true ; }
      /// Derivatives
      Genfun::Derivative partial( unsigned int i  ) const override ;
    public:
      double operator() ( const double x ,
                          const double y ,
                          const double z ) const ;
    private:
      // default constructor is disabled
      Adapter3DoubleFunction();
      // assignment operator is disabled
      Adapter3DoubleFunction& operator = ( const Adapter3DoubleFunction&);
    private:
      Function                 m_func  ;
    };
    /// mandatory macro from CLHEP/GenericFunctions
    FUNCTION_OBJECT_IMP( Adapter3DoubleFunction )

    // ========================================================================
    /** @class SimpleFunction Adapters.h GaudiMath/Adapters.h
     *
     *
     *  @author Ivan BELYAEV
     *  @date   2003-08-31
     */
    // ========================================================================
    class GAUDI_API SimpleFunction : public AbsFunction
    {
    public:
      typedef double (*Function1)( const double               ) ;
      typedef double (*Function2)( const double*              ) ;
      typedef double (*Function3)( const std::vector<double>& ) ;
    protected:
      enum Case { TrivialArg , ArrayArg , VectorArg } ;
    public:
      /// From CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( SimpleFunction )

    public:

      /** constructor from the trivial function
       *  @param func pointer to trivial function
       */
      SimpleFunction ( Function1 func ) ;

      /** constructor from the simple function with array-like argument
       *  @param func pointer to trivial function
       *  @param dim  dimension of the argument
       */
      SimpleFunction ( Function2    func ,
                       const size_t dim ) ;

      /** constructor from the simple function with vector argument
       *  @param func pointer to trivial function
       *  @param dim  dimension of the argument
       */
      SimpleFunction ( Function3    func ,
                       const size_t dim ) ;

      /// copy constructor
      SimpleFunction ( const SimpleFunction& ) ;

      // destructor
      ~SimpleFunction() override = default;
    public:

      /// dimensionality of the problem
      unsigned int dimensionality         () const override { return m_DIM   ; }
      /// Does this function have an analytic derivative?
      bool  hasAnalyticDerivative         () const override { return true    ; }
      /// Function value
      double             operator()  ( double          ) const override ;
      /// Function value
      double             operator()  ( const Argument& ) const override ;
      /// Derivatives
      Genfun::Derivative partial     ( unsigned int i  ) const override ;

    private:

      // the default constructor  is disabled
      SimpleFunction() ;
      // the assignment operator is disabled
      SimpleFunction& operator=(const SimpleFunction& );

    private:

      Case                         m_case     ;
      size_t                       m_DIM      ;

      Function1                    m_func1    ;

      Function2                    m_func2    ;
      std::unique_ptr<double[]>    m_arg2     ;

      Function3                    m_func3    ;
      mutable std::vector<double>  m_arg3     ;
    };
    /// From CLHEP/GenericFunctions
    FUNCTION_OBJECT_IMP( SimpleFunction )

  } // end of namespace GaudiMathImeplementation
} // end of namespace Genfun

#if defined(__clang__) || defined(__cling__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) && __GNUC__ >= 5
#pragma GCC diagnostic pop
#endif

#endif // GAUDIMATH_FUNADAPTERS_H
// ============================================================================
