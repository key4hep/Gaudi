// ============================================================================
// Include files
// ============================================================================
// Local
// ============================================================================
#include "AlgContextSvc.h"
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IIncidentSvc.h"
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
// standard initialization of the service
// ============================================================================
StatusCode AlgContextSvc::initialize ()
{
  // Initialize the base class
  StatusCode sc = Service::initialize () ;
  if ( sc.isFailure () ) { return sc ; }
  // Incident Service
  if ( m_inc     )
  {
    m_inc -> removeListener ( this ) ;
    m_inc.reset();
  }
  // perform more checks?
  if ( m_check )
  {
    m_inc = Service::service ( "IncidentSvc" , true ) ;
    if ( !m_inc )
    {
      error() << "Could not locate 'IncidentSvc'" << endmsg ;
      return StatusCode::FAILURE ;
    }
    m_inc -> addListener ( this , IncidentType::BeginEvent ) ;
    m_inc -> addListener ( this , IncidentType::EndEvent   ) ;
  }
  if ( m_algorithms.get() && !m_algorithms->empty() )
  {
    warning() << "Non-empty stack of algorithms #"
              << m_algorithms->size() << endmsg ;
  }
  return StatusCode::SUCCESS ;
}
// ============================================================================
// standard finalization  of the service  @see IService
// ============================================================================
StatusCode AlgContextSvc::finalize   ()
{
  if ( m_algorithms.get() && !m_algorithms->empty() )
  {
    warning() << "Non-empty stack of algorithms #"
              << m_algorithms->size() << endmsg ;
  }
  // Incident Service
  if ( m_inc )
  {
    m_inc -> removeListener ( this ) ;
    m_inc.reset();
  }
  // finalize the base class
  return Service::finalize () ;
}
// ============================================================================
// set     the currently executing algorithm  ("push_back") @see IAlgContextSvc
// ============================================================================
StatusCode AlgContextSvc::setCurrentAlg  ( IAlgorithm* a )
{
  if ( !a )
  {
    warning() << "IAlgorithm* points to NULL" << endmsg ;
    return StatusCode::RECOVERABLE ;
  }
  // check whether thread-local algorithm list already exists
  // if not, create it
  if ( ! m_algorithms.get()) {
    m_algorithms.reset( new IAlgContextSvc::Algorithms() );
  }
  m_algorithms->push_back ( a ) ;

  return StatusCode::SUCCESS ;
}
// ============================================================================
// remove the algorithm                       ("pop_back") @see IAlgContextSvc
// ============================================================================
StatusCode AlgContextSvc::unSetCurrentAlg ( IAlgorithm* a )
{
  // check whether thread-local algorithm list already exists
  // if not, create it
  if ( ! m_algorithms.get()) {
    m_algorithms.reset( new IAlgContextSvc::Algorithms() );
  }

  if ( !a )
  {
    warning() << "IAlgorithm* points to NULL" << endmsg ;
    return StatusCode::RECOVERABLE ;
  }
    if ( m_algorithms->empty() || m_algorithms->back() != a )
   {
    error() << "Algorithm stack is invalid" << endmsg ;
    return StatusCode::FAILURE ;
  }
  m_algorithms->pop_back() ;

  return StatusCode::SUCCESS ;
}
// ============================================================================
/// accessor to current algorithm: @see IAlgContextSvc
// ============================================================================
IAlgorithm* AlgContextSvc::currentAlg  () const
{
  return (m_algorithms.get() && ! m_algorithms->empty())
    ? m_algorithms->back()
    : nullptr;
}
// ============================================================================
// handle incident @see IIncidentListener
// ============================================================================
void AlgContextSvc::handle ( const Incident& ) {
  if ( m_algorithms.get() && !m_algorithms->empty() ) {
    //skip Incident processing algorithm endevent incident
    if((m_algorithms->size()!=1) || (m_algorithms->back()->type()!="IncidentProcAlg")){
      error() << "Non-empty stack of algorithms #"
	      << m_algorithms->size() << endmsg ;
    }
  }
}
// ============================================================================
// ============================================================================
/// The END
// ============================================================================
