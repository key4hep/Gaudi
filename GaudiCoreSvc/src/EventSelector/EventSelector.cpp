// $Id: EventSelector.cpp,v 1.46 2008/10/30 18:01:03 marcocle Exp $

// Include files
#include "GaudiKernel/xtoa.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/Tokenizer.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IDataStreamTool.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/EventSelectorDataStream.h"
#include "GaudiKernel/AppReturnCode.h"

#include "EventSelector.h"
#include "EventIterator.h"
#include <climits>

DECLARE_SERVICE_FACTORY(EventSelector)

// Standard constructor
EventSelector::EventSelector(const std::string& name, ISvcLocator* svcloc )
  : base_class( name, svcloc)
{
  m_incidentSvc       = 0;
  m_toolSvc           = 0;
  m_streamCount       = 0;
  m_firstEvent        = 0;
  m_evtPrintFrequency = 10;
  m_evtMax            = INT_MAX;
  declareProperty( "Input",      m_streamSpecs);
  declareProperty( "FirstEvent", m_firstEvent);
  declareProperty( "EvtMax",     m_evtMax);
  declareProperty( "PrintFreq",  m_evtPrintFrequency);
  declareProperty( "StreamManager",  m_streamManager="DataStreamTool");
  m_reconfigure = false;
}

// Standard destructor
EventSelector::~EventSelector()
{
}

StatusCode
EventSelector::resetCriteria(const std::string& /* criteria */,
                             Context&           /* context  */)  const
{
  return StatusCode::FAILURE;
}

// Progress report
void EventSelector::printEvtInfo(const EvtSelectorContext* iter) const {
  if ( 0 != iter )  {
    long count = iter->numEvent();
    // Print an message every m_evtPrintFrequency events
    if ( 0 == iter->context() )   {
      MsgStream log(msgSvc(), name());
      log << MSG::INFO << "End of event input reached." << endmsg;
    }
    else if( iter->numStreamEvent() == -1 ) {
      // Intial value for this stream
    }
    else if( m_evtPrintFrequency != -1 && (count % m_evtPrintFrequency == 0))   {
      MsgStream log(msgSvc(), name());
      log << MSG::ALWAYS << "Reading Event record " << count+1
          << ". Record number within stream " << iter->ID()+1
          << ": " << iter->numStreamEvent()+1 << endmsg;
    }
  }
  else  {
    MsgStream log(msgSvc(), name());
    log << MSG::INFO << "End of event input reached." << endmsg;
  }
}

// IEvtSelector::first()
StatusCode
EventSelector::firstOfNextStream(bool shutDown, EvtSelectorContext& iter) const {
  StatusCode status = StatusCode::SUCCESS;
  IDataStreamTool::size_type iter_id = (m_reconfigure) ? 0 : iter.ID()+1;
  if ( m_reconfigure ) const_cast<EventSelector*>(this)->m_reconfigure = false;
  if ( shutDown )   {
    if ( iter.ID() >= 0 && iter.ID() < (long)m_streamtool->size() )   {
      const EventSelectorDataStream* s = m_streamtool->getStream(iter.ID());
      if ( s->isInitialized() )    {
        EventSelector* thisPtr = const_cast<EventSelector*>(this);
        if ( s->selector() && iter.context() )  {
          Context* ctxt = iter.context();
          s->selector()->releaseContext(ctxt).ignore();
          iter.set(0,0);
        }
        status = thisPtr->m_streamtool->finalizeStream(const_cast<EventSelectorDataStream*>(s));
        iter.set(0,0);
      }
    }
  }

  const EventSelectorDataStream* s ;
  status = m_streamtool->getNextStream( s , iter_id );

  if ( status.isSuccess() )   {

    if ( !s->isInitialized() )    {
      EventSelector* thisPtr = const_cast<EventSelector*>(this);
      status = thisPtr->m_streamtool->initializeStream(const_cast<EventSelectorDataStream*>(s));
    }

    if ( status.isSuccess() ) {
      const IEvtSelector* sel = s->selector();
      if ( sel )    {
        Context* ctxt = 0;
        status = sel->createContext(ctxt);
        if ( status.isSuccess() )   {
          status = sel->resetCriteria(s->criteria(), *ctxt);
          if ( status.isSuccess() )   {
            MsgStream log(msgSvc(), name());
            iter.set(this, iter_id, ctxt, 0);
            log << MSG::INFO << *s << endmsg;
            m_incidentSvc->fireIncident(Incident(s->dbName(),IncidentType::BeginInputFile));
            return StatusCode::SUCCESS;
          }
        }
      }
    }
    if(s!=NULL) {
      m_incidentSvc->fireIncident(Incident(s->dbName(),IncidentType::FailInputFile));
    }
  }

  iter.set(this, -1, 0, 0);
  status.setChecked();
  //m_incidentSvc->fireIncident(Incident(s->dbName(),IncidentType::FailInputFile));
  return StatusCode::FAILURE;
}

