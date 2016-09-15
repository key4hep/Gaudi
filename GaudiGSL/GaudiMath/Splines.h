#ifndef GAUDIMATH_SPLINES_H
#define GAUDIMATH_SPLINES_H 1
// ============================================================================
// Include files
// ============================================================================
// STD/STL
// ============================================================================
#include <vector>
#include <utility>
#include <algorithm>
#include <memory>
// ============================================================================
// from CLHEP
// ============================================================================
#include "CLHEP/GenericFunctions/GenericFunctions.hh"
#include "CLHEP/GenericFunctions/Argument.hh"
#include "CLHEP/GenericFunctions/AbsFunction.hh"
// ============================================================================
// GaudiGSL/GaudiMath
// ============================================================================
#include "GaudiMath/Interpolation.h"
// ============================================================================
// GSL
// ============================================================================
#include "gsl/gsl_interp.h"
#include "gsl/gsl_spline.h"
// ============================================================================
#include "GaudiKernel/Kernel.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif

namespace Genfun
{
  namespace GaudiMathImplementation
  {

    class  GAUDI_API SplineBase
    {
    public:
      typedef std::vector<double>                    Data1D ;
      typedef std::vector<std::pair<double,double> > Data2D ;
    public:
      /** constructor from vectors and type
       *  @param x    vector of x
       *  @param y    vector of y(x)
       *  @param type interpolation type
       */
      SplineBase
      ( const Data1D&                        x    ,
        const Data1D&                        y    ,
        const GaudiMath::Interpolation::Type type ) ;
      /** constructor from vector of (x,y(x)) pairs
       *  @param data vector of (x,y(x)) pairs
       *  @param type interpolaiton type
       */
      SplineBase
      ( const Data2D&                        data ,
        const GaudiMath::Interpolation::Type type ) ;
      /** templated constructor in the spirit of STL-algorithms
       *  @param type    interpolation type
       *  @param begin_x begin of X-sequence
       *  @param end_x   end   of X-sequence
       *  @param begin_Y begin of Y-sequence
       */
      template <class DATAX, class DATAY>
      SplineBase
      ( const GaudiMath::Interpolation::Type type    ,
        DATAX                                begin_x ,
        DATAX                                end_x   ,
        DATAY                                begin_y )
        : m_dim       ( end_x - begin_x )
        , m_x         ( new double[ end_x - begin_x ] )
        , m_y         ( new double[ end_x - begin_x ] )
        , m_type      ( type )
      {
        std::copy ( begin_x , end_x                         , m_x.get() ) ;
        std::copy ( begin_y , begin_y + ( end_x - begin_x ) , m_y.get() ) ;
      }
      /** templated constructor from the sequence of (x,y(x)) pairs
       *  as sequence of pairs the class TabulatedProperty
       *  can be used
       *  @param type  interpolation type
       *  @param begin begin of sequence of (x,y(x)) pairs
       *  @param end   end   of sequence of (x,y(x)) pairs
       */
      template <class DATA>
      SplineBase
      ( const GaudiMath::Interpolation::Type type  ,
        DATA                                 begin ,
        DATA                                 end   )
        : m_dim       ( end - begin  )
        , m_x         ( new double[ end - begin ] )
        , m_y         ( new double[ end - begin ] )
        , m_type      ( type )
      {
        double* _x = m_x.get() ;
        double* _y = m_y.get() ;
        for ( auto it = begin ; end != it ; ++ it )
        {
          *_x++ = it -> first  ;
          *_y++ = it -> second ;
        };
      }
      /// copy constructor
      SplineBase( const SplineBase& ) ;
      /// destructor
      virtual ~SplineBase();
    public:
      /// evaluate the function
      double eval   ( const double x ) const ;
      /// evaluate the first derivative
      double deriv  ( const double x ) const ;
      /// evaluate the second  derivative
      double deriv2 ( const double x ) const ;
      /// evaluate the integral on [a,b]
      double integ  ( const double a , const double b ) const ;
    protected:
      // initialize
      void initialize () const ;
    private:
      // default constructor is disabled
      SplineBase() = delete;
      // assigenement operator  is disabled
      SplineBase& operator=( const SplineBase& ) = delete;
    private:
      mutable bool                   m_init   = false;
      size_t                         m_dim    ;
      std::unique_ptr<double[]>      m_x      ;
      std::unique_ptr<double[]>      m_y      ;
      mutable gsl_spline*            m_spline = nullptr;
      mutable gsl_interp_accel*      m_accel  = nullptr;
      GaudiMath::Interpolation::Type m_type   ;
    };

