#include "IncidentAsyncTestAlg.h"

#include "IIncidentAsyncTestSvc.h"

#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/DataObjectHandle.h"
#include "GaudiKernel/DataObject.h"

// Static Factory declaration
DECLARE_COMPONENT(IncidentAsyncTestAlg)


//=============================================================================
IncidentAsyncTestAlg::IncidentAsyncTestAlg(const std::string& name ,
                                                 ISvcLocator*  pSvcLocator )
  :Algorithm(name,pSvcLocator)
{
  declareProperty("ServiceName", m_serviceName="IncTestSvc" );
  declareProperty("inpKeys", m_inpKeys);
  declareProperty("outKeys", m_outKeys);

}


//=============================================================================
StatusCode IncidentAsyncTestAlg::initialize() {
  StatusCode sc = Algorithm::initialize();
  if (sc.isFailure()) return sc;
  //get service containing event data
  m_service = service(m_serviceName,true);
  //Copied from CPUCruncher.cpp
  int i=0;
  for (auto k: m_inpKeys) {
    debug() << "adding input key " << k << endmsg;
    m_inputObjHandles.push_back( new DataObjectHandle<DataObject>( k, Gaudi::DataHandle::Reader, this ));
    declareInput(m_inputObjHandles.back());
    i++;
  }

  i = 0;
  for (auto k: m_outKeys) {
    debug() << "adding output key " << k << endmsg;
    m_outputObjHandles.push_back( new DataObjectHandle<DataObject>( k, Gaudi::DataHandle::Writer, this ));
    declareOutput(m_outputObjHandles.back() );
    i++;
  }
  
  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode IncidentAsyncTestAlg::execute() {
  uint64_t data=0;
  MsgStream logstream(msgSvc(), name());
  for (auto & inputHandle: m_inputObjHandles){
    if(!inputHandle->isValid())
      continue;
    
    DataObject* obj = nullptr;
    obj = inputHandle->get();
    if (obj == nullptr)
      logstream << MSG::ERROR << "A read object was a null pointer." << endmsg;
  }
  
  m_service->getData(&data);
  for (auto & outputHandle: m_outputObjHandles){
    if(!outputHandle->isValid())
      continue;
    outputHandle->put(new DataObject());
  }
  info() << "Read data "<<data << endmsg;
  return StatusCode::SUCCESS;
}

//=============================================================================
StatusCode IncidentAsyncTestAlg::finalize() {
  info() << "Finalizing " << endmsg;
  return Algorithm::finalize();
}

IncidentAsyncTestAlg::~IncidentAsyncTestAlg(){
  for (uint i = 0; i < m_inputObjHandles.size(); ++i) {
		delete m_inputObjHandles[i];
	}

  for (uint i = 0; i < m_outputObjHandles.size(); ++i) {
		delete m_outputObjHandles[i];
	}
}
