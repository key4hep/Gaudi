#ifndef GAUDIKERNEL_ICLASSINFO_H
#define GAUDIKERNEL_ICLASSINFO_H

// Experiment specific include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/ClassID.h"

/** @class IClassInfo IClassInfo.h GaudiKernel/IClassInfo.h

    Data base class allowing to store persistent type information
*/
class GAUDI_API IClassInfo {
public:
  /// Standard Destructor
  virtual ~IClassInfo() = default;
  /// Retrieve Pointer to class definition structure
  virtual const CLID& clID() const = 0;
};

#endif // GAUDIKERNEL_ICLASSINFO_H
