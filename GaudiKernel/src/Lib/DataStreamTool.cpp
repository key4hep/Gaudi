// $Id: DataStreamTool.cpp,v 1.5 2008/04/04 15:12:19 marcocle Exp $
// Include files

// from Gaudi
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/xtoa.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/Tokenizer.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/EventSelectorDataStream.h"
#include "GaudiKernel/DataStreamTool.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/SvcFactory.h"

#include <sstream>

//-----------------------------------------------------------------------------
// Implementation file for class : DataStreamTool
//
// 2006-09-21 : Andres Felipe Osorio Oliveros
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
// Now the declaration is done in GaudiSvc
//DECLARE_TOOL_FACTORY( DataStreamTool )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
DataStreamTool::DataStreamTool( const std::string& type,
                                const std::string& name,
                                const IInterface* parent )
  : base_class ( type, name , parent )
{
  //declareInterface<IDataStreamTool>(this);

  m_incidentSvc       = 0;
  m_streamCount       = 0;
  m_streamID          = 0;

}
//=============================================================================
// Destructor
//=============================================================================
DataStreamTool::~DataStreamTool() {
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
  if( !m_incidentSvc.isValid() )  {
    logger << MSG::FATAL << "Error retrieving IncidentSvc." << endmsg;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;

}

StatusCode DataStreamTool::addStream(const std::string & input) {

  if ( NULL != getStream(input) )   {
    MsgStream log(msgSvc(), name());
    log << MSG::WARNING << "Input stream " << input << "already in use" << endmsg;
  }

  m_streamSpecs.push_back(input);

  std::ostringstream strname;
  strname << name() << '_' << ++m_streamCount;

  EventSelectorDataStream* s = 0;

  StatusCode status = createStream(strname.str(), input , s );

  if( status.isSuccess() && 0 != s ) {
    s->addRef();
    m_streams.push_back(s);
    status = StatusCode::SUCCESS;
  }
  else {
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

  for ( StreamSpecs::const_iterator itr = inputs.begin(); itr != inputs.end() && status.isSuccess(); ++itr )    {

    status = addStream(*itr);

  }

  return status;

}

StatusCode DataStreamTool::finalize() {
  clear().ignore();
  m_incidentSvc = 0; // release

  return AlgTool::finalize();
}

StatusCode DataStreamTool::initializeStream(EventSelectorDataStream* s)   {
  IEvtSelector* sel = 0;
  StatusCode status = s->initialize();
  if ( status.isSuccess() )   {
    status = createSelector(s->name(), s->selectorType(), sel);
    if ( status.isSuccess() )   {
      SmartIF<IProperty> prop(sel); //Att: IProperty, IService used to point to EventSelector
      SmartIF<IService>  isvc(sel);
      s->setSelector(sel);
      sel->release();  // No need of this interface anymore, it is passed to the stream
      if ( prop.isValid( ) && isvc.isValid( ) )   {
        const Properties& p = s->properties();
        for(Properties::const_iterator i=p.begin(); i!=p.end(); i++)   {
          prop->setProperty((*i)).ignore();
        }
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
  IService* isvc = ROOT::Reflex::PluginService::Create<IService*>(typ, nam, serviceLocator());
  if ( isvc ) {
    StatusCode status = isvc->queryInterface(IEvtSelector::interfaceID(), (void**)&sel);
    if ( status.isSuccess() ) {
      return status;
    }
    sel = 0;
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
      if ( isvc.isValid() )   {
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

  Streams::iterator i = getStreamIterator(info);

  if ( m_streams.end() != i )   {
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
  Streams::iterator i = getStreamIterator(info);
  if ( m_streams.end() == i ) return NULL;
  return *i;
}

DataStreamTool::Streams::iterator DataStreamTool::getStreamIterator ( const std::string& info ) {
  for ( Streams::iterator i = m_streams.begin(); i != m_streams.end(); i++ )    {
    if ( (*i)->definition() == info )    {
      return i;
    }
  }
  return m_streams.end();
}

EventSelectorDataStream * DataStreamTool::getStream( size_type pos ) {
  if ( (pos >= 0) && ((size_t)pos < m_streams.size()) ) // pos has to point inside the vector
    return m_streams[pos];
  else
    return 0;
}

EventSelectorDataStream * DataStreamTool::lastStream()
{
  if (m_streams.size() > 1 )
    return *(--m_streams.end());
  else return *m_streams.begin();

}



StatusCode DataStreamTool::clear()
{

  StatusCode iret, status = StatusCode::SUCCESS;
  iret.ignore();

  MsgStream log(msgSvc(), name());

  // disconnect the streams
  for ( StreamSpecs::const_iterator il = m_streamSpecs.begin(); il != m_streamSpecs.end(); il++ ) {
    EventSelectorDataStream* s = getStream(*il);
    if ( NULL != s )   {
      if ( s->isInitialized() )    {
        iret = finalizeStream(s);
        if ( !iret.isSuccess() )  {
          log << MSG::ERROR << "Error finalizing Stream" << *il << endmsg;
          status = iret;
        }
      }
      iret = eraseStream( *il );
      if ( !iret.isSuccess() )    {
        log << MSG::ERROR << "Error diconnecting Stream" << *il << endmsg;
        status = iret;
      }
    }
  }

  m_streamSpecs.clear();

  return status;
}


StatusCode DataStreamTool::connectStream( EventSelectorDataStream *s)
{

  if ( 0 != s )   {
    s->addRef();
    m_streams.push_back(s);
    return StatusCode::SUCCESS;
  }

  return StatusCode::FAILURE;

}

StatusCode DataStreamTool::connectStream( const std::string & info )
{

  if ( NULL != getStream(info) )   {
    MsgStream log(msgSvc(), name());
    log << MSG::WARNING << "Input stream " << info << "already in use" << endmsg;
  }
  std::ostringstream nam;
  nam << name() << '_' << ++m_streamCount;
  EventSelectorDataStream* s = 0;
  StatusCode status = createStream(nam.str(), info, s);
  if ( status.isSuccess() )   {
    return connectStream(s);
  }
  s->release();
  return status;


}

/*

  Taking control over Streams and return them to EventSelector

*/


StatusCode DataStreamTool::getNextStream( const EventSelectorDataStream * & esds, size_type & dsid )
{

  EventSelectorDataStream * nextStream = getStream(dsid);
  if ( NULL == nextStream ) return StatusCode::FAILURE; //<-end of streams reached

  esds = nextStream;
  ++m_streamID;

  return StatusCode::SUCCESS;

}

StatusCode DataStreamTool::getPreviousStream( const EventSelectorDataStream * & esds, size_type & dsid )
{

  EventSelectorDataStream * previousStream = getStream(dsid);
  if (  NULL == previousStream ) return StatusCode::FAILURE; //<-begin of streams reached

  esds = previousStream;
  --m_streamID;

  return StatusCode::SUCCESS;

}



