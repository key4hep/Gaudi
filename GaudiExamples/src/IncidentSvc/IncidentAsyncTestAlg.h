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
  using Algorithm::Algorithm;
  ~IncidentAsyncTestAlg() override;

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

  static std::string &incident();

private:
  StringProperty  m_serviceName {this, "ServiceName",  "IncTestSvc" };
  StringArrayProperty  m_inpKeys {this, "inpKeys"};
  StringArrayProperty  m_outKeys {this, "outKeys"};
  SmartIF<IIncidentAsyncTestSvc> m_service;
  std::vector<DataObjectHandle<DataObject> *> m_inputObjHandles;
  std::vector<DataObjectHandle<DataObject> *> m_outputObjHandles;
};

#endif /*GAUDIEXAMPLES_INCIDENTREGISTRYTESTALG_H_*/
