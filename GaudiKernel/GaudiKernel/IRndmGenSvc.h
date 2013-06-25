#ifndef GAUDIKERNEL_IRNDMGENSVC_H
#define GAUDIKERNEL_IRNDMGENSVC_H

// STL include files
#include <string>
#include <vector>

// Framework include files
#include "GaudiKernel/IService.h"
#include "GaudiKernel/IRndmGen.h"

// Forward declarations
class IFactory;
class IRndmGen;
class StreamBuffer;
class IRndmEngine;
class IRndmGenFactory;

/** @class IRndmGenSvc IRndmGenSvc.h GaudiKernel/IRndmGenSvc.h

    Random Generator service interface definition
    Definition of a interface for a service to access
    random generators according to predefined distributions:

    The Random Generator creates proper instances of requested
    random generators and delivers them to the user.

    On request the service dumps the type and state of the random
    number engine to a StreamBuffer object allowing to save and to restore
    the exact state of the random number generator.

    @author  M.Frank
    @version 1.0
*/
class GAUDI_API IRndmGenSvc: virtual public IService {
public:
  /// InterfaceID
  DeclareInterfaceID(IRndmGenSvc,2,0);
  /** Retrieve engine.
      @return pointer to random number engine
  */
  virtual IRndmEngine* engine() = 0;

  /** Add a Generator factory.
      @param fac Pointer to factory to be added
      @return StatusCode indicating success or failure.
  */
  virtual StatusCode generator(const IRndmGen::Param& par, IRndmGen*& refpGen) = 0;

  /// virtual destructor
  virtual ~IRndmGenSvc() {}
};

#endif // GAUDIKERNEL_IRNDMGENSVC_H
