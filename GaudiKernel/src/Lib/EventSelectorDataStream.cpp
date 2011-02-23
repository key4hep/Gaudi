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
#include "GaudiKernel/Tokenizer.h"
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
  Tokenizer tok(true);

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

  tok.analyse(m_definition, " ", "", "", "=", "'", "'");
  for ( Tokenizer::Items::iterator i = tok.items().begin(); i != tok.items().end(); i++ )   {
    long hash = -1;
    const std::string& tag = (*i).tag();
    const std::string& val = (*i).value();
    switch( ::toupper(tag[0]) )    {
    case 'A':
      auth = val;
      break;
    case 'C':
      svc  = "EvtTupleSvc";
      isData = false;
    case 'E':
      hash = val.find('#');
      if ( hash > 0 )   {
        cnt  = val.substr(0,hash);
        item = val.substr(hash+1, val.length()-hash-1);
      }
      else    {
        cnt  = val;
        item = "Address";
      }
      break;
    case 'D':
      m_criteria     = "FILE " + val;
      m_dbName=val;
      break;
    case 'F':
      switch( ::toupper(tag[1]) )    {
      case 'I':
        m_criteria   = "FILE " + val;
	m_dbName=val;
        break;
      case 'U':
        stmt = val;
        break;
      default:
        break;
      }
      break;
    case 'J':
      m_criteria     = "JOBID " + val;
      m_dbName=val;
      dbtyp          = "SICB";
      break;
    case 'T':
      switch( ::toupper(tag[1]) )    {
      case 'Y':
        dbtyp = val;
        break;
      default:
        break;
      }
      break;
    case 'S':
      switch( ::toupper(tag[1]) )    {
      case 'E':
        sel = val;
        break;
      case 'V':
        svc = val;
	collsvc = val;
        break;
      default:
        break;
      }
      break;
    default:
      m_properties->push_back(StringProperty(tag,val));
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