    class GAUDI_API GSLSpline : public AbsFunction
    {
    public:
      typedef SplineBase::Data1D Data1D ;
      typedef SplineBase::Data2D Data2D ;
    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( GSLSpline )
    public:
      /** constructor from vectors and type
       *
       *  @code
       *
       *   Data1D x = .. ;
       *   Data1D y = .. ;
       *
       *   const AbsFunction& func =
       *        GSLSpline( x , y , GaudiMath::Interpolation::Cspline ) ;
       *
       *   const double value = 1 ;
       *   const double result = func( value ) ;
       *
       *  @endcode
       *
       *
       *  @param x    vector of x
       *  @param y    vector of y
       *  @param type interpolation type
       */
      GSLSpline
      ( const Data1D&                        x    ,
        const Data1D&                        y    ,
        const GaudiMath::Interpolation::Type type ) ;
      /** constructor from data vector
       *
       *  @code
       *
       *   Data2D data = ... ;
       *
       *   const AbsFunction& func =
       *        GSLSpline( data , GaudiMath::Interpolation::Cspline );
       *
       *   const double value = 1 ;
       *   const double result = func( value ) ;
       *
       *  @endcode
       *
       *
       *  @param data vector of (x,y) pairs
       *  @param type interpolation type
       */
      GSLSpline
      ( const Data2D&                        data ,
        const GaudiMath::Interpolation::Type type ) ;
      /** templated constructor in the spirit of STL-algorithms
       *
       *  It is assumed that vector "y" has tehlength
       *    AT LEAST as long as "x"
       *
       *  @code
       *
       *   SEQURNCE1 x = .. ;
       *   SEQUENCE2 y = .. ;
       *
       *   const AbsFunction& func =
       *        GSLSpline( GaudiMath::Interpoaltion::Cspline ,
       *                   x.begin ()  ,
       *                   x.end   ()  ,
       *                   y.begin ()  );
       *
       *   const double value = 1 ;
       *   const double result = func( value ) ;
       *
       *  @endcode
       *
       *  @param begin_x begin of X-sequence
       *  @param end_x   end   of X-sequence
       *  @param begin_Y begin of Y-sequence
       *  @param type    interpolation type
       */
      template <class DATAX, class DATAY>
      GSLSpline
      ( const GaudiMath::Interpolation::Type type    ,
        DATAX                                begin_x ,
        DATAX                                end_x   ,
        DATAY                                begin_y )
        : AbsFunction ( )
        , m_spline( type , begin_x , end_x , begin_y )
      {}
      /** templated constructor from the sequence of pairs
       *  as sequence of pairs the class TabulatedProperty
       *  can be used
       *
       *  @code
       *  @endcode
       *
       *  @param begin begin of sequence of pairs
       *  @param end   end   of sequence of pairs
       *  @param type  interpolation type
       */
      template <class DATA>
      GSLSpline
      ( const GaudiMath::Interpolation::Type type  ,
        DATA                                 begin ,
        DATA                                 end   )
        : AbsFunction ( )
        , m_spline( type , begin , end )
      {}
      /// constructor from base
      GSLSpline ( const SplineBase& ) ;
      /// copy constructor
      GSLSpline ( const GSLSpline& ) ;
      /// destructor
      virtual ~GSLSpline() ;
    public:
      /// main methgod: evaluate teh function
      virtual double operator() ( double a          ) const ;
      /// main methgod: evaluate teh function
      virtual double operator() ( const Argument& x ) const ;
      virtual unsigned int dimensionality () const { return 1   ; }
      /// Does this function have an analytic derivative?
      virtual bool  hasAnalyticDerivative() const { return true ; }
      /// Derivatives
      virtual Genfun::Derivative partial( unsigned int i  ) const ;
    public:
      /// acess to the spline function
      inline   const SplineBase& spline() const { return m_spline ; }
      /// cast operator to the spline function
      operator const SplineBase&       () const { return spline() ; }
    private:
      /// default construtor   is disabled ;
      GSLSpline() = delete;
      /// assignement operator is disabled ;
      GSLSpline& operator=( const GSLSpline& ) = delete;
    private:
      // the actual spline function
      SplineBase m_spline ;
    };

