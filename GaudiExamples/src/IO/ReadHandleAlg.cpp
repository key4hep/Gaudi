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

ReadHandleAlg::ReadHandleAlg(const std::string& nam, ISvcLocator* pSvc)
: GaudiAlgorithm(nam, pSvc), 
  m_inputHandle("/Event/MyCollision",Gaudi::DataHandle::Reader, this)
{
  declareProperty ( "Input", m_inputHandle);
}



//--------------------------------------------------------------------
// Execute
//--------------------------------------------------------------------
StatusCode ReadHandleAlg::execute() {
  
  MsgStream log(msgSvc(), name());
  
  Collision* c = m_inputHandle.get();

  const int evtNum = getContext()? getContext()->evt() : -1;

  log << MSG::INFO << "Event " << evtNum << " Collision number " <<  c->collision() << endmsg;
  
  return StatusCode::SUCCESS;
}
