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
  if ( m_alg ) { m_alg -> addRef() ; }
  if ( m_svc ) { m_svc -> addRef() ; }
  if ( m_svc && m_alg ) { m_svc->setCurrentAlg ( m_alg ).ignore() ; }
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
  if ( m_alg ) { m_alg -> addRef() ; }
  if ( m_svc ) { m_svc -> addRef() ; }
  if ( m_svc && m_alg ) { m_svc->setCurrentAlg ( m_alg ).ignore() ; }
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
  if ( m_svc &&  m_alg )
  { m_svc->unSetCurrentAlg ( m_alg ).ignore() ; }
  if ( m_svc ) { m_svc -> release () ; }
  if ( m_alg ) { m_alg -> release () ; }
}
// ============================================================================


// ============================================================================
// The END
// ============================================================================