    class GAUDI_API GSLSplineDeriv : public AbsFunction
    {
    public:
      typedef SplineBase::Data1D Data1D ;
      typedef SplineBase::Data2D Data2D ;
    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( GSLSplineDeriv )
    public:
      /** constructor from vectors and type
       *
       *  @code
       *
       *   Data1D x = .. ;
       *   Data1D y = .. ;
       *
       *   const AbsFunction& func =
       *        GSLSplineDeriv( x , y , GaudiMath::Interpolation::Cspline ) ;
       *
       *   const double value = 1 ;
       *   const double result = func( value ) ;
       *
       *  @endcode
       *
       *
       *  @param x    vector of x
       *  @param y    vector of y
       *  @param type interpolation type
       */
      GSLSplineDeriv
      ( const Data1D&                        x    ,
        const Data1D&                        y    ,
        const GaudiMath::Interpolation::Type type ) ;
      /** constructor from data vector
       *
       *  @code
       *
       *   Data2D data = ... ;
       *
       *   const AbsFunction& func =
       *        GSLSpline( data , GaudiMath::Interpolation::Cspline );
       *
       *   const double value = 1 ;
       *   const double result = func( value ) ;
       *
       *  @endcode
       *
       *
       *  @param data vector of (x,y) pairs
       *  @param type interpolation type
       */
      GSLSplineDeriv
      ( const Data2D&                        data ,
        const GaudiMath::Interpolation::Type type ) ;
      /** templated constructor in the spirit of STL-algorithms
       *
       *  It is assumed that vector "y" has tehlength
       *    AT LEAST as long as "x"
       *
       *  @code
       *
       *   SEQURNCE1 x = .. ;
       *   SEQUENCE2 y = .. ;
       *
       *   const AbsFunction& func =
       *        GSLSpline( GaudiMath::Interpoaltion::Cspline ,
       *                   x.begin ()  ,
       *                   x.end   ()  ,
       *                   y.begin ()  );
       *
       *   const double value = 1 ;
       *   const double result = func( value ) ;
       *
       *  @endcode
       *
       *  @param begin_x begin of X-sequence
       *  @param end_x   end   of X-sequence
       *  @param begin_Y begin of Y-sequence
       *  @param type    interpolation type
       */
      template <class DATAX, class DATAY>
      GSLSplineDeriv
      ( const GaudiMath::Interpolation::Type type    ,
        DATAX                                begin_x ,
        DATAX                                end_x   ,
        DATAY                                begin_y )
        : AbsFunction ( )
        , m_spline( type , begin_x , end_x , begin_y )
      {}
      /** templated constructor from the sequence of pairs
       *  as sequence of pairs the class TabulatedProperty
       *  can be used
       *
       *  @code
       *  @endcode
       *
       *  @param begin begin of sequence of pairs
       *  @param end   end   of sequence of pairs
       *  @param type  interpolation type
       */
      template <class DATA>
      GSLSplineDeriv
      ( const GaudiMath::Interpolation::Type type  ,
        DATA                                 begin ,
        DATA                                 end   )
        : AbsFunction ( )
        , m_spline( type , begin , end )
      {}
      /// constructor from base
      GSLSplineDeriv ( const SplineBase&     ) ;
      /// copy constructor
      GSLSplineDeriv ( const GSLSplineDeriv& ) ;
      /// destructor
      ~GSLSplineDeriv() override;
    public:
      /// main method: evaluate the function
      double operator() ( double a          ) const override;
      /// main method: evaluate the function
      double operator() ( const Argument& x ) const override;
      unsigned int dimensionality () const override { return 1   ; }
      /// Does this function have an analytic derivative?
      bool  hasAnalyticDerivative() const override { return true ; }
      /// Derivatives
      Genfun::Derivative partial( unsigned int i  ) const override ;
    public:
      /// acess to the spline function
      inline   const SplineBase& spline() const { return m_spline ; }
      /// cast operator to the spline function
      operator const SplineBase&       () const { return spline() ; }
    private:
      /// default construtor   is desabled ;
      GSLSplineDeriv() ;
      /// assignement operator is desabled ;
      GSLSplineDeriv& operator=( const GSLSplineDeriv& ) ;
    private:
      // the actual spline function
      SplineBase m_spline ;
    };

