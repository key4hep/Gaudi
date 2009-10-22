// $Id: Integral.cpp,v 1.2 2007/11/20 13:00:17 marcocle Exp $
// ============================================================================
// Include files 
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/GaudiException.h"
// ============================================================================
// GaudiMath
// ============================================================================
#include "GaudiMath/Integral.h"
#include "GaudiMath/NumericalDefiniteIntegral.h"
#include "GaudiMath/Constant.h"
// ============================================================================

namespace GaudiMath 
{
  
  double Integral 
  ( const Genfun::AbsFunction&                function , 
    const double                              a        , 
    const double                              b        ,
    const GaudiMath::Integration::Type        type     , 
    const GaudiMath::Integration::KronrodRule rule     ,
    const double                              epsabs   ,
    const double                              epsrel   ,
    const size_t                              size     )
  {
    if ( 1 != function.dimensionality() ) 
      { throw GaudiException
          ("GaudiMath::Integral: illegal function dimension" , 
           "*GaudiMath*" , StatusCode::FAILURE ); }
    
    const Genfun::AbsFunction& one   = 
      Genfun::GaudiMathImplementation::Constant ( 1.0 , 1 ) ;
    
    const Genfun::AbsFunction& cross = 
      Genfun::FunctionDirectProduct (&one , &function ) ;
    
    const Genfun::AbsFunction& result = 
      Genfun::GaudiMathImplementation::NumericalDefiniteIntegral 
      ( cross , 1 , a , b, type , rule , epsabs , epsrel , size ) ;
    
    return result( Genfun::Argument(1) ) ;
  };
  
  double Integral 
  ( const Genfun::AbsFunction&                 function  ,
    const double                               a         , 
    const double                               b         , 
    const std::vector<double>&                 points    ,
    const double                               epsabs    , 
    const double                               epsrel    ,
    const size_t                               size      ) 
  {

  if ( 1 != function.dimensionality() ) 
      { throw GaudiException
          ("GaudiMath::Integral: illegal function dimension" , 
           "*GaudiMath*" , StatusCode::FAILURE ); }
    
    const Genfun::AbsFunction& one   = 
      Genfun::GaudiMathImplementation::Constant ( 1.0 , 1 ) ;
    
    const Genfun::AbsFunction& cross = 
      Genfun::FunctionDirectProduct (&one , &function ) ;
    
    const Genfun::AbsFunction& result = 
      Genfun::GaudiMathImplementation::NumericalDefiniteIntegral 
      ( cross , 1 , a , b, points , epsabs , epsrel , size ) ;
    
    return result( Genfun::Argument(1) ) ;
  } ;
  
  double Integral 
  ( const Genfun::AbsFunction&                 function  ,
    const double                               a         ,
    const GaudiMath::Integration::Inf          b         ,
    const double                               epsabs    , 
    const double                               epsrel    , 
    const size_t                               size      ) 
  {
    if ( 1 != function.dimensionality() ) 
      { throw GaudiException
          ("GaudiMath::Integral: illegal function dimension" , 
           "*GaudiMath*" , StatusCode::FAILURE ); }
    
    const Genfun::AbsFunction& one   = 
      Genfun::GaudiMathImplementation::Constant ( 1.0 , 1 ) ;
    
    const Genfun::AbsFunction& cross = 
      Genfun::FunctionDirectProduct (&one , &function ) ;
    
    const Genfun::AbsFunction& result = 
      Genfun::GaudiMathImplementation::NumericalDefiniteIntegral 
      ( cross , 1 , a , b , epsabs , epsrel , size ) ;
    
    return result( Genfun::Argument(1) ) ;
  };
  
  double Integral 
  ( const Genfun::AbsFunction&                 function  ,
    const GaudiMath::Integration::Inf          a         ,
    const double                               b         ,
    const double                               epsabs    , 
    const double                               epsrel    , 
    const size_t                               size      ) 
  {
    if ( 1 != function.dimensionality() ) 
      { throw GaudiException
          ("GaudiMath::Integral: illegal function dimension" , 
           "*GaudiMath*" , StatusCode::FAILURE ); }
    
    const Genfun::AbsFunction& one   = 
      Genfun::GaudiMathImplementation::Constant ( 1.0 , 1 ) ;
    
    const Genfun::AbsFunction& cross = 
      Genfun::FunctionDirectProduct (&one , &function ) ;
    
    const Genfun::AbsFunction& result = 
      Genfun::GaudiMathImplementation::NumericalDefiniteIntegral 
      ( cross , 1 , a , b , epsabs , epsrel , size ) ;
    
    return result( Genfun::Argument(1) ) ;
  };
  
  

  double Integral 
  ( const Genfun::AbsFunction&                 function  ,
    const GaudiMath::Integration::Inf          /* a  */  ,
    const GaudiMath::Integration::Inf          /* b  */  ,
    const double                               epsabs    , 
    const double                               epsrel    , 
    const size_t                               size      ) 
  {
    if ( 1 != function.dimensionality() ) 
      { throw GaudiException
          ("GaudiMath::Integral: illegal function dimension" , 
           "*GaudiMath*" , StatusCode::FAILURE ); }
    
    const Genfun::AbsFunction& one   = 
      Genfun::GaudiMathImplementation::Constant ( 1.0 , 1 ) ;
    
    const Genfun::AbsFunction& cross = 
      Genfun::FunctionDirectProduct (&one , &function ) ;
    
    // FIXME: (MCl) the static_cast below are needed to avoid warnings and to
    // match the signature in NumericalDefiniteIntegral.h (around line 288).  
    const Genfun::AbsFunction& result = 
      Genfun::GaudiMathImplementation::NumericalDefiniteIntegral 
      ( cross , 1 , static_cast<float>(epsabs) , static_cast<float>(epsrel) , size ) ;
    
    return result( Genfun::Argument(1) ) ;
  };
  
  



  
};


// ============================================================================
// The END 
// ============================================================================

