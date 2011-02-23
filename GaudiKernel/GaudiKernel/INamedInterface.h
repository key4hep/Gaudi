#ifndef GAUDIKERNEL_INAMEDINTERFACE_H
#define GAUDIKERNEL_INAMEDINTERFACE_H

// Include files
#include "GaudiKernel/IInterface.h"
#include <string>

/**
 * @class INamedInterface INamedInterface.h GaudiKernel/INamedInterface.h
 *
 * IInterface compliant class extending IInterface with the name() method.
 *
 * @author Charles Leggett
 */
class GAUDI_API INamedInterface: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(INamedInterface,1,0);

  /// Retrieve the name of the instance.
  virtual const std::string& name() const = 0;

  /// Virtual destructor (always needed for abstract classes).
  virtual ~INamedInterface() {}

};

#endif
