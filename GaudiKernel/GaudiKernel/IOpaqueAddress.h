// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IOpaqueAddress.h,v 1.6 2006/11/30 15:22:07 dquarrie Exp $
#ifndef GAUDIKERNEL_IOPAQUEADDRESS_H
#define GAUDIKERNEL_IOPAQUEADDRESS_H

// STL include files
#include <string>

// Experiment specific include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/ClassID.h"

// forward declarations
class IRegistry;

/** @class IOpaqueAddress IOpaqueAddress.h GaudiKernel/IOpaqueAddress.h

    Opaque address interface definition.
    The opaque address defines the behaviour necesary to load
    and store transient objects.

    @author M.Frank
    @version 1.0
*/
class GAUDI_API IOpaqueAddress {
public:

  /// destructor
  virtual ~IOpaqueAddress() { }

  /// Add reference to object
  virtual unsigned long        addRef     () = 0;
  /// release reference to object
  virtual unsigned long        release    () = 0;
  /// Retrieve class information from link
  virtual const CLID&          clID       () const = 0;
  /// Retrieve service type
  virtual long                 svcType    () const = 0;
  /// Update branch name
  virtual IRegistry*           registry   () const = 0;
  /// Update directory pointer
  virtual void                 setRegistry(IRegistry* r) = 0;
  /// Retrieve String parameters
  virtual const std::string*   par        () const = 0;
  /// Access to generic link parameters
  virtual const unsigned long* ipar       () const = 0;
};
#endif // GAUDIKERNEL_IOPAQUEADDRESS_H
