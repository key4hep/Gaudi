// $Id: FuncMinimum.h,v 1.3 2006/11/30 10:40:53 mato Exp $
// ============================================================================
#ifndef ALGTOOLS_FUNCMINIMUM_H
#define ALGTOOLS_FUNCMINIMUM_H 1
// Include files
// from CLHEP
#include "CLHEP/GenericFunctions/GenericFunctions.hh"
#include "CLHEP/GenericFunctions/Argument.hh"
#include "CLHEP/GenericFunctions/AbsFunction.hh"
#include "CLHEP/Matrix/SymMatrix.h"
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
//from GSL
#include "gsl/gsl_multimin.h"

// local
#include "GaudiGSL/IFuncMinimum.h"

/// forward declaration
template <class TOOL> class ToolFactory;

/** @class  FuncMinimum FuncMinimum.h
 *  The simplest concrete implementation of IFuncMinimum interface
 *  @see    GaudiGSL/IFuncMinimum.h
 *  @author Kirill Miklyaev kirillm@iris1.itep.ru
 *  @date   2002-09-14
 */

class FuncMinimum : public extends1<GaudiTool, IFuncMinimum> {
  // friend factory for instantiation
  friend class ToolFactory<FuncMinimum> ;
public:
  typedef std::vector<const GenFunc*> Gradient;
public:

  /** Find minimum of the function "GenFunc"
   *  @see IFuncMinimum.h
   *  @return StatusCode
   */
  virtual StatusCode minimum( const GenFunc& func  ,
                              Arg&           arg   ) const;

  /** Find minimum of the function "GenFunc"
   *  @see IFuncMinimum.h
   *  @return StatusCode
   */
  virtual StatusCode minimum( const GenFunc& func  ,
                              Arg&           arg   ,
                              Covariance&    covar ) const;

  /// Overriding initialize
  virtual StatusCode initialize ();
  virtual StatusCode finalize   ();

  virtual ~FuncMinimum( ); ///< Destructor

  class FuncMinimumMisc
  {
  public:
    /** standard constructor
     *
     */
    FuncMinimumMisc (const GenFunc& func ,
                     Arg&           arg  );
    // destructor
    ~FuncMinimumMisc () ;

  public:
    inline const Arg&       argument () const { return m_argum ;}
    inline       Arg&       argument ()       { return m_argum ;}
    inline const GenFunc*   equation () const { return m_eq    ;}
    inline const Gradient&  gradient () const { return m_grad  ;}
  private:
    // default constructor is disabled
    FuncMinimumMisc () ;
    // copy constructor is disabled
    FuncMinimumMisc           ( const FuncMinimumMisc& );
    // assignment operator is disabled
    FuncMinimumMisc& operator=( const FuncMinimumMisc& );
  private:
    Arg              m_argum ;
    const GenFunc*   m_eq    ;
    Gradient         m_grad  ;
  };


  /** Standard constructor
   *  @see GaudiTool
   *  @param type    tool type
   *  @param name    tool name
   *  @param parent  parent of the tool
   */
  FuncMinimum( const std::string& type,
               const std::string& name,
               const IInterface* parent);

private:

  /// default constructor is private
  FuncMinimum();
  /// copy constructor is private
  FuncMinimum           ( const FuncMinimum& );
  /// assignment operator is
  FuncMinimum& operator=( const FuncMinimum& );

private:

  std::string m_algType         ;
  double      m_max_iter        ;
  double      m_norm_gradient   ;
  double      m_step_size       ;
  double      m_tol             ;
  const gsl_multimin_fdfminimizer_type* m_type ;
};

// ============================================================================
// The END
// ============================================================================
#endif // ALGTOOLS_FUNCMINIMUM_H
// ============================================================================
