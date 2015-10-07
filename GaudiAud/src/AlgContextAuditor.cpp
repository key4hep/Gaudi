#include <cassert>

#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/MsgStream.h"

#include "AlgContextAuditor.h"

// ============================================================================
/** @file
 *  Implementation file for class AlgContexAuditor
 *  @author M. Shapiro, LBNL
 *  @author modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 */
// ============================================================================
namespace
{
  /** make a safe cast using "smart interface"
   *  @see  INamedInterface
   *  @see  IAlgorithm
   *  @see  SmartIF
   *  @see  IInterface::queryInterface
   *  @param  ni pointer to INamedInterface
   *  @return pointer to IAlgorithm
   */
  inline IAlgorithm* toAlg ( IInterface* ni )
  {
    if ( 0 == ni ) { return 0 ; }
    SmartIF<IAlgorithm> alg ( ni ) ;
    return alg ;
  }
}
// ============================================================================
// mandatory auditor factory, needed for instantiation
// ============================================================================
DECLARE_COMPONENT(AlgContextAuditor)
// ============================================================================
// standard constructor @see Auditor
// ============================================================================
AlgContextAuditor::AlgContextAuditor
( const std::string& name ,
  ISvcLocator*       pSvc )
  : Auditor( name , pSvc )
{}
// ============================================================================
// standard initialization, see @IAuditor
// ============================================================================
StatusCode AlgContextAuditor::initialize()
{
  // initialize the base class
  StatusCode sc = Auditor::initialize() ;
  if ( sc.isFailure() ) { return sc ; }                           // RETURN
  m_svc = service(  "AlgContextSvc" , true ) ;
  if ( !m_svc ) {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::ERROR << "Invalid pointer to IAlgContextSvc" << endmsg ;
    return StatusCode::FAILURE ;           // RETURN
  }
  return StatusCode::SUCCESS ;
}
// ============================================================================
// standard finalization, see @IAuditor
// ============================================================================
StatusCode AlgContextAuditor::finalize ()
{
  m_svc.reset();
  // finalize the base class
  return Auditor::finalize () ;
}
// ============================================================================
void AlgContextAuditor::beforeInitialize ( INamedInterface*  a ) {
  if ( m_svc ) {
    IAlgorithm* alg = toAlg(a);
    if (alg) m_svc -> setCurrentAlg ( alg ).ignore() ;
  }
}
// ============================================================================
void AlgContextAuditor::afterInitialize  ( INamedInterface*  a ) {
  if ( m_svc ) {
    IAlgorithm* alg = toAlg(a);
    if (alg) m_svc -> unSetCurrentAlg ( alg ).ignore() ;
  }
}
// ============================================================================
void AlgContextAuditor::beforeFinalize   ( INamedInterface*  a ) {
  if ( m_svc ) {
    IAlgorithm* alg = toAlg(a);
    if (alg) m_svc -> setCurrentAlg ( alg ).ignore() ;
  }
}
// ============================================================================
void AlgContextAuditor::afterFinalize    ( INamedInterface*  a ) {
  if ( m_svc ) {
    IAlgorithm* alg = toAlg(a);
    if (alg) m_svc -> unSetCurrentAlg ( alg ).ignore() ;
  }
}
// ============================================================================
void AlgContextAuditor::beforeExecute    ( INamedInterface*  a ) {
  if ( m_svc ) {
    IAlgorithm* alg = toAlg(a);
    if (alg) m_svc -> setCurrentAlg ( alg ).ignore() ;
  }
}
// ============================================================================
void AlgContextAuditor::afterExecute     ( INamedInterface*  a       ,
                                           const StatusCode& /* s */ ) {
  if ( m_svc ) {
    IAlgorithm* alg = toAlg(a);
    if (alg) m_svc -> unSetCurrentAlg ( alg ).ignore() ;
  }
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================

