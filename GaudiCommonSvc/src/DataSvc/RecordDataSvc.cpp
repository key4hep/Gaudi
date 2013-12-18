//====================================================================
//	RecordDataSvc.cpp
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: implementation of the Transient event data service.
//
//  Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 10/12/08| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define  DATASVC_RECORDDATASVC_CPP

#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/DataIncident.h"
#include "GaudiKernel/RegistryEntry.h"

#include "RecordDataSvc.h"
using namespace std;

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_COMPONENT(RecordDataSvc)

/// Service initialisation
StatusCode RecordDataSvc::initialize()    {
  // Nothing to do: just call base class initialisation
  StatusCode      sc  = DataSvc::initialize();
  MsgStream log(msgSvc(),name());

  if ( !sc.isSuccess() ) { // Base class failure
    return sc;
  }
  // Attach data loader facility
  sc = service(m_persSvcName, m_cnvSvc, true);
  if ( !sc.isSuccess() ) {
    log << MSG::ERROR << "Failed to access RecordPersistencySvc." << endmsg;
    return sc;
  }
  SmartIF<IProperty> prp(m_cnvSvc);
  if ( prp ) {
    //prp->setProperty();
  }
  sc = setDataLoader( m_cnvSvc );
  if ( !sc.isSuccess() ) {
    log << MSG::ERROR << "Failed to attach dataloader RecordPersistencySvc." << endmsg;
    return sc;
  }

  sc = setRoot(m_rootName, new DataObject());
  if( !sc.isSuccess() )  {
    log << MSG::WARNING << "Error declaring Record root DataObject" << endmsg;
    return sc;
  }

  if( !m_incidentSvc )  {
    log << MSG::FATAL << "IncidentSvc is invalid--base class failed." << endmsg;
    return sc;
  }

  /// We need to subscribe to 2 incident:
  /// 1) FILE_OPEN_READ:            fired by conversion service on open file
  /// 2) IncidentType::BeginEvent   fired by event loop BEFORE the event processing starts.
  ///                               Do everything to bootstract access to the old event record.
  m_incidentSvc->addListener(this,"FILE_OPEN_READ");
  m_incidentSvc->addListener(this,m_saveIncidentName);
  return sc;
}

/// Service reinitialisation
StatusCode RecordDataSvc::reinitialize()    {
  // Do nothing for this service
  return StatusCode::SUCCESS;
}

/// Service finalization
StatusCode RecordDataSvc::finalize()    {
  if( m_incidentSvc ) m_incidentSvc->removeListener(this);
  if( m_cnvSvc ) m_cnvSvc->release();
  m_cnvSvc = 0;
  DataSvc::finalize().ignore();
  return StatusCode::SUCCESS ;
}

/// Inform that a new incident has occured
void RecordDataSvc::handle(const Incident& incident) {
  if ( incident.type() == "FILE_OPEN_READ" ) {
    typedef ContextIncident<IOpaqueAddress*> Ctxt;
    const Ctxt* inc = dynamic_cast<const Ctxt*>(&incident);
    if ( inc ) {
      registerRecord(inc->source(),inc->tag());
      if ( !m_incidentName.empty() ) {
	StringV incidents(m_incidents);
	m_incidents.clear();
	for( StringV::const_iterator i=incidents.begin(); i!=incidents.end();++i)
	  m_incidentSvc->fireIncident(Incident(*i,m_incidentName));
      }
      return;
    }
    MsgStream log(msgSvc(),name());
    log << MSG::ALWAYS << "Received invalid incident of type:" << incident.type() << endmsg;
  }
  else if ( incident.type() == m_saveIncidentName ) {
    MsgStream log(msgSvc(),name());
    log << MSG::ALWAYS << "Saving records not implemented." << endmsg;
  }
}

/// Load dependent records into memory
void RecordDataSvc::loadRecords(IRegistry* pObj) {
  if ( 0 != pObj )    {
    typedef vector<IRegistry*> Leaves;
    Leaves leaves;
    DataObject* p = 0;
    MsgStream log(msgSvc(),name());
    const string& id0 = pObj->identifier();
    StatusCode sc = retrieveObject(id0, p);
    if ( sc.isSuccess() ) {
      log << MSG::DEBUG << "Loaded records object: " << id0 << endmsg;
      sc = objectLeaves(pObj, leaves);
      if ( sc.isSuccess() )  {
	for ( Leaves::const_iterator i=leaves.begin(); i != leaves.end(); i++ )
	  loadRecords(*i);
      }
    }
    else  {
      log << MSG::ERROR << "Failed to load records object: " << pObj->identifier() << endmsg;
    }
  }
}

/// Load new run record into the data store if necessary
void RecordDataSvc::registerRecord(const string& data, IOpaqueAddress* pAddr)   {
  if ( !data.empty() && 0 != pAddr ) {
    MsgStream log(msgSvc(),name());
    string fid = data;
    log << MSG::DEBUG << "Request to load record for file " << fid << endmsg;
    StatusCode sc = registerAddress(m_root,fid,pAddr);
    if ( !sc.isSuccess() ) {
      log << MSG::WARNING << "Failed to register record for:" << fid << endmsg;
      pAddr->release();
      return;
    }
    if ( m_autoLoad ) {
      loadRecords(pAddr->registry());
    }
    m_incidents.push_back(pAddr->registry()->identifier());
  }
  else if ( !data.empty() && 0 == pAddr ) {
    MsgStream log(msgSvc(),name());
    log << MSG::INFO << "Failed to register record for:" << data << " [Invalid Address]" << endmsg;
  }
}

/// Standard Constructor
RecordDataSvc::RecordDataSvc(const string& name,ISvcLocator* svc)
: base_class(name,svc), m_cnvSvc(0)
{
  m_rootName = "/Records";
  declareProperty("AutoLoad",       m_autoLoad = true);
  declareProperty("IncidentName",   m_incidentName = "");
  declareProperty("SaveIncident",   m_saveIncidentName = "SAVE_RECORD");
  declareProperty("PersistencySvc", m_persSvcName = "PersistencySvc/RecordPersistencySvc");
}

/// Standard Destructor
RecordDataSvc::~RecordDataSvc()  {
}
