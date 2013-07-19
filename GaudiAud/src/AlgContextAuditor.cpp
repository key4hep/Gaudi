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
  , m_svc   ( 0    )
{}
// ============================================================================
// destructor
// ============================================================================
AlgContextAuditor::~AlgContextAuditor() {}
// ============================================================================
// standard initialization, see @IAuditor
// ============================================================================
StatusCode AlgContextAuditor::initialize()
{
  // initialize the base class
  StatusCode sc = Auditor::initialize() ;
  if ( sc.isFailure() ) { return sc ; }                           // RETURN
  if ( 0 != m_svc ) { m_svc -> release() ; m_svc = 0 ; }
  sc = Auditor::service ( "AlgContextSvc" , m_svc , true ) ;
  if ( sc.isFailure() )
  {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::ERROR << "Unable to locate 'AlgContextSvc'" << sc << endmsg ;
    m_svc = 0 ;
    return sc ;  // RETURN
  }
  if ( 0 == m_svc     )
  {
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
  if ( 0 != m_svc ) { m_svc-> release() ; m_svc = 0 ; }
  // finalize the base class
  return Auditor::finalize () ;
}
// ============================================================================
void AlgContextAuditor::beforeInitialize ( INamedInterface*  a ) {
  if ( 0 != m_svc ) {
    IAlgorithm* alg = toAlg(a);
    if (alg != 0) m_svc -> setCurrentAlg ( alg ).ignore() ;
  }
}
// ============================================================================
void AlgContextAuditor::afterInitialize  ( INamedInterface*  a ) {
  if ( 0 != m_svc ) {
    IAlgorithm* alg = toAlg(a);
    if (alg != 0) m_svc -> unSetCurrentAlg ( alg ).ignore() ;
  }
}
// ============================================================================
void AlgContextAuditor::beforeFinalize   ( INamedInterface*  a ) {
  if ( 0 != m_svc ) {
    IAlgorithm* alg = toAlg(a);
    if (alg != 0) m_svc -> setCurrentAlg ( alg ).ignore() ;
  }
}
// ============================================================================
void AlgContextAuditor::afterFinalize    ( INamedInterface*  a ) {
  if ( 0 != m_svc ) {
    IAlgorithm* alg = toAlg(a);
    if (alg != 0) m_svc -> unSetCurrentAlg ( alg ).ignore() ;
  }
}
// ============================================================================
void AlgContextAuditor::beforeExecute    ( INamedInterface*  a ) {
  if ( 0 != m_svc ) {
    IAlgorithm* alg = toAlg(a);
    if (alg != 0) m_svc -> setCurrentAlg ( alg ).ignore() ;
  }
}
// ============================================================================
void AlgContextAuditor::afterExecute     ( INamedInterface*  a       ,
                                           const StatusCode& /* s */ ) {
  if ( 0 != m_svc ) {
    IAlgorithm* alg = toAlg(a);
    if (alg != 0) m_svc -> unSetCurrentAlg ( alg ).ignore() ;
  }
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================

