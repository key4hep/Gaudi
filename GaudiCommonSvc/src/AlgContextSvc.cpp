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
// Standard Constructor
// ============================================================================
AlgContextSvc::AlgContextSvc
( const std::string& name ,
  ISvcLocator*       svc  )
  : base_class ( name , svc  )
{
  declareProperty ( "Check" , m_check , "Flag to perform more checks" );
}
// ============================================================================
// standard initialization of the service
// ============================================================================
StatusCode AlgContextSvc::initialize ()
{
  // Initialize the base class
  StatusCode sc = Service::initialize () ;
  if ( sc.isFailure () ) { return sc ; }

  // perform more checks?
  if ( m_check )
  {
    sc = Service::service ( "IncidentSvc" , m_inc , true ) ;
    if ( sc.isFailure() )
    {
      MsgStream log ( msgSvc() , name() )  ;
      log << MSG::ERROR << "Could not locate 'IncidentSvc'" << sc << endmsg ;
      return sc ;                                               // RETURN
    }
    if ( !m_inc )
    {
      MsgStream log ( msgSvc() , name() ) ;
      log << MSG::ERROR << "Invalid pointer to IIncindentSvc" << endmsg ;
      return StatusCode::FAILURE ;                               // RETURN
    }
    m_inc -> addListener ( this , IncidentType::BeginEvent ) ;
    m_inc -> addListener ( this , IncidentType::EndEvent   ) ;
  }
  if ( m_algorithms.get() && !m_algorithms->empty() )
  {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::WARNING
        << "Non-empty stack of algorithms #"
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
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::WARNING
        << "Non-empty stack of algorithms #"
        << m_algorithms->size() << endmsg ;
  }
  // Incident Service
  if ( m_inc )
  {
    m_inc -> removeListener ( this ) ;
    m_inc -> release() ;
    m_inc = nullptr ;
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
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::WARNING << "IAlgorithm* points to NULL" << endmsg ;
    return StatusCode::RECOVERABLE ;                              // RETURN
  }
  // check whether thread-local algorithm list already exists
  // if not, create it
  if ( ! m_algorithms.get()) {
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
  if ( ! m_algorithms.get()) {
    m_algorithms.reset( new IAlgContextSvc::Algorithms() );
  }

  if ( !a )
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
  return (m_algorithms.get() && ! m_algorithms->empty())
    ? m_algorithms->back()
    : nullptr;
}
// ============================================================================
// handle incident @see IIncidentListener
// ============================================================================
void AlgContextSvc::handle ( const Incident& ) {
  if ( m_algorithms.get() && !m_algorithms->empty() ) {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::ERROR
        << "Non-empty stack of algorithms #"
        << m_algorithms->size() << endmsg ;
  }
}
// ============================================================================
// ============================================================================
/// The END
// ============================================================================
