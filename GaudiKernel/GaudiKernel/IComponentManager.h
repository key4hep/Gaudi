#ifndef ICOMPONENTMANAGER_H_
#define ICOMPONENTMANAGER_H_

// Include files
#include "GaudiKernel/IStateful.h"

/** @class IComponentManager IComponentManager.h GaudiKernel/IComponentManager.h

    @author Marco Clemencic
*/
class GAUDI_API IComponentManager : virtual public IStateful
{
public:
  /// InterfaceID
  DeclareInterfaceID( IComponentManager, 1, 0 );

  /// Basic interface id of the managed components.
  virtual const InterfaceID& componentBaseInterface() const = 0;

  /// Function to call to update the outputLevel of the components (after a change in MessageSvc).
  virtual void outputLevelUpdate() = 0;
};

#endif /* ICOMPONENTMANAGER_H_ */
