#define  GAUDIALG_GAUDIALGORITHM_CPP 1
// ============================================================================
// include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ThreadLocalContext.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================
/** @file
 *
 *  Implementation file for the class GaudiAlgorithm
 *
 *  @see GaudiAlgorithm
 *  @see Algorithm
 *  @see IAlgorithm
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @author Christopher Jones  Christopher.Rob.Jones@cern.ch
 *  @date 30/06/2001
 */
// ============================================================================
// templated methods
// ============================================================================
#include "GaudiCommon.icpp"
// ============================================================================
template class GaudiCommon<Algorithm>;
// ============================================================================
// Standard algorithm like constructor
// ============================================================================
GaudiAlgorithm::GaudiAlgorithm ( const std::string&  name        ,
                                 ISvcLocator*        pSvcLocator )
  : GaudiCommon<Algorithm> ( name , pSvcLocator )
{
  setProperty ( "RegisterForContextService" , true ).ignore() ;
}
// ============================================================================
// standard initialization method
// ============================================================================
StatusCode GaudiAlgorithm::initialize()
{
  // initialize the base class
  const StatusCode sc = GaudiCommon<Algorithm>::initialize() ;
  if ( sc.isFailure() ) { return sc; }

  // Add any customisations here, that cannot go in GaudiCommon

  // return
  return sc;
}
// ============================================================================
// standard finalization method
// ============================================================================
StatusCode GaudiAlgorithm::finalize()
{
  if ( msgLevel(MSG::DEBUG) )
    debug() << "Finalize base class GaudiAlgorithm" << endmsg;

  // reset pointers
  m_evtColSvc.reset() ;

  // finalize the base class and return
  return GaudiCommon<Algorithm>::finalize() ;
}
// ============================================================================
// standard execution method
// ============================================================================
StatusCode GaudiAlgorithm::execute()
{
  return Error ( "Default GaudiAlgorithm execute method called !!" ) ;
}
// ============================================================================
// The standard event collection service
// ============================================================================
SmartIF<INTupleSvc>& GaudiAlgorithm::evtColSvc() const
{
  if ( !m_evtColSvc )
  { m_evtColSvc = svc< INTupleSvc > ( "EvtTupleSvc" , true ) ; }
  //
  return m_evtColSvc ;
}
// ============================================================================
/*  The generic actions for the execution.
 *  @see  Algorithm
 *  @see IAlgorithm
 *  @see Algorithm::sysExecute
 *  @return status code
 */
// ============================================================================
StatusCode GaudiAlgorithm::sysExecute (const EventContext& evtCtx)
{
  IAlgContextSvc* ctx = nullptr ;
  if ( registerContext() ) { ctx = contextSvc() ; }
  // Lock the context
  Gaudi::Utils::AlgContext cnt ( ctx , this ) ;  ///< guard/sentry

  // Do not execute if one or more of the m_vetoObjs exist in TES
  auto it = std::find_if( std::begin(m_vetoObjs), std::end(m_vetoObjs),
                          [&](const std::string& loc) { return this->exist<DataObject>(loc); } );
  if ( it != std::end(m_vetoObjs) ) {
    if ( msgLevel(MSG::DEBUG) )
      debug() << *it << " found, skipping event " << endmsg;
    return StatusCode::SUCCESS;
  }

  // Execute if m_requireObjs is empty
  // or if one or more of the m_requireObjs exist in TES
  bool doIt = m_requireObjs.empty() ||
              std::any_of( std::begin(m_requireObjs), std::end(m_requireObjs),
                           [&](const std::string& loc) { return this->exist<DataObject>(loc); } );

  // execute the generic method:
  if( doIt ) return Algorithm::sysExecute(evtCtx) ;
  return StatusCode::SUCCESS;
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
