#ifndef GAUDIKERNEL_ICPUCRUNCHSVC_H
#define GAUDIKERNEL_ICPUCRUNCHSVC_H 1

#include "GaudiKernel/IService.h"
#include <chrono>

class GAUDI_API ICPUCrunchSvc : virtual public IService {

public:
  virtual ~ICPUCrunchSvc(){};

  DeclareInterfaceID( ICPUCrunchSvc, 1, 0 );

public:
  virtual std::chrono::milliseconds crunch_for( const std::chrono::milliseconds& ) const = 0;
};

#endif