// IEvtSelector::first()
StatusCode
EventSelector::lastOfPreviousStream(bool shutDown, EvtSelectorContext& iter) const {
  StatusCode status = StatusCode::SUCCESS;
  if ( shutDown )   {
    if ( iter.ID() >= 0 && iter.ID() < (long)m_streamtool->size() )   {
      const EventSelectorDataStream* s = m_streamtool->getStream(iter.ID());
      if ( s->isInitialized() )    {
        EventSelector* thisPtr = const_cast<EventSelector*>(this);
        if ( s->selector() && iter.context() )  {
          Context* ctxt = iter.context();
          s->selector()->releaseContext(ctxt);
          iter.set(0,0);
        }
        status = thisPtr->m_streamtool->finalizeStream(const_cast<EventSelectorDataStream*>(s));
        iter.set(0,0);
      }
    }
  }

  IDataStreamTool::size_type iter_id = iter.ID()-1;
  const EventSelectorDataStream* s ;
  status = m_streamtool->getPreviousStream( s , iter_id );

  if ( status.isSuccess() )   {

    if ( !s->isInitialized() )    {
      EventSelector* thisPtr = const_cast<EventSelector*>(this);
      status = thisPtr->m_streamtool->initializeStream(const_cast<EventSelectorDataStream*>(s));
    }
    if ( status.isSuccess() )   {
      const IEvtSelector* sel = s->selector();
      if ( sel )  {
        Context* ctxt = 0;
        status = sel->createContext(ctxt);
        if ( status.isSuccess() )   {
          status = sel->resetCriteria(s->criteria(), *ctxt);
          if ( status.isSuccess() )   {
            MsgStream log(msgSvc(), name());
            iter.set(this, iter_id, ctxt, 0);
            log << MSG::INFO << *s << endmsg;
            return StatusCode::SUCCESS;
          }
        }
      }
    }
  }

  iter.set(this, -1, 0, 0);
  return StatusCode::FAILURE;
}

