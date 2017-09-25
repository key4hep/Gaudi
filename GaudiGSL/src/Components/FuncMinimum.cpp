// Include files

#include <cstdio>
#include <cstdlib>
// from Gaudi
#include "GaudiKernel/MsgStream.h"
// from GSL
#include "gsl/gsl_blas.h"
#include "gsl/gsl_errno.h"
#include "gsl/gsl_linalg.h"
#include "gsl/gsl_math.h"
#include "gsl/gsl_matrix.h"
#include "gsl/gsl_multimin.h"
#include "gsl/gsl_vector.h"

// local
#include "FuncMinimum.h"

// Handle CLHEP 2.0.x move to CLHEP namespace
namespace CLHEP
{
}
using namespace CLHEP;

/** @file
 *  Implementation file for class : FuncMinimum
 *  @see FuncMinimum.h
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 *  @date 2002-09-14
 */

// ============================================================================
// ============================================================================
FuncMinimum::FuncMinimumMisc::FuncMinimumMisc( const FuncMinimum::GenFunc& func, FuncMinimum::Arg& arg )
    : m_argum( arg ), m_eq( &func ), m_grad()
{
  const size_t N = func.dimensionality();

  for ( size_t i = 0; i < N; ++i ) {
    Genfun::GENFUNCTION fun = func.partial( i );
    m_grad.push_back( fun.clone() );
  }
}
//=============================================================================
namespace
{
  using namespace Genfun;
  /// The function which we minimize
  double fun_gsl( const gsl_vector* v, void* params )
  {
    FuncMinimum::FuncMinimumMisc* local = static_cast<FuncMinimum::FuncMinimumMisc*>( params );
    const FuncMinimum::GenFunc& eq      = *( local->equation() );
    Argument& arg                       = local->argument();

    for ( unsigned int i = 0; i < v->size; ++i ) {
      arg[i] = gsl_vector_get( v, i );
    }

    return eq( arg );
  }

  /// The gradient of the function "fun_gsl"
  void dfun_gsl( const gsl_vector* v, void* params, gsl_vector* df )
  {
    FuncMinimum::FuncMinimumMisc* local = static_cast<FuncMinimum::FuncMinimumMisc*>( params );
    const FuncMinimum::Gradient& grad   = local->gradient();
    Argument& arg                       = local->argument();

    for ( unsigned int i = 0; i < v->size; ++i ) {
      arg[i] = gsl_vector_get( v, i );
    }

    for ( unsigned int i = 0; i < df->size; ++i ) {
      Genfun::GENFUNCTION f = *( grad[i] );
      gsl_vector_set( df, i, f( arg ) );
    }
  }

  /// Compute both the function "fun_gsl"
  /// and gradient of this function "dfun_gsl" together
  void fdfun_gsl( const gsl_vector* v, void* params, double* f, gsl_vector* df )
  {
    *f = fun_gsl( v, params );
    dfun_gsl( v, params, df );
  }
}

//=============================================================================
/** Find minimum of the function "GenFunc"
 *  @see IFuncMinimum.h
 *  @return StatusCode
 */
