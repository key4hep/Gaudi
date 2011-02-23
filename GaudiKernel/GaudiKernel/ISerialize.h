// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/ISerialize.h,v 1.3 2001/01/19 14:22:55 mato Exp $
#ifndef GAUDIKERNEL_ISERIALIZE_H
#define GAUDIKERNEL_ISERIALIZE_H

// Framework include files
#include "GaudiKernel/IInterface.h"

// Forward declarations
class StreamBuffer;

/** @class ISerialize ISerialize.h GaudiKernel/ISerialize.h

    Object serialization interface definition.
    Definition of a interface to save and restore objects.

    @author M.Frank
*/
class GAUDI_API ISerialize: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(ISerialize,2,0);

  /** Input serialization from stream buffer. Restores the status of the object.
      @param   str    Reference to Streambuffer to be filled
      @return  Reference to filled stream buffer
  */
  virtual StreamBuffer& serialize(StreamBuffer& str) = 0;
  /** Output serialization to stream buffer. Saves the status of the object.
      @param   str    Reference to Streambuffer to be read
      @return  Reference to read stream buffer
  */
  virtual StreamBuffer& serialize(StreamBuffer& str) const = 0;
};

#endif // GAUDIKERNEL_ISERIALIZE_H
