// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IClassInfo.h,v 1.3 2006/11/30 15:22:07 dquarrie Exp $
#ifndef GAUDIKERNEL_ICLASSINFO_H
#define GAUDIKERNEL_ICLASSINFO_H

// Experiment specific include files
#include "GaudiKernel/Kernel.h"

/** @class IClassInfo IClassInfo.h GaudiKernel/IClassInfo.h

    Data base class allowing to store persistent type information
*/
class GAUDI_API IClassInfo {
public:
  /// Standard Destructor
  virtual ~IClassInfo() { }
  /// Retrieve Pointer to class definition structure
  virtual const CLID& clID() const = 0;
};

#endif // GAUDIKERNEL_ICLASSINFO_H