    class GAUDI_API GSLSplineDeriv2 : public AbsFunction
    {
    public:
      typedef SplineBase::Data1D Data1D ;
      typedef SplineBase::Data2D Data2D ;
    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( GSLSplineDeriv2 )
    public:
      /** constructor from vectors and type
       *
       *  @code
       *
       *   Data1D x = .. ;
       *   Data1D y = .. ;
       *
       *   const AbsFunction& func =
       *        GSLSplineDeriv2( x , y , GaudiMath::Interpolation::Cspline ) ;
       *
       *   const double value = 1 ;
       *   const double result = func( value ) ;
       *
       *  @endcode
       *
       *
       *  @param x    vector of x
       *  @param y    vector of y
       *  @param type interpolation type
       */
      GSLSplineDeriv2
      ( const Data1D&                        x    ,
        const Data1D&                        y    ,
        const GaudiMath::Interpolation::Type type ) ;
      /** constructor from data vector
       *
       *  @code
       *
       *   Data2D data = ... ;
       *
       *   const AbsFunction& func =
       *        GSLSplineDeriv2( data , GaudiMath::Interpolation::Cspline );
       *
       *   const double value = 1 ;
       *   const double result = func( value ) ;
       *
       *  @endcode
       *
       *
       *  @param data vector of (x,y) pairs
       *  @param type interpolation type
       */
      GSLSplineDeriv2
      ( const Data2D&                        data ,
        const GaudiMath::Interpolation::Type type ) ;
      /** templated constructor in the spirit of STL-algorithms
       *
       *  It is assumed that vector "y" has tehlength
       *    AT LEAST as long as "x"
       *
       *  @code
       *
       *   SEQURNCE1 x = .. ;
       *   SEQUENCE2 y = .. ;
       *
       *   const AbsFunction& func =
       *        GSLSpline( GaudiMath::Interpoaltion::Cspline ,
       *                   x.begin ()  ,
       *                   x.end   ()  ,
       *                   y.begin ()  );
       *
       *   const double value = 1 ;
       *   const double result = func( value ) ;
       *
       *  @endcode
       *
       *  @param begin_x begin of X-sequence
       *  @param end_x   end   of X-sequence
       *  @param begin_Y begin of Y-sequence
       *  @param type    interpolation type
       */
      template <class DATAX, class DATAY>
      GSLSplineDeriv2
      ( const GaudiMath::Interpolation::Type type    ,
        DATAX                                begin_x ,
        DATAX                                end_x   ,
        DATAY                                begin_y )
        : AbsFunction ( )
        , m_spline( type , begin_x , end_x , begin_y )
      {}
      /** templated constructor from the sequence of pairs
       *  as sequence of pairs the class TabulatedProperty
       *  can be used
       *
       *  @code
       *  @endcode
       *
       *  @param begin begin of sequence of pairs
       *  @param end   end   of sequence of pairs
       *  @param type  interpolation type
       */
      template <class DATA>
      GSLSplineDeriv2
      ( const GaudiMath::Interpolation::Type type  ,
        DATA                                 begin ,
        DATA                                 end   )
        : AbsFunction ( )
        , m_spline( type , begin , end )
      {}
      /// constructor from base
      GSLSplineDeriv2 ( const SplineBase&      ) ;
      /// copy constructor
      GSLSplineDeriv2 ( const GSLSplineDeriv2& ) ;
      /// destructor
      ~GSLSplineDeriv2() override;
    public:
      /// main method: evaluate the function
      double operator() ( double a          ) const override;
      /// main method: evaluate the function
      double operator() ( const Argument& x ) const override;
      unsigned int dimensionality () const override { return 1   ; }
      /// Does this function have an analytic derivative?
      bool  hasAnalyticDerivative() const override { return true ; }
      /// Derivatives
      Genfun::Derivative partial( unsigned int i  ) const override;
    public:
      /// acess to the spline function
      inline   const SplineBase& spline() const { return m_spline ; }
      /// cast operator to the spline function
      operator const SplineBase&       () const { return spline() ; }
    private:
      /// default construtor   is disabled ;
      GSLSplineDeriv2() = delete;
      /// assignement operator is disabled ;
      GSLSplineDeriv2& operator=( const GSLSplineDeriv2& ) = delete;
    private:
      // the actual spline function
      SplineBase m_spline ;
    };


