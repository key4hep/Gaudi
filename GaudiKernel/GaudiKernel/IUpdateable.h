// $Header:$
#ifndef GAUDIKERNEL_IUPDATEABLE_H
#define GAUDIKERNEL_IUPDATEABLE_H

#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/IInterface.h"

/** @class IUpdateable IUpdateable.h GaudiKernel/IUpdateable.h
  *
  *  Object update interface definition.
  *  Definition of a interface to recalculate object properties.
  *
  *   @author  M.Frank
  *   @version 1.0
  */
class GAUDI_API IUpdateable {
public:

  /// Default destructor
  virtual ~IUpdateable() { }

  /** Output serialization to stream buffer. Saves the status of the object.
    * @param   flag    Flag to trigger implementation specific action.
    *
    * @return  Status code indicating success or failure.
    */
  virtual StatusCode update(int flag) = 0;
};

/** @class IUpdateableInterface IUpdateable.h GaudiKernel/IUpdateable.h
  *
  *  Component interface counterpart to the above ABC.
  *
  *   @author  M.Frank
  *   @version 1.0
  */
class GAUDI_API IUpdateableIF : virtual public IInterface, virtual public IUpdateable {
public:
  /// InterfaceID
  DeclareInterfaceID(IUpdateableIF,1,0);

  /// Default destructor
  virtual ~IUpdateableIF() {}
};

#endif // GAUDIKERNEL_IUPDATEABLE_H
