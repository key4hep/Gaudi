#ifndef DETDESC_IREADOUT_H
#define DETDESC_IREADOUT_H 1

// Base class
#include "DetDesc/IConditionInfo.h"

///---------------------------------------------------------------------------
/** @class IReadOut IReadOut.h Det/DetDesc/IReadOut.h

    An abstract interface to get the readout information of
    a detector element.

    @author Andrea Valassi
    @date December 2001
*///--------------------------------------------------------------------------

class GAUDI_API IReadOut: virtual public IConditionInfo {
public:
  /// InterfaceID
  DeclareInterfaceID(IReadOut, 1, 0);
};

#endif // DETDESC_IREADOUT_H




