#ifndef GAUDIKERNEL_IAUDITORSVC_H
#define GAUDIKERNEL_IAUDITORSVC_H

#include "GaudiKernel/IAuditor.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/System.h"
#include <string>

/** @class IAuditorSvc IAuditorSvc.h GaudiKernel/IAuditorSvc.h

    The interface implemented by the IAuditorSvc base class.

    @author Pere Mato
*/
class GAUDI_API IAuditorSvc : virtual public extend_interfaces<IService, IAuditor> {
public:
  /// InterfaceID
  DeclareInterfaceID( IAuditorSvc, 3, 0 );

  /// management functionality: retrieve an Auditor
  virtual IAuditor* getAuditor( const std::string& name ) = 0;
};

#endif // INTERFACES_IAUDITORSVC_H
