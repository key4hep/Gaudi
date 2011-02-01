//$Id: IDetDataSvc.h,v 1.4 2008/10/27 19:22:20 marcocle Exp $
#ifndef GAUDIKERNEL_IDETDATASVC_H
#define GAUDIKERNEL_IDETDATASVC_H 1

// Base class
#include "GaudiKernel/IInterface.h"

// Forward declarations
namespace Gaudi { class Time; }

///---------------------------------------------------------------------------
/** @class IDetDataSvc IDetDataSvc.h GaudiKernel/IDetDataSvc.h

    Abstract interface for a DataSvc manipulating condition data
    (i.e. DataObjects implementing IValidity).

    @author Andrea Valassi
    @date August 2001
*///--------------------------------------------------------------------------

class GAUDI_API IDetDataSvc: virtual public IInterface
{
public:
  /// InterfaceID
  DeclareInterfaceID(IDetDataSvc,2,0);

  /// Check if the event time has been set
  virtual bool validEventTime() const = 0;

  /// Get the event time
  virtual const Gaudi::Time& eventTime() const = 0;

  /// Set the event time
  virtual void setEventTime( const Gaudi::Time& ) = 0;
};

#endif // GAUDIKERNEL_IDETDATASVC_H
