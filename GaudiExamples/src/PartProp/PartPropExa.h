#ifndef GAUDIEXAMPLES_PARTPROPEXA_H
#define GAUDIEXAMPLES_PARTPROPEXA_H 1

#include "GaudiKernel/Algorithm.h"
#include "HepPDT/CommonParticleData.hh"
#include "HepPDT/ProcessUnknownID.hh"

class IPartPropSvc;

class PartPropExa : public Algorithm
{

public:
  PartPropExa( const std::string& name, ISvcLocator* pSvcLocator );
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  IPartPropSvc* m_pps;
};

namespace HepPDT
{
  class TestUnknownID : public ProcessUnknownID
  {
  public:
    TestUnknownID() {}

    virtual CommonParticleData* processUnknownID( ParticleID, const ParticleDataTable& pdt ) override;
  };
}

#endif // GAUDIEXAMPLES_PARTPROPEXA_H
