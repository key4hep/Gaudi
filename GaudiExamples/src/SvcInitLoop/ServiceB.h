#ifndef SERVICEB_H
#define SERVICEB_H 1

#include "GaudiKernel/Service.h"

struct ServiceB : Service {
  using Service::Service;
  StatusCode initialize() override;
};

#endif
