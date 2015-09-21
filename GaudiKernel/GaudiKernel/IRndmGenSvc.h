#ifndef GAUDIKERNEL_IRNDMGENSVC_H
#define GAUDIKERNEL_IRNDMGENSVC_H

// STL include files
#include <string>
#include <vector>

// Framework include files
#include "GaudiKernel/IService.h"
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/SmartIF.h"

// Forward declarations
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
  DeclareInterfaceID(IRndmGenSvc,2,1);
  /** Retrieve engine.
      @return pointer to random number engine
  */
  virtual IRndmEngine* engine() = 0;

  /** Add a Generator factory.
      @param fac Pointer to factory to be added
      @return StatusCode indicating success or failure.
  */
  virtual StatusCode generator(const IRndmGen::Param& par, IRndmGen*& refpGen) = 0;

  SmartIF<IRndmGen> generator(const IRndmGen::Param& par) {
    IRndmGen *gen = nullptr;;
    generator(par, gen).ignore();
    auto r = SmartIF<IRndmGen>{ gen };
    if (gen) { gen->release(); }
    return r;
  }

  /// virtual destructor
  virtual ~IRndmGenSvc() = default;
};

#endif // GAUDIKERNEL_IRNDMGENSVC_H
