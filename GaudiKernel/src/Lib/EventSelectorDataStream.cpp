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
#include "GaudiKernel/EventSelectorDataStream.h"
#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IPersistencySvc.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"

// Output friend
MsgStream& operator<<( MsgStream& s, const EventSelectorDataStream& obj ) {
  return s << "Stream:" << obj.name() << " Def:" << obj.definition();
}

// Output friend
std::ostream& operator<<( std::ostream& s, const EventSelectorDataStream& obj ) {
  return s << "Stream:" << obj.name() << " Def:" << obj.definition();
}

// Standard Constructor
EventSelectorDataStream::EventSelectorDataStream( std::string nam, std::string def, ISvcLocator* svcloc )
    : m_name{std::move( nam )}, m_definition{std::move( def )}, m_pSvcLocator( svcloc ) {}

// Set selector
void EventSelectorDataStream::setSelector( IEvtSelector* pSelector ) { m_pSelector = pSelector; }

// Allow access to individual properties by name
Gaudi::Property<std::string>* EventSelectorDataStream::property( const std::string& nam ) {
  auto i = std::find_if( std::begin( m_properties ), std::end( m_properties ),
                         [&]( const Gaudi::Property<std::string>& j ) { return j.name() == nam; } );
  return i != std::end( m_properties ) ? &( *i ) : nullptr;
}

// Allow access to individual properties by name
const Gaudi::Property<std::string>* EventSelectorDataStream::property( const std::string& nam ) const {
  auto i = std::find_if( std::begin( m_properties ), std::end( m_properties ),
                         [&]( const Gaudi::Property<std::string>& j ) { return j.name() == nam; } );
  return i != std::end( m_properties ) ? &( *i ) : nullptr;
}

// Parse input criteria
StatusCode EventSelectorDataStream::initialize() {
  bool        isData = true;
  std::string auth, dbtyp, collsvc, item, crit, sel, svc, stmt;
  std::string cnt = "/Event";
  std::string db  = "<Unknown>";

  auto eds = m_pSvcLocator->service<IDataManagerSvc>( "EventDataSvc" );
  if ( !eds ) {
    std::cout << "ERROR: Unable to localize interface IDataManagerSvc from service EventDataSvc" << std::endl;
    return StatusCode::FAILURE;
  } else {
    cnt = eds->rootName();
  }
  m_selectorType = m_criteria = m_dbName = "";
  m_properties.clear();

  using Parser = Gaudi::Utils::AttribStringParser;
  for ( auto attrib : Parser( m_definition ) ) {
    long hash = -1;
    switch ( ::toupper( attrib.tag[0] ) ) {
    case 'A':
      auth = std::move( attrib.value );
      break;
    case 'C':
      svc    = "EvtTupleSvc";
      isData = false;
    /* FALLTHROUGH */
    case 'E':
      hash = attrib.value.find( '#' );
      if ( hash > 0 ) {
        cnt  = attrib.value.substr( 0, hash );
        item = attrib.value.substr( hash + 1 );
      } else {
        cnt  = std::move( attrib.value );
        item = "Address";
      }
      break;
    case 'D':
      m_criteria = "FILE " + attrib.value;
      m_dbName   = std::move( attrib.value );
      break;
    case 'F':
      switch ( ::toupper( attrib.tag[1] ) ) {
      case 'I':
        m_criteria = "FILE " + attrib.value;
        m_dbName   = std::move( attrib.value );
        break;
      case 'U':
        stmt = std::move( attrib.value );
        break;
      default:
        break;
      }
      break;
    case 'J':
      m_criteria = "JOBID " + attrib.value;
      m_dbName   = std::move( attrib.value );
      dbtyp      = "SICB";
      break;
    case 'T':
      switch ( ::toupper( attrib.tag[1] ) ) {
      case 'Y':
        dbtyp = std::move( attrib.value );
        break;
      default:
        break;
      }
      break;
    case 'S':
      switch ( ::toupper( attrib.tag[1] ) ) {
      case 'E':
        sel = std::move( attrib.value );
        break;
      case 'V':
        svc     = std::move( attrib.value );
        collsvc = svc;
        break;
      default:
        break;
      }
      break;
    default:
      m_properties.emplace_back( attrib.tag, attrib.value );
      break;
    }
  }
  if ( !isData ) { // Unfortunately options do not come in order...
    m_selectorType = "EventCollectionSelector";
    svc            = "EvtTupleSvc";
  } else if ( dbtyp.compare( 0, 4, "POOL" ) == 0 ) {
    m_selectorType = "PoolDbEvtSelector";
  } else if ( svc.empty() ) {
    m_selectorType = "DbEvtSelector";
  } else {
    m_selectorType = svc;
  }
  StatusCode status = StatusCode::SUCCESS;
  if ( svc.empty() && !dbtyp.empty() ) {
    auto ipers = m_pSvcLocator->service<IPersistencySvc>( "EventPersistencySvc" );
    if ( ipers ) {
      IConversionSvc* icnvSvc = nullptr;
      status                  = ipers->getService( dbtyp, icnvSvc );
      if ( status.isSuccess() ) {
        auto isvc = SmartIF<INamedInterface>{icnvSvc};
        if ( isvc ) svc = isvc->name();
      }
    }
  }
  m_properties.emplace_back( "Function", stmt );
  m_properties.emplace_back( "CnvService", svc );
  m_properties.emplace_back( "Authentication", auth );
  m_properties.emplace_back( "Container", cnt );
  m_properties.emplace_back( "Item", item );
  m_properties.emplace_back( "Criteria", sel );
  m_properties.emplace_back( "DbType", dbtyp );
  m_properties.emplace_back( "DB", m_criteria );
  if ( !isData && !collsvc.empty() ) { m_properties.emplace_back( "DbService", collsvc ); }

  m_initialized = status.isSuccess();
  return status;
}

// Parse input criteria
StatusCode EventSelectorDataStream::finalize() {
  setSelector( nullptr );
  m_properties.clear();
  m_initialized = false;
  return StatusCode::SUCCESS;
}
