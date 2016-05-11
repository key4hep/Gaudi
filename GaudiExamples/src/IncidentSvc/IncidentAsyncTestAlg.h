#ifndef GAUDIEXAMPLES_INCIDENTASYNCTESTALG_H_
#define GAUDIEXAMPLES_INCIDENTASYNCTESTALG_H_

#include "GaudiKernel/Algorithm.h"
#include <memory>

class IIncidentSvc;
class IncidentListener;
class IIncidentAsyncTestSvc;

class IncidentAsyncTestAlg: public Algorithm
{
public:
  IncidentAsyncTestAlg(const std::string& name ,
		       ISvcLocator*       pSvcLocator );
  ~IncidentAsyncTestAlg() override;
  
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;
  
  static std::string &incident();
  
private:
  std::string m_serviceName;
  SmartIF<IIncidentAsyncTestSvc> m_service;
  std::vector<std::string> m_inpKeys, m_outKeys;
  std::vector<DataObjectHandle<DataObject> *> m_inputObjHandles;
  std::vector<DataObjectHandle<DataObject> *> m_outputObjHandles;
  
};

#endif /*GAUDIEXAMPLES_INCIDENTREGISTRYTESTALG_H_*/