StatusCode FuncMinimum::minimum( const IFuncMinimum::GenFunc& func, IFuncMinimum::Arg& arg ) const
//=============================================================================
{
  using namespace Genfun;

  gsl_vector_view vect = gsl_vector_view_array( &arg[0], arg.dimension() );
  FuncMinimumMisc local( func, arg );

  gsl_multimin_function_fdf function;

  function.f      = &fun_gsl;
  function.df     = &dfun_gsl;
  function.fdf    = &fdfun_gsl;
  function.n      = vect.vector.size;
  function.params = (void*)&local;

  size_t iter                             = 0;
  int status                              = 0;
  const gsl_multimin_fdfminimizer_type* T = m_type;

  gsl_multimin_fdfminimizer* s;

  s = gsl_multimin_fdfminimizer_alloc( T, vect.vector.size );

  gsl_multimin_fdfminimizer_set( s, &function, &vect.vector, m_step_size, m_tol );

  for ( iter = 0; iter < m_max_iter; ++iter ) {
    status = gsl_multimin_fdfminimizer_iterate( s );

    if ( status ) {
      return Error( "Error from gsl_multimin_fdfminimizer_iterate '" + std::string( gsl_strerror( status ) ) + "'" );
    }

    status = gsl_multimin_test_gradient( s->gradient, m_norm_gradient );

    if ( status != GSL_CONTINUE ) {
      break;
    }
  }

  for ( unsigned int i = 0; i < vect.vector.size; ++i ) {
    gsl_vector_set( &vect.vector, i, gsl_vector_get( s->x, i ) );
  }

  if ( status == GSL_SUCCESS ) {
    debug() << "We stopped in the method on the " << iter << " iteration (we have maximum " << m_max_iter
            << " iterations)" << endmsg;

    msgStream() << "The Euclidean norm of gradient = " << gsl_blas_dnrm2( s->gradient )
                << " by the absolute tolerance = " << m_norm_gradient << endmsg;
  } else if ( status == GSL_CONTINUE && iter <= m_max_iter ) {
    return Error( "Method finished with '" + std::string( gsl_strerror( status ) ) + "' error" );
  } else {
    return Error( "Method finished with '" + std::string( gsl_strerror( status ) ) + "' error" );
  }

  gsl_multimin_fdfminimizer_free( s );

  if ( status ) {
    return Error( "Method finished with '" + std::string( gsl_strerror( status ) ) + "' error" );
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
/** Find minimum of the function "GenFunc"
 *  @see IFuncMinimum.h
 *  @return StatusCode
 */
StatusCode FuncMinimum::minimum( const IFuncMinimum::GenFunc& func, IFuncMinimum::Arg& arg,
                                 IFuncMinimum::Covariance& covar ) const
//=============================================================================
{
  /// Find minimum of our function
  StatusCode sc = minimum( func, arg );

  if ( sc.isFailure() ) {
    return Error( "MINIMUM IS NOT FOUND. StatusCode =  '" + std::to_string( sc.getCode() ) + '\'', sc );
  }
  HepSymMatrix cov( arg.dimension(), 0 );
  for ( unsigned int i = 0; i < arg.dimension(); ++i ) {
    auto f = func.partial( i );
    for ( unsigned int j = i; j < arg.dimension(); ++j ) {
      auto fij = f.partial( j );
      cov( i + 1, j + 1 ) = 0.5 * fij( arg );
    }
  }

  int inv;
  covar = cov.inverse( inv );
  return inv == 0 ? StatusCode::SUCCESS : Error( "Matrix of Error is not complete successful" );
}

//=============================================================================
StatusCode FuncMinimum::initialize()

{
  StatusCode sc = GaudiTool::initialize();

  if ( sc.isFailure() ) {
    return Error( "Could not initialize base class GaudiTool", sc );
  }

  /// The algorithm for multidimensional minimization
  if ( "conjugate_fr" == m_algType ) {
    m_type = gsl_multimin_fdfminimizer_conjugate_fr;
    debug() << "Minimization algorithm to be used: "
            << "'gsl_multimin_fdfminimizer_conjugate_fr'" << endmsg;
  } else if ( "conjugate_pr" == m_algType ) {
    m_type = gsl_multimin_fdfminimizer_conjugate_pr;
    debug() << "Minimization algorithm to be used: "
            << "'gsl_multimin_fdfminimizer_conjugate_pr'" << endmsg;
  } else if ( "vector_bfgs" == m_algType ) {
    m_type = gsl_multimin_fdfminimizer_vector_bfgs;
    debug() << "Minimization algorithm to be used: "
            << "'gsl_multimin_fdfminimizer_vector_bfgs'" << endmsg;
  } else if ( "steepest_descent" == m_algType ) {
    m_type = gsl_multimin_fdfminimizer_steepest_descent;
    debug() << "Minimization algorithm to be used: "
            << "'gsl_multimin_fdfminimizer_steepest_descent'" << endmsg;
  } else {
    return Error( " Unknown algorithm type '" + m_algType + "'" );
  }

  return StatusCode::SUCCESS;
}
//=============================================================================

// Declaration of the Tool Factory
DECLARE_COMPONENT( FuncMinimum )