/// Create a new event loop context
StatusCode EventSelector::createContext(Context*& refpCtxt) const
{
  // Max event is zero. Return begin = end
  refpCtxt = 0;
  if ( m_firstEvent < 0 ) {
    MsgStream log(msgSvc(), name());
    log << MSG::ERROR  << "First Event = " << m_firstEvent << " not valid" << endmsg;
    log << MSG::ERROR  << "It should be > 0 " << endmsg;
    return StatusCode::FAILURE;    // if failure => iterators = end();
  }
  EvtSelectorContext* ctxt = new EvtSelectorContext(this);
  ctxt->set(0, -1, 0, 0);
  firstOfNextStream(true, *ctxt).ignore();
  refpCtxt = ctxt;
  long nskip = m_firstEvent;
  while( --nskip > 0 )    {
    StatusCode sc = next(*refpCtxt);
    if ( sc.isFailure() ) {
      MsgStream log(msgSvc(), name());
      log << MSG::ERROR << " createContext() failed to start with event number "
          << m_firstEvent << endmsg;
      releaseContext(refpCtxt);
      refpCtxt = 0;
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

/// Get next iteration item from the event loop context
StatusCode EventSelector::next(Context& refCtxt) const  {
  return next(refCtxt, 1);
}

/// Get next iteration item from the event loop context, but skip jump elements
StatusCode EventSelector::next(Context& refCtxt, int /* jump */ ) const  {
  EvtSelectorContext *pIt  = dynamic_cast<EvtSelectorContext*>(&refCtxt);
  if ( pIt )    {
    if ( pIt->ID() != -1 ) {
      const EventSelectorDataStream* s = m_streamtool->getStream(pIt->ID());
      Context* it = pIt->context();
      IEvtSelector* sel = s->selector();
      if ( it && sel )    { // First exploit the current stream
        StatusCode sc = sel->next(*it);  // This stream is empty: advance to the next stream
        if ( !sc.isSuccess() )   {
          m_incidentSvc->fireIncident(Incident(s->dbName(),IncidentType::EndInputFile));
          sc = firstOfNextStream(true, *pIt);
          if (sc.isSuccess() ) sc = next(*pIt);
        }
        else  {
          pIt->increaseCounters(false);
          pIt->set(it, 0);
          printEvtInfo(pIt);
        }
        return sc;
      }
      else if ( m_reconfigure )  {
        StatusCode sc = firstOfNextStream(false, *pIt);
        printEvtInfo(pIt);
        return sc;
      }
    }
    else if ( m_reconfigure )  {
      StatusCode sc = firstOfNextStream(false, *pIt);
      printEvtInfo(pIt);
      return sc;
    }
    pIt->increaseCounters(false);
  }
  printEvtInfo(pIt);
  return StatusCode::FAILURE;
}

/// Get previous iteration item from the event loop context
StatusCode EventSelector::previous(Context& refCtxt) const  {
  return previous(refCtxt, 1);
}

/// Get previous iteration item from the event loop context, but skip jump elements
StatusCode EventSelector::previous(Context& refCtxt, int jump) const  {
  EvtSelectorContext *pIt  = dynamic_cast<EvtSelectorContext*>(&refCtxt);
  if ( pIt && jump > 0 )    {
    StatusCode sc = StatusCode::SUCCESS;
    for ( int i = 0; i < jump && sc.isSuccess(); ++i ) {
      const EventSelectorDataStream* s = m_streamtool->getStream(pIt->ID());
      Context* it = pIt->context();
      IEvtSelector* sel = s->selector();
      if ( it && sel )    { // First exploit the current stream
                            // This stream is empty: advance to the next stream
        sc = sel->previous(*it);  // This stream is empty: advance to the next stream
        if ( !sc.isSuccess() )   {
          sc = lastOfPreviousStream(true, *pIt);
        }
        else  {
          pIt->increaseCounters(false);
          pIt->set(it, 0);
        }
        printEvtInfo(pIt);
        if ( !sc.isSuccess() ) {
          return sc;
        }
      }
      pIt->increaseCounters(false);
    }
    return sc;
  }
  printEvtInfo(pIt);
  return StatusCode::FAILURE;
}

/// Access last item in the iteration
StatusCode EventSelector::last(Context& refCtxt) const {
  EvtSelectorContext *pIt  = dynamic_cast<EvtSelectorContext*>(&refCtxt);
  if ( pIt )    {
  }
  return StatusCode::FAILURE;
}

/// Rewind the dataset
StatusCode EventSelector::rewind(Context& refCtxt) const  {
  EvtSelectorContext *ctxt  = dynamic_cast<EvtSelectorContext*>(&refCtxt);
  if ( ctxt )    {
    ctxt->set(0, -1, 0, 0);
    firstOfNextStream(true, *ctxt);
    long nskip = m_firstEvent;
    while( --nskip > 0 )    {
      StatusCode sc = next(*ctxt);
      if ( sc.isFailure() ) {
        MsgStream log(msgSvc(), name());
        log << MSG::ERROR << "rewind() failed to start with event number "
            << m_firstEvent << endmsg;
        return StatusCode::FAILURE;
      }
    }
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

/// Create new Opaque address corresponding to the current record
StatusCode
EventSelector::createAddress(const Context&   refCtxt,
                             IOpaqueAddress*& refpAddr) const
{
  const EvtSelectorContext *cpIt  = dynamic_cast<const EvtSelectorContext*>(&refCtxt);
  EvtSelectorContext *pIt  = const_cast<EvtSelectorContext*>(cpIt);
  refpAddr = 0;
  if ( pIt )    {
    const EventSelectorDataStream* s = m_streamtool->getStream(pIt->ID());
    Context* it = pIt->context();
    IEvtSelector* sel = s->selector();
    if ( it && sel )    {
      IOpaqueAddress* pAddr = 0;
      StatusCode sc = sel->createAddress(*it, pAddr);
      if ( sc.isSuccess() )  {
        refpAddr = pAddr;
      }
      pIt->set(it, pAddr);
      return sc;
    }
  }
  return StatusCode::FAILURE;
}

// Release existing event iteration context
StatusCode EventSelector::releaseContext(Context*& refCtxt) const  {
  const EvtSelectorContext *cpIt = dynamic_cast<const EvtSelectorContext*>(refCtxt);
  EvtSelectorContext       *pIt  = const_cast<EvtSelectorContext*>(cpIt);
  if ( pIt && pIt->ID() >= 0 && pIt->ID() < (long)m_streamtool->size() ) {
    const EventSelectorDataStream* s = m_streamtool->getStream(pIt->ID());
    Context* it = pIt->context();
    IEvtSelector* sel = s->selector();
    if ( it && sel )    {
      StatusCode sc = sel->releaseContext(it);
      if ( sc.isSuccess() )  {
        refCtxt = 0;
        delete pIt;
        return sc;
      }
    }
  }
  if ( pIt )   {
    delete pIt;
  }
  return StatusCode::SUCCESS;
}

/// IService implementation: Db event selector override
StatusCode EventSelector::initialize()    {
  // Initialize base class
  StatusCode status = Service::initialize();
  MsgStream logger(msgSvc(), name());
  if ( !status.isSuccess() )    {
    logger << MSG::ERROR << "Error initializing base class Service!" << endmsg;
    return status;
  }
  // Get the references to the services that are needed by the ApplicationMgr itself
  m_incidentSvc = serviceLocator()->service("IncidentSvc");
  if( !m_incidentSvc.isValid() )  {
    logger << MSG::FATAL << "Error retrieving IncidentSvc." << endmsg;
    return StatusCode::FAILURE;
  }
  if ( m_evtMax != INT_MAX )   {
    logger << MSG::ERROR << "EvtMax is an obsolete property of the event selector." << endmsg;
    logger << MSG::ERROR << "Please set \"ApplicationMgr.EvtMax = " << m_evtMax
           << ";\" to process the requested number of events." << endmsg;
    return StatusCode::FAILURE;
  }

  m_toolSvc = serviceLocator()->service("ToolSvc");
  if ( !m_toolSvc.isValid() ) {
    logger << MSG::ERROR << " Could not locate the Tool Service! " << endmsg;
    return StatusCode::FAILURE;
  }

  status = m_toolSvc->retrieveTool(m_streamManager.c_str(), m_streamtool, this);

  if( status.isFailure() ) {
    logger << MSG::ERROR << "Error initializing "
           << m_streamManager << endmsg;
    return status;
  }

  status = m_streamtool->clear();
  if( status.isFailure() ) {
    // Message already printed by the tool
    return status;
  }

  status = m_streamtool->addStreams(m_streamSpecs);

  m_streamSpecsLast = m_streamSpecs;

  m_streamID          = 0;

  return status;
}

// Re-initialize
StatusCode EventSelector::reinitialize() {
  if ( FSMState() != Gaudi::StateMachine::INITIALIZED ) {
    MsgStream logger(msgSvc(), name());
    logger << MSG::ERROR << "Cannot reinitialize: service not in state initialized" << endmsg;
    return StatusCode::FAILURE;
  }

  if( m_streamSpecsLast != m_streamSpecs ) {
    StatusCode status = m_streamtool->clear();
    if ( status.isFailure() ) return status;
    m_streamSpecsLast = m_streamSpecs;
    m_reconfigure = true;
    return m_streamtool->addStreams(m_streamSpecs);
  }

  return StatusCode::SUCCESS;
}

//
StatusCode EventSelector::finalize()    {

  if (msgLevel(MSG::DEBUG)) {
    MsgStream log(msgSvc(), name());
    log << MSG::DEBUG << "finalize()" << endmsg;
  }

  m_incidentSvc = 0;

  if (m_streamtool) {
    if (m_toolSvc.isValid()) {
      m_toolSvc->releaseTool(m_streamtool).ignore();
    } else {
      // It should not be possible to get here
      m_streamtool->release();
    }
    m_streamtool = 0;
  }

  m_toolSvc = 0;

  return Service::finalize();
}

