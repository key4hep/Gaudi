#ifndef GAUDIEXAMPLES_PARTPROPEXA_H
#define GAUDIEXAMPLES_PARTPROPEXA_H 1

#include "GaudiKernel/Algorithm.h"

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


#endif // GAUDIEXAMPLES_PARTPROPEXA_H
