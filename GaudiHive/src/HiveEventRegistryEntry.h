#ifndef GAUDIHIVE_HIVEEVENTREGISTRYENTRY_H
#define GAUDIHIVE_HIVEEVENTREGISTRYENTRY_H

// STL include files
#include <string>
#include <vector>

// Framework include files
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/RegistryEntry.h"

// tbb include files
#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_vector.h"

// Forward declarations
// class DataSvc;
class DataObject;
class IDataProviderSvc;
class IOpaqueAddress;
class IDataStoreAgent;

namespace Hive
{

  class HiveRegistryEntry;

  class GAUDI_API HiveEventRegistryEntry : public DataSvcHelpers::RegistryEntry
  {
  public:
    friend class ::DataSvc;

  private:
    /// registry for all data published in the event; getting read from time to time
    tbb::concurrent_queue<std::string> m_publishedData;

  private:
    /** The following entries serve two aspects:
      1) They are faster for recursive calls, because they are non-virtual
      2) They can be re-used for the non-const entry points using a
         const_cast of the result.
    */
    /// Recursive helper to assemble the full path name of the entry
    void assemblePath( std::string& buffer ) const;
    /// internals of the adding to the data store
    HiveRegistryEntry* i_add( const std::string& );

  public:
    /// Standard Constructor
    HiveEventRegistryEntry( const std::string& path, DataSvcHelpers::RegistryEntry* parent = 0 );
    /// Standard Destructor
    virtual ~HiveEventRegistryEntry();
    /// Add entry to data store
    virtual long add( const std::string& name, DataObject* pObject, bool is_soft = false );
    /// Add entry to data store
    virtual long add( const std::string& name, IOpaqueAddress* pAddress, bool is_soft = false );
    /// Add object to the container
    virtual long add( IRegistry* obj );
    /// Get list of new products
    virtual tbb::concurrent_queue<std::string>& new_products() { return m_publishedData; }
  };

} // namespace Hive
#endif // GAUDIHIVE_HIVEEVENTREGISTRYENTRY_H
