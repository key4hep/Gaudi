//====================================================================
//	EventSelectorDataStream.cpp
//--------------------------------------------------------------------
//
//	Package    : EventSelectorDataStream  (The LHCb Event Selector Package)
//
//
//	Author     : M.Frank
//      Created    : 4/10/00
//	Changes    : R. Lambert 2009-09-04
//
//====================================================================
#define GAUDISVC_EVENTSELECTOR_EVENTSELECTORDATASTREAM_CPP 1
// Include files
#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IPersistencySvc.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/EventSelectorDataStream.h"


// Output friend
MsgStream& operator<<(MsgStream& s, const EventSelectorDataStream& obj)    {
  s << "Stream:"   << obj.name() << " Def:" << obj.definition();
  return s;
}

// Output friend
std::ostream& operator<<(std::ostream& s, const EventSelectorDataStream& obj)    {
  s << "Stream:"   << obj.name() << " Def:" << obj.definition();
  return s;
}

// Standard Constructor
EventSelectorDataStream::EventSelectorDataStream(const std::string& nam, const std::string& def, ISvcLocator* svcloc)
: m_pSelector(0),
  m_pSvcLocator(svcloc)
{
  m_name = nam;
  m_definition = def;
  m_initialized = false;
  m_properties = new Properties();
}

// Standard Constructor
EventSelectorDataStream::~EventSelectorDataStream()   {
  setSelector(0);
  delete m_properties;
}

// Set selector
void EventSelectorDataStream::setSelector(IEvtSelector* pSelector)   {
  if ( 0 != pSelector   )  pSelector->addRef();
  if ( 0 != m_pSelector )  m_pSelector->release();
  m_pSelector = pSelector;
}

// Allow access to individual properties by name
StringProperty* EventSelectorDataStream::property(const std::string& nam)    {
  for ( Properties::iterator i = m_properties->begin(); i != m_properties->end(); i++ )   {
    if ( (*i).name() == nam )    {
      return &(*i);
    }
  }
  return 0;
}

// Allow access to individual properties by name
const StringProperty* EventSelectorDataStream::property(const std::string& nam)   const  {
  for ( Properties::const_iterator i = m_properties->begin(); i != m_properties->end(); i++ )   {
    if ( (*i).name() == nam )    {
      return &(*i);
    }
  }
  return 0;
}

// Parse input criteria
StatusCode EventSelectorDataStream::initialize()   {
  bool isData = true;
  std::string auth, dbtyp, collsvc, item, crit, sel, svc, stmt;
  std::string cnt    = "/Event";
  std::string db     = "<Unknown>";

  SmartIF<IDataManagerSvc> eds(m_pSvcLocator->service("EventDataSvc"));
  if( !eds.isValid() ) {
    std::cout << "ERROR: Unable to localize interface IDataManagerSvc from service EventDataSvc"
              << std::endl;
    return StatusCode::FAILURE;
  }
  else {
    cnt = eds->rootName();
  }
  m_selectorType = m_criteria = m_dbName= "";
  m_properties->erase(m_properties->begin(), m_properties->end());

  using Parser = Gaudi::Utils::AttribStringParser;
  for (auto attrib: Parser(m_definition)) {
    long hash = -1;
    switch( ::toupper(attrib.tag[0]) )    {
    case 'A':
      auth = std::move(attrib.value);
      break;
    case 'C':
      svc  = "EvtTupleSvc";
      isData = false;
      /* no break */
    case 'E':
      hash = attrib.value.find('#');
      if ( hash > 0 )   {
        cnt  = attrib.value.substr(0, hash);
        item = attrib.value.substr(hash + 1);
      }
      else    {
        cnt  = std::move(attrib.value);
        item = "Address";
      }
      break;
    case 'D':
      m_criteria     = "FILE " + attrib.value;
      m_dbName = std::move(attrib.value);
      break;
    case 'F':
      switch( ::toupper(attrib.tag[1]) )    {
      case 'I':
        m_criteria   = "FILE " + attrib.value;
	m_dbName = std::move(attrib.value);
        break;
      case 'U':
        stmt = std::move(attrib.value);
        break;
      default:
        break;
      }
      break;
    case 'J':
      m_criteria     = "JOBID " + attrib.value;
      m_dbName = std::move(attrib.value);
      dbtyp          = "SICB";
      break;
    case 'T':
      switch( ::toupper(attrib.tag[1]) )    {
      case 'Y':
        dbtyp = std::move(attrib.value);
        break;
      default:
        break;
      }
      break;
    case 'S':
      switch( ::toupper(attrib.tag[1]) )    {
      case 'E':
        sel = std::move(attrib.value);
        break;
      case 'V':
        svc = std::move(attrib.value);
	collsvc = svc;
        break;
      default:
        break;
      }
      break;
    default:
      m_properties->push_back(StringProperty(attrib.tag, attrib.value));
      break;
    }
  }
  if ( !isData )    { // Unfortunately options do not come in order...
    m_selectorType = "EventCollectionSelector";
    svc  = "EvtTupleSvc";
  }
  else if ( dbtyp.substr(0,4) == "POOL" )    {
    m_selectorType = "PoolDbEvtSelector";
  }
  else if ( svc.empty() ) {
    m_selectorType = "DbEvtSelector";
  }
  else  {
    m_selectorType = svc;
  }
  StatusCode status = StatusCode::SUCCESS;
  if ( svc.length() == 0 && dbtyp.length() != 0 )    {
    SmartIF<IPersistencySvc> ipers(m_pSvcLocator->service("EventPersistencySvc"));
    if ( ipers.isValid() )   {
      IConversionSvc* icnvSvc = 0;
      status = ipers->getService(dbtyp, icnvSvc);
      if ( status.isSuccess() )   {
        IService* isvc = 0;
        status = icnvSvc->queryInterface(IService::interfaceID(), pp_cast<void>(&isvc));
        if ( status.isSuccess() )   {
          svc = isvc->name();
          isvc->release();
        }
      }
    }
  }
  m_properties->push_back( StringProperty("Function",      stmt));
  m_properties->push_back( StringProperty("CnvService",    svc));
  m_properties->push_back( StringProperty("Authentication",auth));
  m_properties->push_back( StringProperty("Container",     cnt));
  m_properties->push_back( StringProperty("Item",          item));
  m_properties->push_back( StringProperty("Criteria",      sel));
  m_properties->push_back( StringProperty("DbType",        dbtyp));
  m_properties->push_back( StringProperty("DB",            m_criteria));
  if ( !isData && !collsvc.empty() )    {
    m_properties->push_back( StringProperty("DbService",   collsvc));
  }

  m_initialized = status.isSuccess();
  return status;
}

// Parse input criteria
StatusCode EventSelectorDataStream::finalize()   {
  setSelector(0);
  if ( m_properties )  {
    m_properties->clear();
  }
  m_initialized = false;
  return StatusCode::SUCCESS;
}
