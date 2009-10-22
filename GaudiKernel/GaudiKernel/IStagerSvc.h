// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IStagerSvc.h,v 1.1 2005/07/05 10:55:09 hmd Exp $
#ifndef INTERFACES_ISTAGERSVC_H
#define INTERFACES_ISTAGERSVC_H

// Include Files
#include "GaudiKernel/IInterface.h"
#include <string>

// Forward declarations
class IIncidentListener;
class Incident;

class GAUDI_API IStagerSvc: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IStagerSvc,2,0);

  /// Retrieve interface ID
  virtual StatusCode getStreams()=0;
  virtual StatusCode readAhead()=0;
  virtual StatusCode initStage()=0;

};

#endif // GAUDIKERNEL_STAGERSVC_H
