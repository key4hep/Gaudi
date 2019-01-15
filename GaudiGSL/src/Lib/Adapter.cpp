// Include files
// ============================================================================
// GaudiMath
// ============================================================================
#include "GaudiMath/GaudiMath.h"
// ============================================================================
// AIDA
// ============================================================================
#include "AIDA/IFunction.h"
// ============================================================================
#include <cassert>
#include <cstring>

namespace Genfun
{
  namespace GaudiMathImplementation
  {

    AdapterIFunction::AdapterIFunction( const AIDA::IFunction& fun ) : m_fun( &fun ), m_arg( fun.dimension(), 0 ) {}

    double AdapterIFunction::operator()( double x ) const
    {
      assert( begin( m_arg ) != end( m_arg ) );
      m_arg[0] = x;
      std::fill( std::next( begin( m_arg ) ), end( m_arg ), 0.0 );
      return m_fun->value( m_arg );
    }

    double AdapterIFunction::operator()( const Genfun::Argument& x ) const
    {
      for ( size_t i = 0; i < m_arg.size(); ++i ) {
        m_arg[i] = x[i];
      }
      return m_fun->value( m_arg );
    }

    Genfun::Derivative AdapterIFunction::partial( unsigned int i ) const
    {
      if ( i >= m_arg.size() ) {
        const AbsFunction& aux = GaudiMath::Constant( 0, m_arg.size() );
        return Genfun::FunctionNoop( &aux );
      };
      const AbsFunction& aux = GaudiMath::Derivative( *this, i );
      return Genfun::FunctionNoop( &aux );
    }

    Adapter2DoubleFunction::Adapter2DoubleFunction( Adapter2DoubleFunction::Function func )
        : AbsFunction(), m_func( func )
    {
    }

    double Adapter2DoubleFunction::operator()( double x ) const { return m_func( x, 0 ); }

    double Adapter2DoubleFunction::operator()( const Genfun::Argument& x ) const { return m_func( x[0], x[1] ); }

    double Adapter2DoubleFunction::operator()( const double x, const double y ) const { return m_func( x, y ); }

    /// Derivatives
    Genfun::Derivative Adapter2DoubleFunction::partial( unsigned int i ) const
    {
      if ( i >= 2 ) {
        GaudiMath::Constant aux{0, 2};
        return Genfun::FunctionNoop( &aux );
      }
      GaudiMath::Derivative aux{*this, i};
      return Genfun::FunctionNoop( &aux );
    }

    Adapter3DoubleFunction::Adapter3DoubleFunction( Adapter3DoubleFunction::Function func ) : m_func( func ) {}

    double Adapter3DoubleFunction::operator()( double x ) const { return m_func( x, 0, 0 ); }

    double Adapter3DoubleFunction::operator()( const Genfun::Argument& x ) const { return m_func( x[0], x[1], x[2] ); }

    double Adapter3DoubleFunction::operator()( const double x, const double y, const double z ) const
    {
      return m_func( x, y, z );
    }

    /// Derivatives
    Genfun::Derivative Adapter3DoubleFunction::partial( unsigned int i ) const
    {
      if ( i >= 3 ) {
        GaudiMath::Constant aux{0, 3};
        return Genfun::FunctionNoop( &aux );
      }
      GaudiMath::Derivative aux{*this, i};
      return Genfun::FunctionNoop( &aux );
    }

    // ========================================================================

    // =======================================================================
    /** constructor from the trivial function
     *  @param func pointer to trivial function
     */
    // ========================================================================
    SimpleFunction::SimpleFunction( SimpleFunction::Function1 func ) : m_func( func ) {}
    // ========================================================================

    // ========================================================================
    /** constructor from the simple function with array-like argument
     *  @param func pointer to trivial function
     *  @param dim  dimension of the argument
     */
    // ========================================================================
    SimpleFunction::SimpleFunction( SimpleFunction::Function2 func, const size_t dim ) : m_func( func ), m_arg( dim, 0 )
    {
    }
    // ========================================================================

    // ========================================================================
    /** constructor from the simple function with array-like argument
     *  @param func pointer to trivial function
     *  @param dim  dimension of the argument
     */
    // ========================================================================
    SimpleFunction::SimpleFunction( SimpleFunction::Function3 func, const size_t dim ) : m_func( func ), m_arg( dim, 0 )
    {
    }
    // ========================================================================

    // ========================================================================
    /// Derivatives
    // ========================================================================
    Genfun::Derivative SimpleFunction::partial( unsigned int i ) const
    {
      const auto dim = dimensionality();
      if ( i >= dim ) {
        GaudiMath::Constant aux{0, dim};
        return Genfun::FunctionNoop( &aux );
      }
      GaudiMath::Derivative aux{*this, i};
      return Genfun::FunctionNoop( &aux );
    }

    // ========================================================================
    /// Function value
    // ========================================================================
    double SimpleFunction::operator()( double value ) const
    {
      return ::detail::dispatch_variant( m_func, [&]( const Function1& fun ) { return ( *fun )( value ); },
                                         [&]( const Function2& fun ) {
                                           m_arg[0] = value;
                                           std::fill( std::next( m_arg.begin() ), m_arg.end(), 0.0 );
                                           return ( *fun )( m_arg.data() );
                                         },
                                         [&]( const Function3& fun ) {
                                           m_arg[0] = value;
                                           std::fill( std::next( m_arg.begin() ), m_arg.end(), 0.0 );
                                           return ( *fun )( m_arg );
                                         } );
    }
    // ========================================================================

    // ========================================================================
    /// Function value
    // ========================================================================
    double SimpleFunction::operator()( const Argument& argument ) const
    {
      return ::detail::dispatch_variant( m_func, [&]( const Function1& f ) { return ( *f )( argument[0] ); },
                                         [&]( const Function2& f ) {
                                           for ( size_t i = 0; i < m_arg.size(); ++i ) {
                                             m_arg[i] = argument[i];
                                           }
                                           return ( *f )( m_arg.data() );
                                         },
                                         [&]( const Function3& f ) {
                                           for ( size_t i = 0; i < m_arg.size(); ++i ) {
                                             m_arg[i] = argument[i];
                                           }
                                           return ( *f )( m_arg );
                                         } );
    }

  } // end of namespace GaudiMathImplementation
} // end of namespace Genfun

// ============================================================================
// The END
// ============================================================================
