#include <GaudiKernel/IAlgContextSvc.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/ThreadLocalContext.h>

/** @file
 *  Implementation file for class Gaudi::Utils::AlgContext
 *  @date 2007-05-17
 *  @author Vanya BELYAEV
 */

namespace {
  // This dummy context is needed for the AlgContect constructor for single thread cases (initialize, finalize, etc.).
  static const EventContext s_dummyDefaultCtx;
} // namespace

Gaudi::Utils::AlgContext::AlgContext( IAlgorithm* alg, IAlgContextSvc* svc, const EventContext& context )
    : m_svc( svc ), m_alg( alg ), m_context( context ) {
  if ( m_svc && m_alg ) { m_svc->setCurrentAlg( m_alg.get(), m_context ).ignore(); }
}

Gaudi::Utils::AlgContext::AlgContext( IAlgorithm* alg, IAlgContextSvc* svc )
    : AlgContext( alg, svc, s_dummyDefaultCtx ) {}

Gaudi::Utils::AlgContext::~AlgContext() {
  if ( m_svc && m_alg ) { m_svc->unSetCurrentAlg( m_alg.get(), m_context ).ignore(); }
}
