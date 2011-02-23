// $Id: GetAlgs.cpp,v 1.1 2007/09/25 16:12:41 marcocle Exp $
// ============================================================================
// Include files
// ============================================================================
// STD&STL
// ============================================================================
#include <algorithm>
#include <functional>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IAlgorithm.h"
// ============================================================================
// GaudiAlg 
// ============================================================================
#include "GaudiAlg/GetAlg.h"
#include "GaudiAlg/GetAlgs.h"
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/GaudiHistoAlg.h"
#include "GaudiAlg/GaudiTupleAlg.h"
#include "GaudiAlg/GaudiSequencer.h"
#include "GaudiAlg/Sequencer.h"
// ============================================================================
/** @file 
 *  Implementation file for the functions from the file GaudiKernel/GetAlgs.h
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ============================================================================
namespace 
{
  template <class TYPE>
  TYPE* getAlg (  const IAlgContextSvc* svc ) 
  {
    if ( 0 == svc            ) { return 0 ; }  // RETURN
    typedef IAlgContextSvc::Algorithms ALGS ;
    const ALGS& algs = svc->algorithms() ;
    ALGS::const_reverse_iterator it = std::find_if 
      ( algs.rbegin ()                        , 
        algs.rend   ()                        , 
        Gaudi::Utils::AlgTypeSelector<TYPE>() ) ;
    if ( algs.rend() == it  ) { return 0 ; }  // RETURN 
    IAlgorithm* alg = *it ;
    return dynamic_cast<TYPE*>(  alg ) ;      // RETURN
  }
}
// ============================================================================
/*  simple function to extract the last active
 *   GaudiAlgorithm from the context
 *
 *  @code 
 * 
 *  // get the context service:
 *  const IAlgContextSvc* svc = ... ;
 *  
 *  GaudiAlgorithm* ga = getGaudiAlg ( svc ) ;
 *
 *  @endcode 
 * 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ============================================================================
GaudiAlgorithm* 
Gaudi::Utils::getGaudiAlg       ( const IAlgContextSvc* svc ) 
{ return getAlg<GaudiAlgorithm> ( svc ) ; }
// ============================================================================
/*  simple function to extract the last active
 *   GaudiHistoAlg from the context
 *
 *  @code 
 * 
 *  // get the context service:
 *  const IAlgContextSvc* svc = ... ;
 *  
 *  GaudiHistoAlg* ha = getHistoAlg ( svc ) ;
 *
 *  @endcode 
 * 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ============================================================================
GaudiHistoAlg*  
Gaudi::Utils::getHistoAlg       ( const IAlgContextSvc* svc ) 
{ return getAlg<GaudiHistoAlg> ( svc ) ; }
// ============================================================================
/* simple function to extract the last active
 *   GaudiTupleAlg from the context
 *
 *  @code 
 * 
 *  // get the context service:
 *  const IAlgContextSvc* svc = ... ;
 *  
 *  GaudiTupleAlg* ta = getTupleAlg ( svc ) ;
 *
 *  @endcode 
 * 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ============================================================================
GaudiTupleAlg*  
Gaudi::Utils::getTupleAlg       ( const IAlgContextSvc* svc )
{ return getAlg<GaudiTupleAlg> ( svc ) ; }
// ============================================================================
/*  simple function to extract the last active
 *   GaudiSequencer from the context
 *
 *  @code 
 * 
 *  // get the context service:
 *  const IAlgContextSvc* svc = ... ;
 *  
 *  GaudiSequencer* sa = getGaudiSequencer ( svc ) ;
 *
 *  @endcode 
 * 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ============================================================================
GaudiSequencer* 
Gaudi::Utils::getGaudiSequencer ( const IAlgContextSvc* svc ) 
{ return getAlg<GaudiSequencer> ( svc ) ; }
// ============================================================================
/* simple function to extract the last active
 *   Sequencer from the context
 *
 *  @code 
 * 
 *  // get the context service:
 *  const IAlgContextSvc* svc = ... ;
 *  
 *  Sequencer* sa = getSequencerAlg ( svc ) ;
 *
 *  @endcode 
 * 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
Sequencer*      
Gaudi::Utils::getSequencerAlg   ( const IAlgContextSvc* svc ) 
{ return getAlg<Sequencer> ( svc ) ; }
// ========================================================================
/* simple function to extract the last active
 *   "Sequencer" () GaudiSequencer or Sequencer) 
 *   from the context
 *
 *  @code 
 * 
 *  // get the context service:
 *  const IAlgContextSvc* svc = ... ;
 *  
 *  IAlgorithm* a = getSequencer ( svc ) ;
 *
 *  @endcode 
 * 
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ========================================================================
IAlgorithm*     
Gaudi::Utils::getSequencer ( const IAlgContextSvc* svc ) 
{
  if ( 0 == svc            ) { return 0 ; }  // RETURN
  //
  typedef IAlgContextSvc::Algorithms ALGS ;
  const ALGS& algs = svc->algorithms() ;
  AlgTypeSelector<GaudiSequencer> sel1 ;
  AlgTypeSelector<Sequencer>      sel2 ;
  for ( ALGS::const_reverse_iterator it = algs.rbegin() ; 
        algs.rend() != it ; ++it ) 
  {
    if ( sel1 ( *it ) ) { return *it ; }
    if ( sel2 ( *it ) ) { return *it ; }
  }
  return 0 ;                                    // RETURN ;
}
// ========================================================================


// ============================================================================
// The END 
// ============================================================================
