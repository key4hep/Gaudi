#ifndef GAUDIEXAMPLES_INCIDENTASYNCTESTALG_H_
#define GAUDIEXAMPLES_INCIDENTASYNCTESTALG_H_

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/DataObjectHandle.h"
#include <memory>

class IIncidentSvc;
class IncidentListener;
class IIncidentAsyncTestSvc;

class IncidentAsyncTestAlg : public Algorithm
{
public:
  using Algorithm::Algorithm;

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

  bool isClonable() const override { return true; }

  static std::string& incident();

private:
  Gaudi::Property<std::string> m_serviceName{this, "ServiceName", "IncTestSvc"};
  Gaudi::Property<std::vector<std::string>> m_inpKeys{this, "inpKeys"};
  Gaudi::Property<std::vector<std::string>> m_outKeys{this, "outKeys"};
  SmartIF<IIncidentAsyncTestSvc> m_service;
  std::vector<std::unique_ptr<DataObjectHandle<DataObject>>> m_inputObjHandles;
  std::vector<std::unique_ptr<DataObjectHandle<DataObject>>> m_outputObjHandles;
};

#endif /*GAUDIEXAMPLES_INCIDENTREGISTRYTESTALG_H_*/
