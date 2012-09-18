#ifndef GAUDIHIVE_HIVEREGISTRYENTRY_H
#define GAUDIHIVE_HIVEREGISTRYENTRY_H

// STL include files
#include <vector>
#include <string>

// Framework include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/RegistryEntry.h"
#include "GaudiKernel/DataSvc.h"

// tbb include files
#include "tbb/concurrent_vector.h"

// Forward declarations
//class DataSvc;
class DataObject;
class IDataProviderSvc;
class IOpaqueAddress;
class IDataStoreAgent;


namespace Hive   {

  class HiveEventRegistryEntry;

  class GAUDI_API HiveRegistryEntry : public DataSvcHelpers::RegistryEntry  {
  public:
    friend class ::DataSvc;
  private:
    /// Pointer to event the data belong to
    HiveEventRegistryEntry* m_eventRegistryEntry;

  private:
    /** The following entries serve two aspects:
      1) They are faster for recursive calls, because they are non-virtual
      2) They can be re-used for the non-const entry points using a
         const_cast of the result.
    */
    /// Recursive helper to assemble the full path name of the entry
    void  assemblePath(std::string& buffer)  const;
    /// internals of the adding to the data store
    HiveRegistryEntry* i_add(const std::string&);
  public:
    /// Standard Constructor
    HiveRegistryEntry(const std::string& path, RegistryEntry* parent = 0);
    /// Standard Destructor
    virtual ~HiveRegistryEntry();
    /// Add entry to data store
    virtual long add(const std::string& name,
                     DataObject* pObject,
                     bool is_soft = false);
    /// Add entry to data store
    virtual long add(const std::string& name,
                     IOpaqueAddress* pAddress,
                     bool is_soft = false);
    /// Add object to the container
    virtual long add(IRegistry* obj);
    ///
    virtual void setEventRegistry(HiveEventRegistryEntry* obj){m_eventRegistryEntry = obj;};
  };
}
#endif  // GAUDIHIVE_HIVEREGISTRYENTRY_H
