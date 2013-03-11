// Framework include files
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IDataProviderSvc.h"

// Example related include files
#include "ReadHandleAlg.h"

#include "GaudiKernel/System.h"

using namespace Gaudi::Examples;

DECLARE_ALGORITHM_FACTORY(ReadHandleAlg)

//--------------------------------------------------------------------
// Initialize
//--------------------------------------------------------------------
StatusCode ReadHandleAlg::initialize() {
  MsgStream log(msgSvc(), name());
  StatusCode sc = declareDataObj(m_inputName,m_inputHandle);
  return sc;
}

//--------------------------------------------------------------------
// Finalize
//--------------------------------------------------------------------
StatusCode ReadHandleAlg::finalize() {

  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------
// Execute
//--------------------------------------------------------------------
StatusCode ReadHandleAlg::execute() {
  
  MsgStream log(msgSvc(), name());
  
  Collision* c = m_inputHandle->get();

  const int evtNum = getContext()? getContext()->m_evt_num : -1;

  log << MSG::INFO << "Event " << evtNum << " Collision number " <<  c->collision() << endmsg;
  
  return StatusCode::SUCCESS;
}