    class GAUDI_API GSLSplineInteg : public AbsFunction
    {
    public:
      typedef SplineBase::Data1D Data1D ;
      typedef SplineBase::Data2D Data2D ;
    public:
      /// mandatory macro from CLHEP/GenericFunctions
      FUNCTION_OBJECT_DEF( GSLSplineInteg )
    public:
      /** constructor from vectors and type
       *
       *  @code
       *
       *   Data1D x = .. ;
       *   Data1D y = .. ;
       *
       *   const AbsFunction& func =
       *    GSLSplineInteg( x , y , GaudiMath::Interpolation::Cspline , -1 ) ;
       *
       *   const double value = 1 ;
       *   const double result = func( value ) ;
       *
       *  @endcode
       *
       *  @param x    vector of x
       *  @param y    vector of y
       *  @param type interpolation type
       *  @param low  low integration limit
       */
      GSLSplineInteg
      ( const Data1D&                        x        ,
        const Data1D&                        y        ,
        const GaudiMath::Interpolation::Type type     ,
        const double                         low  = 0 ) ;
      /** constructor from data vector
       *
       *  @code
       *
       *   Data2D data = ... ;
       *
       *   const AbsFunction& func =
       *        GSLSplineInteg( data , GaudiMath::Interpolation::Cspline , -1 );
       *
       *   const double value = 1 ;
       *   const double result = func( value ) ;
       *
       *  @endcode
       *
       *
       *  @param data vector of (x,y) pairs
       *  @param type interpolation type
       *  @param low  low integration limit
       */
      GSLSplineInteg
      ( const Data2D&                        data     ,
        const GaudiMath::Interpolation::Type type     ,
        const double                         low  = 0 ) ;
      /** templated constructor in the spirit of STL-algorithms
       *
       *  It is assumed that vector "y" has tehlength
       *    AT LEAST as long as "x"
       *
       *  @code
       *
       *   SEQURNCE1 x = .. ;
       *   SEQUENCE2 y = .. ;
       *
       *   const AbsFunction& func =
       *        GSLSpline( GaudiMath::Interpoaltion::Cspline ,
       *                   x.begin ()  ,
       *                   x.end   ()  ,
       *                   y.begin ()  );
       *
       *   const double value = 1 ;
       *   const double result = func( value ) ;
       *
       *  @endcode
       *
       *  @param begin_x begin of X-sequence
       *  @param end_x   end   of X-sequence
       *  @param begin_Y begin of Y-sequence
       *  @param type    interpolation type
       *  @param low     low integration limit
       */
      template <class DATAX, class DATAY>
      GSLSplineInteg
      ( const GaudiMath::Interpolation::Type type    ,
        DATAX                                begin_x ,
        DATAX                                end_x   ,
        DATAY                                begin_y ,
        const double                         low     )
        : AbsFunction (      )
        , m_spline    ( type , begin_x , end_x , begin_y )
        , m_low       ( low  )
      {}
      /** templated constructor from the sequence of pairs
       *  as sequence of pairs the class TabulatedProperty
       *  can be used
       *
       *  @code
       *  @endcode
       *
       *  @param begin begin of sequence of pairs
       *  @param end   end   of sequence of pairs
       *  @param type  interpolation type
       *  @param low   low integration limit
       */
      template <class DATA>
      GSLSplineInteg
      ( const GaudiMath::Interpolation::Type type  ,
        DATA&&                               begin ,
        DATA&&                               end   ,
        const double                         low   )
        : m_spline    ( type , std::forward<DATA>(begin) , std::forward<DATA>(end) )
        , m_low       ( low  )
      {}
      /// constructor from base
      GSLSplineInteg ( const SplineBase&              ,
                       const double           low = 0 ) ;
      /// copy constructor
      GSLSplineInteg ( const GSLSplineInteg&      ) ;
      /// destructor
      ~GSLSplineInteg () override;
    public:
      /// main method: evaluate the function
      double operator() ( double a          ) const override;
      /// main method: evaluate the function
      double operator() ( const Argument& x ) const override;
      unsigned int dimensionality () const override { return 1   ; }
      /// Does this function have an analytic derivative?
      bool  hasAnalyticDerivative() const override { return true ; }
      /// Derivatives
      Genfun::Derivative partial( unsigned int i  ) const override;
    public:
      /// acess to the spline function
      inline   const SplineBase& spline() const { return m_spline ; }
      /// cast operator to the spline function
      operator const SplineBase&       () const { return spline() ; }
    private:
      /// default construtor   is disabled ;
      GSLSplineInteg () = delete;
      /// assignement operator is disabled ;
      GSLSplineInteg& operator=( const GSLSplineInteg& ) = delete;
    private:
      // the actual spline function
      SplineBase m_spline ;
      double     m_low    ;
    };

  }
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // GAUDIMATH_SPLINES_H
// ============================================================================
