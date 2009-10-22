#ifndef ICOMPONENTMANAGER_H_
#define ICOMPONENTMANAGER_H_

// Include files
#include "GaudiKernel/IStateful.h"

/** @class IComponentManager IComponentManager.h GaudiKernel/IComponentManager.h

    @author Marco Clemencic
*/
class GAUDI_API IComponentManager: virtual public IStateful {
public:
  /// InterfaceID
  DeclareInterfaceID(IComponentManager,1,0);

  /// Basic interface id of the managed components.
  virtual const InterfaceID& componentBaseInterface() const = 0;

  /// Virtual destructor
  virtual ~IComponentManager();
};

#endif /* ICOMPONENTMANAGER_H_ */
