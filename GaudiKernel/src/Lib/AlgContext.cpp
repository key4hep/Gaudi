// $Id: AlgContext.cpp,v 1.1 2007/05/24 13:48:50 hmd Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.1 $
// ============================================================================
// $Log: AlgContext.cpp,v $
// Revision 1.1  2007/05/24 13:48:50  hmd
// ( Vanya Belyaev) patch #1171. The enhancement of existing Algorithm Context Service
//    is the primary goal of the proposed patch. The existing
//    AlgContextSvc is not safe with respect to e.g. Data-On-Demand
//    service or to operations with subalgorithms. The patched service
//    essentially implements the queue of executing algorithms, thus the
//    problems are eliminiated. In addition the enriched interface
//    provides the access to the whole queue of executing algorithms.
//
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IAlgorithm.h"
// ============================================================================
/** @file
 *  Implementation file for class Gaudi::Utils::AlgContext
 *  @date 2007-05-17
 *  @author Vanya BELYAEV
 */
// ============================================================================
// virtual and protected destcructor
// ============================================================================
IAlgContextSvc::~IAlgContextSvc(){}
// ============================================================================
/* constructor from service and algorithm
 *  Internally invokes IAlgContextSvc::setCurrentAlg
 *  @see IAlgorithm
 *  @see IAlgContextSvc
 *  @param svc pointer to algorithm context service
 *  @param alg pointer to the current algorithm
 */
// ============================================================================
Gaudi::Utils::AlgContext::AlgContext
( IAlgContextSvc* svc ,
  IAlgorithm*     alg )
  : m_svc ( svc )
  , m_alg ( alg )
{
  if ( 0 != m_alg ) { m_alg -> addRef() ; }
  if ( 0 != m_svc ) { m_svc -> addRef() ; }
  if ( 0 != m_svc && 0 != m_alg ) { m_svc->setCurrentAlg ( m_alg ).ignore() ; }
}
// ============================================================================
/*  constructor from service and algorithm
 *  Internally invokes IAlgContextSvc::setCurrentAlg
 *  @see IAlgorithm
 *  @see IAlgContextSvc
 *  @param alg pointer to the current algorithm
 *  @param svc pointer to algorithm context service
 */
// ============================================================================
Gaudi::Utils::AlgContext::AlgContext
( IAlgorithm*     alg ,
  IAlgContextSvc* svc )
  : m_svc ( svc )
  , m_alg ( alg )
{
  if ( 0 != m_alg ) { m_alg -> addRef() ; }
  if ( 0 != m_svc ) { m_svc -> addRef() ; }
  if ( 0 != m_svc && 0 != m_alg ) { m_svc->setCurrentAlg ( m_alg ).ignore() ; }
}
// ============================================================================
/*  destructor
 *  Internally invokes IAlgContextSvc::unSetCurrentAlg
 *  @see IAlgorithm
 *  @see IAlgContextSvc
 */
// ============================================================================
Gaudi::Utils::AlgContext::~AlgContext()
{
  if ( 0 != m_svc && 0 != m_alg )
  { m_svc->unSetCurrentAlg ( m_alg ).ignore() ; }
  if ( 0 != m_svc ) { m_svc -> release () ; m_svc = 0 ; }
  if ( 0 != m_alg ) { m_alg -> release () ; m_alg = 0 ; }
}
// ============================================================================


// ============================================================================
// The END
// ============================================================================

