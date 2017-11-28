#ifndef GAUDIEXAMPLES_INCIDENTLISTENERTESTALG_H_
#define GAUDIEXAMPLES_INCIDENTLISTENERTESTALG_H_

#include "GaudiAlg/GaudiAlgorithm.h"

#include <memory>

class IIncidentSvc;
class IncidentListenerTest;

class IncidentListenerTestAlg : public GaudiAlgorithm
{
public:
  using GaudiAlgorithm::GaudiAlgorithm;

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

  static std::string& incident();

private:
  static std::string s_incidentType;
  SmartIF<IIncidentSvc> m_incSvc;
  std::array<std::unique_ptr<IncidentListenerTest>, 6> m_listener;
};

#endif /*GAUDIEXAMPLES_INCIDENTLISTENERTESTALG_H_*/
