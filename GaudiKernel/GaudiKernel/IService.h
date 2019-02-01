#ifndef GAUDIKERNEL_ISERVICE_H
#define GAUDIKERNEL_ISERVICE_H

// Include files
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/IStateful.h"
#include <string>

/** @class IService IService.h GaudiKernel/IService.h

    General service interface definition

    @author Pere Mato
*/
class ISvcManager;
class ServiceManager;

class GAUDI_API IService : virtual public extend_interfaces<INamedInterface, IStateful> {
  friend class ServiceManager;

public:
  /// InterfaceID
  DeclareInterfaceID( IService, 4, 0 );

  /// Initialize Service
  virtual StatusCode sysInitialize() = 0;
  /// Start Service
  virtual StatusCode sysStart() = 0;
  /// Stop Service
  virtual StatusCode sysStop() = 0;
  /// Finalize Service
  virtual StatusCode sysFinalize() = 0;
  /// Re-initialize the Service
  virtual StatusCode sysReinitialize() = 0;
  /// Re-start the Service
  virtual StatusCode sysRestart() = 0;

protected:
  virtual void setServiceManager( ISvcManager* ) = 0;
};

#endif // GAUDIKERNEL_ISERVICE_H
