// $Id: FastContainersSvc.cpp,v 1.2 2006/11/24 09:56:53 hmd Exp $

// Include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/IIncidentSvc.h"

// local
#include "FastContainersSvc.h"

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_SERVICE_FACTORY(FastContainersSvc)

//-----------------------------------------------------------------------------
// Implementation file for class : FastContainersSvc
//
// 2006-05-03 : Marco Clemencic
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
FastContainersSvc::FastContainersSvc(const std::string& name,ISvcLocator* svc):
  base_class(name,svc)
{
  declareProperty("RootDirName", m_rootDirName = "fc" );
  m_rootName = "/" + m_rootDirName;
}

//=============================================================================
// Destructor
//=============================================================================
FastContainersSvc::~FastContainersSvc() {}

//=========================================================================
//  Initialization
//=========================================================================
StatusCode FastContainersSvc::initialize() {
  // initialize base class
  StatusCode status = DataSvc::initialize();
  if (!status.isSuccess()) return status;

  MsgStream log(msgSvc(), name());
  // create root node
  log << MSG::VERBOSE << "creating root node" << endmsg;
  status = setRoot(m_rootName,new DataObject);
  if (!status.isSuccess()){
    log << MSG::FATAL << "failed to create the root node" << endmsg;
    return status;
  }

  // register to the incident service
  log << MSG::VERBOSE << "registering to the incident service for " << IncidentType::BeginEvent << endmsg;
  m_incidentSvc->addListener(this,IncidentType::BeginEvent);

  return status;
}

//=========================================================================
// Empty reinitialization
//=========================================================================
StatusCode FastContainersSvc::reinitialize() {
  // do nothing
  return StatusCode::SUCCESS;
}

//=========================================================================
// Finalize the service
//=========================================================================
StatusCode FastContainersSvc::finalize() {
  MsgStream log(msgSvc(), name());
  // de-register to the incident service
  log << MSG::VERBOSE << "de-registering from the incident service" << endmsg;
  m_incidentSvc->removeListener(this,IncidentType::BeginEvent);

  return DataSvc::finalize();
}

//=========================================================================
// reset all the fast containers in the transient store
//=========================================================================
void FastContainersSvc::resetStore() {
  traverseTree(&m_storeResetter).ignore();
}

//=========================================================================
// handle BeginEvent incident
//=========================================================================
void FastContainersSvc::handle(const Incident& incident) {
  if (incident.type() == IncidentType::BeginEvent)
    resetStore();
}
//=============================================================================
