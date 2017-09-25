#ifndef INTERFACES_ISTAGERSVC_H
#define INTERFACES_ISTAGERSVC_H

// Include Files
#include "GaudiKernel/IInterface.h"

// Forward declarations
class IIncidentListener;
class Incident;

class GAUDI_API IStagerSvc : virtual public IInterface
{
public:
  /// InterfaceID
  DeclareInterfaceID( IStagerSvc, 2, 0 );

  /// Retrieve interface ID
  virtual StatusCode getStreams() = 0;
  virtual StatusCode readAhead()  = 0;
  virtual StatusCode initStage()  = 0;
};

#endif // GAUDIKERNEL_STAGERSVC_H
