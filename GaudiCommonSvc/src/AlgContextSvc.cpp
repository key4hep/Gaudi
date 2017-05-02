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
#include "GaudiKernel/ConcurrencyFlags.h"

// ============================================================================
/** @file
 *  Implementation firl for class AlgContextSvc
 *  @author ATLAS Collaboration
 *  @author modified by Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @author modified by Sami Kama
 *  @date 2017-03-17 (modified)
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
  auto numSlots=Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents();
  numSlots=(1>numSlots)?1:numSlots;
  if(numSlots>1000){
    warning()<<"Num Slots are greater than 1000. Is this correct? numSlots="<<
      numSlots<<endmsg;
    numSlots=1000;
    warning()<<"Setting numSlots to "<<numSlots<<endmsg;
  }
  m_inEvtLoop.resize(numSlots,0);

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

// implementation of start
// needs to be removed once we have a proper service 
// for getting configuration information at initialization time
// S. Kama 
StatusCode AlgContextSvc::start(){
  auto sc=Service::start();
  auto numSlots=Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents();
  numSlots=(1>numSlots)?1:numSlots;
  if(numSlots>1000){
    warning()<<"Num Slots are greater than 1000. Is this correct? numSlots="<<
      numSlots<<endmsg;
    numSlots=1000;
  }
  m_inEvtLoop.resize(numSlots,0);
  
  return sc;
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
  if(!m_bypassInc){
    auto currSlot=a->getContext().slot();
    if(currSlot==EventContext::INVALID_CONTEXT_ID)currSlot=0;
    if(!m_inEvtLoop[currSlot]) return StatusCode::SUCCESS;
  }
  // check whether thread-local algorithm list already exists
  // if not, create it
  if ( ! m_algorithms.get()) {
    m_algorithms.reset( new IAlgContextSvc::Algorithms() );
  }
  if(a->type()!="IncidentProcAlg")  m_algorithms->push_back ( a ) ;

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

  if ( !a ){
    warning() << "IAlgorithm* points to NULL" << endmsg ;
    return StatusCode::RECOVERABLE ;
  }

  if(!m_bypassInc){
    auto currSlot=a->getContext().slot();
    if(currSlot==EventContext::INVALID_CONTEXT_ID) currSlot=0;
    if(!m_inEvtLoop[currSlot]) return StatusCode::SUCCESS;  
  }
  if(a->type()!="IncidentProcAlg"){
    if ( m_algorithms->empty() || m_algorithms->back() != a ){
      error() << "Algorithm stack is invalid" << endmsg ;
      return StatusCode::FAILURE ;
    }
    m_algorithms->pop_back() ;
  }
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
void AlgContextSvc::handle ( const Incident& inc ) {
  //some false sharing is possible but it should be negligable
  auto currSlot=inc.context().slot();
  if (currSlot==EventContext::INVALID_CONTEXT_ID){
    currSlot=0;
  }
  if(inc.type()=="BeginEvent"){
    m_inEvtLoop[currSlot]=1;
  }else if(inc.type()=="EndEvent"){
    m_inEvtLoop[currSlot]=0;
  }

  // This check is invalidated with RTTI AlgContext object.
  // Whole service needs to be rewritten. Commenting the error until then
  // to prevent test failures.
  // if ( m_algorithms.get() && !m_algorithms->empty() ) {
  //   //skip incident processing algorithm endevent incident
  //   if((m_algorithms->size()!=1) || (m_algorithms->back()->type()!="IncidentProcAlg")){
  //     error() << "Non-empty stack of algorithms #"
  // 	      << m_algorithms->size() << endmsg ;
  //   }
  // }
}
// ============================================================================
// ============================================================================
/// The END
// ============================================================================
