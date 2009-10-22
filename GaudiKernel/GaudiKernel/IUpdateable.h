// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IUpdateable.h,v 1.2 2006/11/30 15:22:07 dquarrie Exp $
#ifndef GAUDIKERNEL_IUPDATEABLE_H
#define GAUDIKERNEL_IUPDATEABLE_H

#include "GaudiKernel/StatusCode.h"

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

  /// destructor
  virtual ~IUpdateable() { }

  /** Output serialisation to stream buffer. Saves the status of the object.
    * @param   flag    Flag to trigger implementation specific action.
    *
    * @return  Status code indicating success or failure.
    */
  virtual StatusCode update(int flag) = 0;
};

#endif // GAUDIKERNEL_IUPDATEABLE_H
