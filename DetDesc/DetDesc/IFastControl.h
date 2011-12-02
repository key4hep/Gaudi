//$Id: IFastControl.h,v 1.1 2001-12-13 19:20:29 andreav Exp $
#ifndef DETDESC_IFASTCONTROL_H
#define DETDESC_IFASTCONTROL_H 1

// Base class
#include "DetDesc/IConditionInfo.h"

///---------------------------------------------------------------------------
/** @class IFastControl IFastControl.h Det/DetDesc/IFastControl.h

    An abstract interface to get the slow control information of
    a detector element.

    @author Andrea Valassi
    @date December 2001
*///--------------------------------------------------------------------------

class GAUDI_API IFastControl: virtual public IConditionInfo {
public:
  /// InterfaceID
  DeclareInterfaceID(IFastControl, 1, 0);
};

#endif // DETDESC_IFASTCONTROL_H




