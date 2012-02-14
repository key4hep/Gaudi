// $Id: AlgContextSvc.cpp,v 1.4 2007/05/24 13:49:47 hmd Exp $
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IIncidentSvc.h"
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
DECLARE_SERVICE_FACTORY(AlgContextSvc)
// ============================================================================
// Standard Constructor
// ============================================================================
AlgContextSvc::AlgContextSvc
( const std::string& name ,
  ISvcLocator*       svc  )
  : base_class ( name , svc  )
  , m_algorithms (  0   )
  , m_inc        (  0   )
  , m_check      ( true )
{
  declareProperty ( "Check" , m_check , "Flag to perform more checks" );
}
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
  // Incident Service
  if ( 0 != m_inc     )
  {
    m_inc -> removeListener ( this ) ;
    m_inc -> release() ;
    m_inc = 0 ;
  }
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
    if ( 0 == m_inc )
    {
      MsgStream log ( msgSvc() , name() ) ;
      log << MSG::ERROR << "Invalid pointer to IIncindentSvc" << endmsg ;
      return StatusCode::FAILURE ;                               // RETURN
    }
    m_inc -> addListener ( this , IncidentType::BeginEvent ) ;
    m_inc -> addListener ( this , IncidentType::EndEvent   ) ;
  }
  if ( !m_algorithms.empty() )
  {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::WARNING
        << "Non-empty stack of algorithms #"
        << m_algorithms.size() << endmsg ;
  }
  return StatusCode::SUCCESS ;
}
// ============================================================================
// standard finalization  of the service  @see IService
// ============================================================================
StatusCode AlgContextSvc::finalize   ()
{
  if ( !m_algorithms.empty() )
  {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::WARNING
        << "Non-empty stack of algorithms #"
        << m_algorithms.size() << endmsg ;
  }
  // Incident Service
  if ( 0 != m_inc     )
  {
    m_inc -> removeListener ( this ) ;
    m_inc -> release() ;
    m_inc = 0 ;
  }
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
    //
    return StatusCode::RECOVERABLE ;                              // RETURN
  }
  m_algorithms.push_back ( a ) ;
  //
  return StatusCode::SUCCESS ;                                    // RETURN
}
// ============================================================================
// remove the algorithm                       ("pop_back") @see IAlgContextSvc
// ============================================================================
StatusCode AlgContextSvc::unSetCurrentAlg ( IAlgorithm* a )
{
  if ( 0 == a )
  {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::WARNING << "IAlgorithm* points to NULL" << endmsg ;
    //
    return StatusCode::RECOVERABLE ;                              // RETURN
  }
  if ( m_algorithms.empty() || m_algorithms.back() != a )
  {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::ERROR << "Algorithm stack is invalid" << endmsg ;
    //
    return StatusCode::FAILURE ;
  }
  //
  m_algorithms.pop_back() ;                                      // POP_BACK
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
/// accessor to current algorithm: @see IAlgContextSvc
// ============================================================================
IAlgorithm* AlgContextSvc::currentAlg  () const
{ return m_algorithms.empty() ? 0 : m_algorithms.back() ; }
// ============================================================================
// handle incident @see IIncidentListener
// ============================================================================
void AlgContextSvc::handle ( const Incident& )
{
  if ( !m_algorithms.empty() )
  {
    MsgStream log ( msgSvc() , name() ) ;
    log << MSG::ERROR
        << "Non-empty stack of algorithms #"
        << m_algorithms.size() << endmsg ;
  }
}
// ============================================================================




// ============================================================================
/// The END
// ============================================================================
