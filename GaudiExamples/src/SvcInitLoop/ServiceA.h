#ifndef SERVICEA_H
#define SERVICEA_H 1

#include "GaudiKernel/Service.h"

class ServiceA: public Service {
public:
  ServiceA(const std::string& nam, ISvcLocator* svcLoc);
  virtual StatusCode initialize();
};


#endif
