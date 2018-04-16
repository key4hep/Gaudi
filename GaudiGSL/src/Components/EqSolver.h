#ifndef ALGTOOLS_EQSOLVER_H
#define ALGTOOLS_EQSOLVER_H 1
// Include files
// from CLHEP
#include "CLHEP/GenericFunctions/AbsFunction.hh"
#include "CLHEP/GenericFunctions/Argument.hh"
#include "CLHEP/GenericFunctions/GenericFunctions.hh"
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
// from GSL
#include "gsl/gsl_multiroots.h"

// local
#include "GaudiGSL/IEqSolver.h"

/** @class  EqSolver EqSolver.h
 *  The simplest concrete implementation of IEqSolver interface
 *  @see    GaudiGSL/IEqSolver.h
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 *  @date   2003-07-07
 */

class EqSolver : public extends<GaudiTool, IEqSolver>
{
public:
  typedef std::vector<Equations> Jacobi;

public:
  /** Solving nonlinear system with N equations in N unknowns
   *  of the function "GenFunc"
   *  @see IEqSolver.h
   *  @return StatusCode
   */
  StatusCode solver( const Equations& funcs, Arg& arg ) const override;

  /// Overriding initialize
  StatusCode initialize() override;

  class EqSolverMisc
  {
  public:
    /** standard constructor
     *
     */
    EqSolverMisc( const Equations& funcs, Arg& arg );
    // destructor
    ~EqSolverMisc();

  public:
    inline const Arg&       argument() const { return m_argum; }
    inline Arg&             argument() { return m_argum; }
    inline const Equations* equations() const { return m_eqs; }
    inline const Jacobi&    jacobi() const { return m_jac; }

  private:
    // default constructor is disabled
    EqSolverMisc() = delete;
    // copy constructor is disabled
    EqSolverMisc( const EqSolverMisc& ) = delete;
    // assignment operator is disabled
    EqSolverMisc& operator=( const EqSolverMisc& ) = delete;

  private:
    Arg              m_argum;
    const Equations* m_eqs;
    Jacobi           m_jac;
  };

  /// Inherited constructor
  using extends::extends;

private:
  /// default constructor is private
  EqSolver() = delete;
  /// copy constructor is private
  EqSolver( const EqSolver& ) = delete;
  /// assignment operator is
  EqSolver& operator=( const EqSolver& ) = delete;

private:
  Gaudi::Property<std::string> m_algType{this, "Algorithm", "fdfsolver_hybridsj",
                                         "type of the algorithm for root finding"};
  Gaudi::Property<double> m_max_iter{this, "Iteration", 1000, "maximum of iteration"};
  Gaudi::Property<double> m_norm_residual{this, "Residual", 1.0e-7, "absolute error bound for the residual value"};

  const gsl_multiroot_fdfsolver_type* m_type = nullptr;
};

// ============================================================================
// The END
// ============================================================================
#endif // ALGTOOLS_EQSOLVER_H
// ============================================================================
