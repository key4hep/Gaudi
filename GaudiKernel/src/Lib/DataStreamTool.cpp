// Include files

// from Gaudi
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/EventSelectorDataStream.h"
#include "GaudiKernel/DataStreamTool.h"
#include "GaudiKernel/Service.h"

//-----------------------------------------------------------------------------
// Implementation file for class : DataStreamTool
//
// 2006-09-21 : Andres Felipe Osorio Oliveros
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
DataStreamTool::DataStreamTool( const std::string& type,
                                const std::string& name,
                                const IInterface* parent )
  : base_class ( type, name , parent )
{
  //declareInterface<IDataStreamTool>(this);
}
//=============================================================================
StatusCode DataStreamTool::initialize() {

  MsgStream logger(msgSvc(), name());

  StatusCode status = AlgTool::initialize();
  if( !status.isSuccess() )  {
    logger << MSG::FATAL << "Error. Cannot initialize base class." << endmsg;
    return status;
  }

  // Get the references to the services that are needed by the ApplicationMgr itself
  m_incidentSvc = serviceLocator()->service("IncidentSvc");
  if( !m_incidentSvc )  {
    logger << MSG::FATAL << "Error retrieving IncidentSvc." << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;

}

StatusCode DataStreamTool::addStream(const std::string & input) {

  if ( getStream(input) )   {
    MsgStream log(msgSvc(), name());
    log << MSG::WARNING << "Input stream " << input << "already in use" << endmsg;
  }

  m_streamSpecs.push_back(input);

  auto strname = name() + '_' + std::to_string( ++m_streamCount );
  EventSelectorDataStream* s = nullptr;

  StatusCode status = createStream(strname, input , s );

  if( status.isSuccess() && s ) {
    s->addRef();
    m_streams.push_back(s);
    status = StatusCode::SUCCESS;
  } else {
    MsgStream log(msgSvc(), name());
    if (s) {
      s->release();
      log << MSG::ERROR << "Error connecting/creating Stream: " << s << endmsg;
    }
    log << MSG::ERROR << "Error connecting/creating Stream: " << input << endmsg;
    status = StatusCode::FAILURE;
  }
  return status;
}

StatusCode DataStreamTool::addStreams(const StreamSpecs & inputs) {

  StatusCode status = StatusCode::SUCCESS;
  for ( auto& i : inputs ) {
      status = addStream(i);
      if  (!status.isSuccess()) break;
  }
  return status;

}

StatusCode DataStreamTool::finalize() {
  clear().ignore();
  m_incidentSvc.reset();
  return AlgTool::finalize();
}

StatusCode DataStreamTool::initializeStream(EventSelectorDataStream* s)   {
  IEvtSelector* sel = nullptr;
  StatusCode status = s->initialize();
  if ( status.isSuccess() )   {
    status = createSelector(s->name(), s->selectorType(), sel);
    if ( status.isSuccess() )   {
      SmartIF<IProperty> prop(sel); //Att: IProperty, IService used to point to EventSelector
      SmartIF<IService>  isvc(sel);
      s->setSelector(sel);
      sel->release();  // No need for this interface anymore, it is passed to the stream
      if ( prop && isvc )   {
        for( const auto& i : s->properties() ) prop->setProperty(i).ignore();
        int output_level = this->outputLevel();
        prop->setProperty(IntegerProperty("OutputLevel",output_level)).ignore();
        // FIXME: (MCl) Why do we have to initialize the selector here?
        return isvc->sysInitialize();
      }
    }
  }
  return StatusCode::FAILURE;
}

// Create (sub-) Event selector service
StatusCode DataStreamTool::createSelector(const std::string& nam, const std::string& typ, IEvtSelector*& sel) {
  IService* isvc = Service::Factory::create(typ, nam, serviceLocator());
  if ( isvc ) {
    StatusCode status = isvc->queryInterface(IEvtSelector::interfaceID(), (void**)&sel);
    if ( status.isSuccess() ) return status;
    sel = nullptr;
    isvc->release();
  }
  MsgStream log(msgSvc(), name());
  log << MSG::ERROR << "Failed to create IEvtSelector " << typ << "/" << nam << endmsg;
  return StatusCode::FAILURE;
}


StatusCode DataStreamTool::finalizeStream(EventSelectorDataStream* s)   {
  if ( s )    {
    IEvtSelector* sel = const_cast<IEvtSelector*>(s->selector());
    if ( sel )    {
      SmartIF<IService> isvc(sel);
      if ( isvc )   {
        isvc->finalize().ignore();
        s->finalize().ignore();
        // Fire EndStream "Incident"
        m_incidentSvc->fireIncident(Incident(name(),IncidentType::EndStream));
        return StatusCode::SUCCESS;
      }
      // Failed to get service interface of sub-event selector
      return StatusCode::FAILURE;
    }
    // No selector (yet) attached - no need to finalize it!
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}


StatusCode DataStreamTool::eraseStream ( const std::string& info )   {

  auto i = getStreamIterator(info);
  if ( i !=  m_streams.end() )   {
    (*i)->release();
    m_streams.erase(i);
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

StatusCode DataStreamTool::createStream(const std::string& nam, const std::string& info,
                                        EventSelectorDataStream*& stream)  {
  stream = new EventSelectorDataStream(nam, info, serviceLocator());
  return StatusCode::SUCCESS;
}


EventSelectorDataStream * DataStreamTool::getStream( const std::string& info ) {
  auto i = getStreamIterator(info);
  return   i != m_streams.end() ? *i : nullptr;
}

DataStreamTool::Streams::iterator DataStreamTool::getStreamIterator ( const std::string& info ) {
  return std::find_if( std::begin(m_streams), std::end(m_streams),
                       [&](const EventSelectorDataStream* i) {
                           return i->definition()==info;
  });
}

EventSelectorDataStream * DataStreamTool::getStream( size_type pos ) {
  // pos has to point inside the vector
  return  ( (pos >= 0) && ((size_t)pos < m_streams.size()) ) ? m_streams[pos]
                                                             : nullptr;
}

EventSelectorDataStream * DataStreamTool::lastStream()
{
  return m_streams.back();
}



StatusCode DataStreamTool::clear()
{

  StatusCode iret, status = StatusCode::SUCCESS;
  iret.ignore();

  MsgStream log(msgSvc(), name());

  // disconnect the streams
  for ( auto& il : m_streamSpecs ) {
    EventSelectorDataStream* s = getStream(il);
    if ( s )   {
      if ( s->isInitialized() )    {
        iret = finalizeStream(s);
        if ( !iret.isSuccess() )  {
          log << MSG::ERROR << "Error finalizing Stream" << il << endmsg;
          status = iret;
        }
      }
      iret = eraseStream( il );
      if ( !iret.isSuccess() )    {
        log << MSG::ERROR << "Error diconnecting Stream" << il << endmsg;
        status = iret;
      }
    }
  }

  m_streamSpecs.clear();

  return status;
}


StatusCode DataStreamTool::connectStream( EventSelectorDataStream *s)
{

  if ( !s ) return StatusCode::FAILURE;
  s->addRef();
  m_streams.push_back(s);
  return StatusCode::SUCCESS;

}

StatusCode DataStreamTool::connectStream( const std::string & info )
{
  if ( getStream(info) )   {
    MsgStream log(msgSvc(), name());
    log << MSG::WARNING << "Input stream " << info << "already in use" << endmsg;
  }
  auto nam  = name() +  '_' + std::to_string( ++m_streamCount);
  EventSelectorDataStream* s = nullptr;
  StatusCode status = createStream(nam, info, s);
  if ( status.isSuccess() ) return connectStream(s);
  s->release();
  return status;
}

/*

  Taking control over Streams and return them to EventSelector

*/


StatusCode DataStreamTool::getNextStream( const EventSelectorDataStream * & esds, size_type & dsid )
{

  EventSelectorDataStream * nextStream = getStream(dsid);
  if ( !nextStream ) return StatusCode::FAILURE; //<-end of streams reached

  esds = nextStream;
  ++m_streamID;

  return StatusCode::SUCCESS;

}

StatusCode DataStreamTool::getPreviousStream( const EventSelectorDataStream * & esds, size_type & dsid )
{

  EventSelectorDataStream * previousStream = getStream(dsid);
  if (  !previousStream ) return StatusCode::FAILURE; //<-begin of streams reached

  esds = previousStream;
  --m_streamID;

  return StatusCode::SUCCESS;

}
