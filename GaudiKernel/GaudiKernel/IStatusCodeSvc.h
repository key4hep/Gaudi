#ifndef GAUDIKERNEL_ISTATUSCODESVC_H
#define GAUDIKERNEL_ISTATUSCODESVC_H

#include "GaudiKernel/IService.h"

#include <string>

class GAUDI_API IStatusCodeSvc: virtual public IService {

public:
  /// InterfaceID
  DeclareInterfaceID(IStatusCodeSvc,2,0);

  virtual void regFnc(const std::string &fnc,
		      const std::string &lib) = 0;

  virtual void list() const = 0;

  virtual bool suppressCheck() const = 0;

};

#endif
