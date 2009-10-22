#ifndef SERVICEB_H
#define SERVICEB_H 1

#include "GaudiKernel/Service.h"

class ServiceB: public Service {
public:
  ServiceB(const std::string& nam, ISvcLocator* svcLoc);
  virtual StatusCode initialize();
};

#endif
