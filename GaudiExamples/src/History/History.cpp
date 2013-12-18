#include "History.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IHistorySvc.h"

#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/Property.h"

#include <sstream>

// Static Factory declaration
DECLARE_COMPONENT(History)

///////////////////////////////////////////////////////////////////////////

History::History( const std::string& name, ISvcLocator* pSvcLocator ) :
  Algorithm(name, pSvcLocator)
{

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode History::initialize() {

  MsgStream log(msgSvc(), name());

  if (service("HistorySvc",m_histSvc).isFailure()) {
    log << MSG::ERROR << "Could not get historySvc" << endmsg;
    return StatusCode::FAILURE;
  }

  log << MSG::INFO << "got historySvc: " << m_histSvc->name() << endmsg;

  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode History::execute() {

  MsgStream log(msgSvc(), name());

  //  StatusCode sc;


  return StatusCode::SUCCESS;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode History::finalize() {

  return StatusCode::SUCCESS;

}
