#ifndef GAUDIKERNEL_IISSUELOGGER_H
#define GAUDIKERNEL_IISSUELOGGER_H

#ifndef GAUDIKERNEL_ISERVICE_H
 #include "GaudiKernel/IService.h"
#endif
#include "GaudiKernel/IssueSeverity.h"
#include <string>


class GAUDI_API IIssueLogger: virtual public IService {

public:
  /// InterfaceID
  DeclareInterfaceID(IIssueLogger,2,0);

  virtual void report(IssueSeverity::Level level, const std::string& msg,
		      const std::string& origin) = 0;
  virtual void report(const IssueSeverity& err) = 0;

};

#endif
