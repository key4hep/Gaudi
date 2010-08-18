#ifndef GAUDIEXAMPLES_PARTPROPEXA_H
#define GAUDIEXAMPLES_PARTPROPEXA_H 1

#include "GaudiKernel/Algorithm.h"
#include "HepPDT/ProcessUnknownID.hh"
#include "HepPDT/CommonParticleData.hh"

class IPartPropSvc;

class PartPropExa : public Algorithm {

public:
  PartPropExa (const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
  
private:

  IPartPropSvc *m_pps;

};

namespace HepPDT {
class TestUnknownID : public ProcessUnknownID {
public:
  TestUnknownID() {}

  virtual 
  CommonParticleData *processUnknownID( ParticleID, 
					const ParticleDataTable & pdt );

};

}

#endif // GAUDIEXAMPLES_PARTPROPEXA_H
