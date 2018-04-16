// ============================================================================
// Include files
// ============================================================================
// GaudiGSL/GaudiMath
// ============================================================================
#include "GaudiMath/GSLFunAdapters.h"
#include "GaudiMath/GaudiMath.h"
// ============================================================================
// GSL
// ============================================================================
#include "gsl/gsl_mode.h"
#include "gsl/gsl_sf_result.h"
// ============================================================================

/** @file
 *  implemenation file for classes from
 *  namespace Genfun::GaudiMathImplementation
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date   2004-03-06
 */

namespace Genfun
{
  namespace GaudiMathImplementation
  {

    GSLFunctionWithError::GSLFunctionWithError( GSLFunctionWithError::Function function )
        : AbsFunction(), m_function( function ), m_result( std::make_unique<gsl_sf_result>() )
    {
      m_result->val = -1.e+10;
      m_result->err = -1.e+10;
    }

    GSLFunctionWithError::GSLFunctionWithError( const GSLFunctionWithError& func )
        : AbsFunction(), m_function( func.m_function ), m_result( std::make_unique<gsl_sf_result>() )
    {
      m_result->val = func.m_result->val;
      m_result->err = func.m_result->err;
    }

    double GSLFunctionWithError::operator()( double x ) const
    {
      ( *m_function )( x, m_result.get() );
      return m_result->val;
    }

    double GSLFunctionWithError::operator()( const Genfun::Argument& x ) const
    {
      ( *m_function )( x[0], m_result.get() );
      return m_result->val;
    }

    Genfun::Derivative GSLFunctionWithError::partial( unsigned int i ) const
    {
      if ( i >= 1 ) {
        const AbsFunction& aux = GaudiMath::Constant( 0, 1 );
        return Genfun::FunctionNoop( &aux );
      };
      const AbsFunction& aux = GaudiMath::Derivative( *this, i );
      return Genfun::FunctionNoop( &aux );
    }

    GSLFunctionWithError::Function GSLFunctionWithError::function() const { return m_function; }

    const gsl_sf_result& GSLFunctionWithError::result() const { return *m_result; }

    double GSLFunctionWithError::error() const { return m_result->err; }

    GSLFunctionWithMode::GSLFunctionWithMode( GSLFunctionWithMode::Function function, const gsl_mode_t& mod )
        : AbsFunction(), m_function( function ), m_mode( std::make_unique<gsl_mode_t>( mod ) )
    {
    }

    GSLFunctionWithMode::GSLFunctionWithMode( const GSLFunctionWithMode& func )
        : AbsFunction(), m_function( func.m_function ), m_mode( std::make_unique<gsl_mode_t>( *( func.m_mode ) ) )
    {
    }

    double GSLFunctionWithMode::operator()( double x ) const { return ( *m_function )( x, *m_mode ); }

    double GSLFunctionWithMode::operator()( const Genfun::Argument& x ) const
    {
      return ( *m_function )( x[0], *m_mode );
    }

    Genfun::Derivative GSLFunctionWithMode::partial( unsigned int i ) const
    {
      if ( i >= 1 ) {
        const AbsFunction& aux = GaudiMath::Constant( 0, 1 );
        return Genfun::FunctionNoop( &aux );
      };
      const AbsFunction& aux = GaudiMath::Derivative( *this, i );
      return Genfun::FunctionNoop( &aux );
    }

    GSLFunctionWithMode::Function GSLFunctionWithMode::function() const { return m_function; }

    const gsl_mode_t& GSLFunctionWithMode::mode() const { return *m_mode; }

    GSLFunctionWithModeAndError::GSLFunctionWithModeAndError( GSLFunctionWithModeAndError::Function function,
                                                              const gsl_mode_t&                     mod )
        : AbsFunction()
        , m_function( function )
        , m_mode( std::make_unique<gsl_mode_t>( mod ) )
        , m_result( std::make_unique<gsl_sf_result>( gsl_sf_result{-1.e+10, -1.e+10} ) )
    {
    }

    GSLFunctionWithModeAndError::GSLFunctionWithModeAndError( const GSLFunctionWithModeAndError& func )
        : AbsFunction()
        , m_function( func.m_function )
        , m_mode( std::make_unique<gsl_mode_t>( *func.m_mode ) )
        , m_result( std::make_unique<gsl_sf_result>( *func.m_result ) )
    {
    }

    double GSLFunctionWithModeAndError::operator()( double x ) const
    {
      ( *m_function )( x, *m_mode, m_result.get() );
      return m_result->val;
    }

    double GSLFunctionWithModeAndError::operator()( const Genfun::Argument& x ) const
    {
      ( *m_function )( x[0], *m_mode, m_result.get() );
      return m_result->val;
    }

    Genfun::Derivative GSLFunctionWithModeAndError::partial( unsigned int i ) const
    {
      if ( i >= 1 ) {
        const AbsFunction& aux = GaudiMath::Constant( 0, 1 );
        return Genfun::FunctionNoop( &aux );
      };
      const AbsFunction& aux = GaudiMath::Derivative( *this, i );
      return Genfun::FunctionNoop( &aux );
    }

    GSLFunctionWithModeAndError::Function GSLFunctionWithModeAndError::function() const { return m_function; }

    const gsl_mode_t& GSLFunctionWithModeAndError::mode() const { return *m_mode; }

    const gsl_sf_result& GSLFunctionWithModeAndError::result() const { return *m_result; }

    double GSLFunctionWithModeAndError::error() const { return m_result->err; }

  } // end of namespace GaudiMathImplementation
} // end of namespace Genfun
