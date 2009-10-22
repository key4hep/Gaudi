#ifndef GAUDIKERNEL_IREGISTRY_H
#define GAUDIKERNEL_IREGISTRY_H

// Include files
#include "GaudiKernel/Kernel.h"
#include <string>

// forward declarations
class DataObject;
class IOpaqueAddress;
class IDataProviderSvc;

/** @class IRegistry IRegistry.h GaudiKernel/IRegistry.h

    The IRegistry represents the entry door to the environment
    any data object residing in a transient data store is embedded.

    Split from directory browsing (which is now sub-classed), providing
    a light-weight interface to differing registry mechanism.

*/
class GAUDI_API IRegistry  {
public:
  /// Type definitions
  /// Name type
  typedef std::string name_type;
  /// Identifier Key type
  typedef std::string id_type;

  /// destructor
  virtual ~IRegistry() { }

  /// Add reference to object
  virtual       unsigned long   addRef    ()         = 0;

  /// release reference to object
  virtual       unsigned long   release   ()         = 0;

  /// Name of the directory (or key)
  virtual const name_type&      name      ()   const = 0;

  /// Full identifier (or key)
  virtual const id_type&        identifier()   const = 0;

  /// Retrieve pointer to Transient Store
  virtual IDataProviderSvc*     dataSvc   ()   const = 0;

  /// Retrieve object behind the link
  virtual DataObject*           object    ()   const = 0;

  /// Retrieve opaque storage address
  virtual IOpaqueAddress*       address   ()   const = 0;

  /// Set/Update Opaque storage address
  virtual void setAddress (IOpaqueAddress* pAddress) = 0;
};
#endif // KERNEL_IREGISTRY_H
