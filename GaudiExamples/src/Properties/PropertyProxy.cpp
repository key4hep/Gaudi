// $Id: PropertyProxy.cpp,v 1.4 2006/11/30 10:35:27 mato Exp $

// Include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/SmartIF.h"
#include "PropertyProxy.h"

// Static Factory declaration

DECLARE_ALGORITHM_FACTORY(PropertyProxy)

// Constructor
//------------------------------------------------------------------------------
PropertyProxy::PropertyProxy(const std::string& name, ISvcLocator* ploc)
           : Algorithm(name, ploc) {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name);
  // Declare remote properties at this moment
  SmartIF<IAlgManager> algMgr (serviceLocator());
  SmartIF<IProperty> rAlgP(algMgr->algorithm("PropertyAlg"));
  if( rAlgP.isValid() ) {
    m_remAlg = rAlgP; // remember it for later
    declareRemoteProperty("RInt", rAlgP, "Int");
    declareRemoteProperty("String", rAlgP );
  }
  else {
    log << MSG::WARNING << " The 'remote' Algorithm PropertyAlg is not found" << endmsg;
  }
}

//------------------------------------------------------------------------------
StatusCode PropertyProxy::initialize() {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name());

  std::string value("empty");
  std::string value1("empty");

  this->getProperty("RInt", value).ignore();
  log << MSG::INFO << " Got property this.RInt = " << value << ";" << endmsg;

  this->setProperty("RInt", "1001").ignore();
  log << MSG::INFO << " Set property this.RInt = " << "1001" << ";" << endmsg;

  this->getProperty("RInt", value).ignore();
  log << MSG::INFO << " Got property this.RInt = " << value << ";" << endmsg;


  this->getProperty("String", value).ignore();
  m_remAlg->getProperty("String", value1).ignore();
  if( value == value1 ) {
    log << MSG::INFO << " Got property this.String = " << value << ";" << endmsg;
  }
  else {
    log << MSG::ERROR << " Local property [" << value1 << "] not equal [" << value << "]" << endmsg;
  }

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode PropertyProxy::execute() {
//------------------------------------------------------------------------------
  MsgStream         log( msgSvc(), name() );
  log << MSG::INFO << "executing...." << endmsg;

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode PropertyProxy::finalize() {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "finalizing...." << endmsg;

  return StatusCode::SUCCESS;
}


