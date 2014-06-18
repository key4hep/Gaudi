// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
// ============================================================================
// Local
// ============================================================================
#include "AlgContextSvc.h"
// ============================================================================
/** @file
 *  Implementation firl for class AlgContextSvc
 *  @author ATLAS Collaboration
 *  @author modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-03-05 (modified)
 */
// ============================================================================
/** Instantiation of a static factory class used by clients to create
 *  instances of this service
 */
DECLARE_COMPONENT(AlgContextSvc)
// ============================================================================
// Standard Constructor
// ============================================================================
AlgContextSvc::AlgContextSvc
( const std::string& name ,
  ISvcLocator*       svc  )
  : base_class ( name , svc  )
{}
// ============================================================================
// Standard Destructor
// ============================================================================
AlgContextSvc::~AlgContextSvc() {}
// ============================================================================
// standard initialization of the service
// ============================================================================
StatusCode AlgContextSvc::initialize ()
{
  // Initialize the base class
  StatusCode sc = Service::initialize () ;
  if ( sc.isFailure () ) { return sc ; }
  return StatusCode::SUCCESS ;
}
// ============================================================================
// standard finalization  of the service  @see IService
// ============================================================================
StatusCode AlgContextSvc::finalize   ()
{
  // finalize the base class
  return Service::finalize () ;
}
// ============================================================================
// set     the currently executing algorithm  ("push_back") @see IAlgContextSvc
// ============================================================================
StatusCode AlgContextSvc::setCurrentAlg  ( IAlgorithm* a )
{
  if ( 0 == a )
  {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::WARNING << "IAlgorithm* points to NULL" << endmsg ;
    return StatusCode::RECOVERABLE ;                              // RETURN
  }
  // check whether thread-local algorithm list already exists
  // if not, create it
  if ( NULL == m_algorithms.get()) {
    m_algorithms.reset( new IAlgContextSvc::Algorithms() );
  }
  m_algorithms->push_back ( a ) ;

  return StatusCode::SUCCESS ;                                    // RETURN
}
// ============================================================================
// remove the algorithm                       ("pop_back") @see IAlgContextSvc
// ============================================================================
StatusCode AlgContextSvc::unSetCurrentAlg ( IAlgorithm* a )
{
  // check whether thread-local algorithm list already exists
  // if not, create it
  if ( NULL == m_algorithms.get()) {
    m_algorithms.reset( new IAlgContextSvc::Algorithms() );
  }

  if ( 0 == a )
  {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::WARNING << "IAlgorithm* points to NULL" << endmsg ;
    return StatusCode::RECOVERABLE ;                              // RETURN
  }
    if ( m_algorithms->empty() || m_algorithms->back() != a )
   {
     MsgStream log ( msgSvc() , name() ) ;
    log << MSG::ERROR << "Algorithm stack is invalid" << endmsg ;
    return StatusCode::FAILURE ;
  }
  m_algorithms->pop_back() ;                                      // POP_BACK

  return StatusCode::SUCCESS ;
}
// ============================================================================
/// accessor to current algorithm: @see IAlgContextSvc
// ============================================================================
IAlgorithm* AlgContextSvc::currentAlg  () const
{
  return (! m_algorithms.get() && ! m_algorithms->empty())
    ? m_algorithms->back()
    : nullptr;
}
// ============================================================================

// ============================================================================
/// The END
// ============================================================================
