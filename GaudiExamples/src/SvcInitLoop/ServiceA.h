#ifndef SERVICEA_H
#define SERVICEA_H 1

#include "GaudiKernel/Service.h"

struct ServiceA : Service {
  using Service::Service;
  StatusCode initialize() override;
};

#endif
