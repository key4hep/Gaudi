// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IClassManager.h,v 1.6 2006/12/11 13:40:13 leggett Exp $
#ifndef GAUDIKERNEL_ICLASSMANAGER_H
#define GAUDIKERNEL_ICLASSMANAGER_H

// Include files
#include "GaudiKernel/IInterface.h"
#include <string>

// Forward class declaration
class IFactory;
class ISvcLocator;

/** @class IClassManager IClassManager.h GaudiKernel/IClassManager.h

    The IClassManager is the interface implemented by the generic Factory in the
    Application Manager to support class management functions.

    @author Pere Mato
*/
class GAUDI_API IClassManager: virtual public IInterface  {
public:
  DeclareInterfaceID(IClassManager,2,0);

  /// Declare a sharable library to be used for creating instances of a given algorithm type.
  virtual StatusCode loadModule( const std::string& module,
				 bool fireIncident=true ) = 0;  // Sharable library name
};

#endif  // GAUDIKERNEL_ICLASSMANAGER_H
