// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IRunable.h,v 1.1 2001/03/14 15:30:13 mato Exp $
#ifndef GAUDIKERNEL_IRUNNABLE_H
#define GAUDIKERNEL_IRUNNABLE_H

#include "GaudiKernel/IInterface.h"

/** @class IRunable IRunable.h GaudiKernel/IRunable.h

    Runable interface definition. Package GaudiKernel

    Abstract interface of an object to be "run".
    Best analogy: java.thread.Thread.run().
    A known user is the application manager, which
    gives control to a runable for execution of the event loop.

   @author Markus Frank
   @version 2.0
*/
class GAUDI_API IRunable: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IRunable,2,0);

  /// Run the class implementation
  virtual StatusCode run() = 0;
};

#endif // GAUDIKERNEL_IRUNNABLE_H
