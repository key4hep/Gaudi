// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IFactory.h,v 1.4 2001/02/23 16:12:10 dquarrie Exp $
#ifndef GAUDIKERNEL_IFACTORY_H
#define GAUDIKERNEL_IFACTORY_H

// Include files
#include "GaudiKernel/IInterface.h"
#include <string>
#include <vector>

/** @class IFactory IFactory.h GaudiKernel/IFactory.h

    The IFactory is the interface used to create contrete instances of
    Objectes. It can be used to create Services, Algorithms, Converters, ...

    @author Pere Mato
*/
class GAUDI_API IFactory: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IFactory,2,0);

  /// Instantiate an instance of a Gaudi class
  virtual IInterface* instantiate( IInterface *parent ) const = 0;
  virtual unsigned long addRef()   const = 0;
  virtual unsigned long release()   const = 0;

  /// Re-declaring these here avoids a compiler warning about hidden
  /// functions. Refer to ARM Page 210.
  /// Increment the reference count of Interface instance
  virtual unsigned long addRef() = 0;
  /// Release Interface instance
  virtual unsigned long release() = 0;

  virtual const std::string& ident() const = 0;

};

#endif // GAUDIKERNEL_IFACTORY_H
