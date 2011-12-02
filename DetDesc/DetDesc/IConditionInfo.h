//$Id: IConditionInfo.h,v 1.4 2009-02-02 12:20:22 marcocle Exp $
#ifndef DETDESC_ICONDITIONINFO_H
#define DETDESC_ICONDITIONINFO_H 1

#include "GaudiKernel/IInterface.h"
// Forward declarations
class IDetectorElement;
class Condition;

///---------------------------------------------------------------------------
/** @class IConditionInfo IConditionInfo.h Det/DetDesc/IConditionInfo.h

    An abstract interface to get the condition information of
    a detector element.

    @author Andrea Valassi
    @date December 2001
*///--------------------------------------------------------------------------

class GAUDI_API IConditionInfo: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IConditionInfo, 2, 0);

  /// Get a pointer to the detector element to which the ConditionInfo belongs
  virtual IDetectorElement* detElem() const = 0 ;

  /// Get the name of the associated condition
  virtual const std::string& conditionName() const = 0 ;

  /// Get a pointer to the associated condition
  virtual Condition* condition() = 0 ;

};

#endif // DETDESC_ICONDITIONINFO_H

