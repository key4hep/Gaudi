// $Id: EqSolver.h,v 1.3 2006/11/30 10:40:53 mato Exp $
// ============================================================================
#ifndef ALGTOOLS_EQSOLVER_H
#define ALGTOOLS_EQSOLVER_H 1
// Include files
// from CLHEP
#include "CLHEP/GenericFunctions/GenericFunctions.hh"
#include "CLHEP/GenericFunctions/Argument.hh"
#include "CLHEP/GenericFunctions/AbsFunction.hh"
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
//from GSL
#include "gsl/gsl_multiroots.h"

// local
#include "GaudiGSL/IEqSolver.h"

/// forward declaration
template <class TOOL> class ToolFactory;

/** @class  EqSolver EqSolver.h
 *  The simplest concrete implementation of IEqSolver interface
 *  @see    GaudiGSL/IEqSolver.h
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 *  @date   2003-07-07
 */

class EqSolver : public extends1<GaudiTool, IEqSolver> {
  // friend factory for instantiation
  friend class ToolFactory<EqSolver> ;
public:
  typedef std::vector<Equations>   Jacobi ;
public:

  /** Solving nonlinear system with N equations in N unknowns
   *  of the function "GenFunc"
   *  @see IEqSolver.h
   *  @return StatusCode
   */
  virtual StatusCode solver( const Equations& funcs ,
                             Arg&             arg   ) const;

  /// Overriding initialize
  virtual StatusCode initialize ();
  virtual StatusCode finalize   ();

  virtual ~EqSolver( ); ///< Destructor

  class  EqSolverMisc
  {
  public:
    /** standard constructor
     *
     */
    EqSolverMisc ( const Equations& funcs ,
                   Arg&             arg   ) ;
    // destructor
    ~EqSolverMisc ();
  public:
    inline const Arg&       argument  () const { return m_argum   ; }
    inline       Arg&       argument  ()       { return m_argum   ; }
    inline const Equations* equations () const { return m_eqs     ; }
    inline const Jacobi&    jacobi    () const { return m_jac     ; }
  private:
    // default constructor is disabled
    EqSolverMisc () ;
    // copy constructor is disabled
    EqSolverMisc           ( const EqSolverMisc& );
    // assignment operator is disabled
    EqSolverMisc& operator=( const EqSolverMisc& );
  private:
    Arg              m_argum ;
    const Equations* m_eqs   ;
    Jacobi           m_jac   ;
  };


  /** Standard constructor
   *  @see GaudiTool
   *  @param type    tool type
   *  @param name    tool name
   *  @param parent  parent of the tool
   */
  EqSolver( const std::string& type,
            const std::string& name,
            const IInterface* parent);

private:

  /// default constructor is private
  EqSolver();
  /// copy constructor is private
  EqSolver           ( const EqSolver& );
  /// assignment operator is
  EqSolver& operator=( const EqSolver& );

private:

  std::string m_algType         ;
  double      m_max_iter        ;
  double      m_norm_residual   ;
  const gsl_multiroot_fdfsolver_type* m_type ;

};

// ============================================================================
// The END
// ============================================================================
#endif // ALGTOOLS_EQSOLVER_H
// ============================================================================
