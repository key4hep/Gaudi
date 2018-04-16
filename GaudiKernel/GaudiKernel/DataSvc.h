#ifndef GAUDIKERNEL_DATASVC_H
#define GAUDIKERNEL_DATASVC_H

// Include files
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SmartIF.h"

#include <boost/utility/string_ref.hpp>

// Forward declarations
// Incident service
class IIncidentSvc;
// Generic address
class IOpaqueAddress;
// Generic interface to data object class
class DataObject;
// Data store agent
class IDataStoreAgent;

// Do not clutter global namespace for helpers...
namespace DataSvcHelpers
{
  // Map of objects where loading is inhibited
  class InhibitMap;
  // Generic registry entry
  class RegistryEntry;
}

/**
 * @class DataSvc DataSvc.h GaudiKernel/DataSvc.h
 *
 * Data service base class. A data service manages the transient data stores
 * and implements the IDataProviderSvc and IDataManagerSvc interfaces.
 *
 * @author Markus Frank
 * @author Sebastien Ponce
 * @version 1.0
*/
class GAUDI_API DataSvc : public extends<Service, IDataProviderSvc, IDataManagerSvc>
{
public:
  /// Define set of load items
  typedef std::vector<DataStoreItem> LoadItems;

protected:
  /// Pointer to data loader service
  SmartIF<IConversionSvc> m_dataLoader = nullptr;
  /// Pointer to incident service
  SmartIF<IIncidentSvc> m_incidentSvc = nullptr;

  Gaudi::Property<CLID>        m_rootCLID{this, "RootCLID", 110 /*CLID_Event*/, "CLID of root entry"};
  Gaudi::Property<std::string> m_rootName{this, "RootName", "/Event", "name of root entry"};
  Gaudi::Property<bool> m_forceLeaves{this, "ForceLeaves", false, "force creation of default leaves on registerObject"};
  Gaudi::Property<std::vector<std::string>> m_inhibitPathes{this, "InhibitPathes", {}, "inhibited leaves"};

  Gaudi::Property<bool> m_enableFaultHdlr{this, "EnableFaultHandler", false,
                                          "enable incidents on data creation requests"};
  Gaudi::Property<std::string> m_faultName{this, "DataFaultName", "DataFault", "Name of the data fault incident"};

  Gaudi::Property<bool> m_enableAccessHdlr{this, "EnableAccessHandler", false,
                                           "enable incidents on data access requests"};
  Gaudi::Property<std::string> m_accessName{this, "DataAccessName", "DataAccess", "Name of the data access incident"};

  /// Items to be pre-loaded
  LoadItems m_preLoads;
  /// Pointer to root entry
  DataSvcHelpers::RegistryEntry* m_root = nullptr;
  /// Map with object paths to be inhibited from loading
  DataSvcHelpers::InhibitMap* m_inhibitMap = nullptr;

public:
  /// IDataManagerSvc: Accessor for root event CLID
  CLID rootCLID() const override;

  /// IDataManagerSvc: Accessor for root event name
  const std::string& rootName() const override;

  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( const std::string& fullPath, IOpaqueAddress* pAddress ) override;

  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( DataObject* parentObj, const std::string& objectPath, IOpaqueAddress* pAddress ) override;

  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( IRegistry* parentObj, const std::string& objectPath, IOpaqueAddress* pAddress ) override;

  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( const std::string& fullPath ) override;

  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( DataObject* pParent, const std::string& objPath ) override;

  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( IRegistry* pParent, const std::string& objPath ) override;

  /** IDataManagerSvc: Explore the object store: retrieve all leaves attached
   *  to the object
   */
  StatusCode objectLeaves( const DataObject* pObject, std::vector<IRegistry*>& refLeaves ) override;
  /** IDataManagerSvc: Explore the object store: retrieve all leaves attached
   *  to the object
   */
  StatusCode objectLeaves( const IRegistry* pRegistry, std::vector<IRegistry*>& refLeaves ) override;

  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  StatusCode objectParent( const DataObject* pObject, IRegistry*& refpParent ) override;
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  StatusCode objectParent( const IRegistry* pRegistry, IRegistry*& refpParent ) override;

  /** IDataManagerSvc: Remove all data objects below the sub tree identified
   *  by its full path name.
   */
  StatusCode clearSubTree( const std::string& sub_tree_path ) override;

  /** IDataManagerSvc: Remove all data objects below the sub tree identified
   *  by the object.
   */
  StatusCode clearSubTree( DataObject* pObject ) override;

  /// IDataManagerSvc: Remove all data objects in the data store.
  StatusCode clearStore() override;

  /** IDataManagerSvc: Analyze by traversing all data objects below the sub
   *  tree identified by its full path name.
   */
  StatusCode traverseSubTree( const std::string& sub_tree_path, IDataStoreAgent* pAgent ) override;

  /// IDataManagerSvc: Analyze by traversing all data objects below the sub tree
  StatusCode traverseSubTree( DataObject* pObject, IDataStoreAgent* pAgent ) override;

  /// IDataManagerSvc: Analyze by traversing all data objects in the data store.
  StatusCode traverseTree( IDataStoreAgent* pAgent ) override;

  /** Initialize data store for new event by giving new event path and root
      object. Takes care to clear the store before reinitializing it  */
  StatusCode setRoot( std::string root_name, DataObject* pRootObj ) override;

  /** Initialize data store for new event by giving new event path and root
      object. Does not clear the store before reinitializing it. This could
      lead to errors and should be handle with care. Use setRoot if unsure */
  virtual StatusCode i_setRoot( std::string root_name, DataObject* pRootObj );

  /** Initialize data store for new event by giving new event path and address
      of root object. Takes care to clear the store before reinitializing it */
  StatusCode setRoot( std::string root_path, IOpaqueAddress* pRootAddr ) override;

  /** Initialize data store for new event by giving new event path and address
   *  of root object. Does not clear the store before reinitializing it. This
   *  could lead to errors and should be handle with care. Use setRoot if unsure
   */
  virtual StatusCode i_setRoot( std::string root_path, IOpaqueAddress* pRootAddr );

  /** IDataManagerSvc: IDataManagerSvc: Pass a default data loader to the
   *  service and optionally a data provider
   */
  StatusCode setDataLoader( IConversionSvc* svc, IDataProviderSvc* dpsvc = nullptr ) override;

  /// Add an item to the preload list
  StatusCode addPreLoadItem( const DataStoreItem& item ) override;

  /// Add an item to the preload list
  StatusCode addPreLoadItem( const std::string& itemPath ) override;

  /// Remove an item from the preload list
  StatusCode removePreLoadItem( const DataStoreItem& item ) override;

  /// Add an item to the preload list
  StatusCode removePreLoadItem( const std::string& itemPath ) override;

  /// Clear the preload list
  StatusCode resetPreLoad() override;

  /** Execute one level of preloading and recursively load until the
      final level is reached.
      @param   depth        current level of loading from requested parent
      @param   load_depth   maximum level of object loading
      @param   pObject      pointer to next level root object
      @return  Status code indicating success or failure.
  */
  virtual StatusCode preLoad( int depth, int load_depth, DataObject* pObject );

  /// load all preload items of the list
  StatusCode preLoad() override;

  /// Register object with the data store.
  StatusCode registerObject( const std::string& fullPath, DataObject* pObject ) override;

  /// Register object with the data store.
  StatusCode registerObject( const std::string& parentPath, const std::string& objPath, DataObject* pObject ) override;

  /// Register object with the data store.
  StatusCode registerObject( const std::string& parentPath, int item, DataObject* pObject ) override;

  /// Register object with the data store.
  StatusCode registerObject( DataObject* parentObj, const std::string& objPath, DataObject* pObject ) override;

  /// Register object with the data store.
  StatusCode registerObject( DataObject* parentObj, int item, DataObject* pObject ) override;

  /// Unregister object from the data store.
  StatusCode unregisterObject( const std::string& fullPath ) override;

  /// Unregister object from the data store.
  StatusCode unregisterObject( const std::string& parentPath, const std::string& objectPath ) override;

  /// Unregister object from the data store.
  StatusCode unregisterObject( const std::string& parentPath, int item ) override;

  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObject ) override;

  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObject, const std::string& objectPath ) override;

  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObject, int item ) override;

  /// Retrieve object from data store.
  StatusCode retrieveObject( IRegistry* pDirectory, const std::string& path, DataObject*& pObject ) override;

  /// Retrieve object identified by its full path from the data store.
  StatusCode retrieveObject( const std::string& fullPath, DataObject*& pObject ) override;

  /// Retrieve object from data store.
  StatusCode retrieveObject( const std::string& parentPath, const std::string& objPath, DataObject*& pObject ) override;

  /// Retrieve object from data store.
  StatusCode retrieveObject( const std::string& parentPath, int item, DataObject*& pObject ) override;

  /// Retrieve object from data store.
  StatusCode retrieveObject( DataObject* parentObj, const std::string& objPath, DataObject*& pObject ) override;

  /// Retrieve object from data store.
  StatusCode retrieveObject( DataObject* parentObj, int item, DataObject*& pObject ) override;

  /// Find object identified by its full path in the data store.
  StatusCode findObject( const std::string& fullPath, DataObject*& pObject ) override;

  /// Find object identified by its full path in the data store.
  StatusCode findObject( IRegistry* pDirectory, const std::string& path, DataObject*& pObject ) override;

  /// Find object in the data store.
  StatusCode findObject( const std::string& parentPath, const std::string& objPath, DataObject*& pObject ) override;

  /// Find object in the data store.
  StatusCode findObject( const std::string& parentPath, int item, DataObject*& pObject ) override;

  /// Find object in the data store.
  StatusCode findObject( DataObject* parentObj, const std::string& objPath, DataObject*& pObject ) override;

  /// Find object in the data store.
  StatusCode findObject( DataObject* parentObj, int item, DataObject*& pObject ) override;

  /// Add a link to another object.
  StatusCode linkObject( IRegistry* from, const std::string& objPath, DataObject* to ) override;

  /// Add a link to another object.
  StatusCode linkObject( const std::string& fromPath, const std::string& objPath, DataObject* to ) override;

  /// Add a link to another object.
  StatusCode linkObject( DataObject* from, const std::string& objPath, DataObject* to ) override;

  /// Add a link to another object.
  StatusCode linkObject( const std::string& fullPath, DataObject* to ) override;

  /// Remove a link to another object.
  StatusCode unlinkObject( IRegistry* from, const std::string& objPath ) override;

  /// Remove a link to another object.
  StatusCode unlinkObject( const std::string& fromPath, const std::string& objPath ) override;

  /// Remove a link to another object.
  StatusCode unlinkObject( DataObject* fromObj, const std::string& objPath ) override;

  /// Remove a link to another object.
  StatusCode unlinkObject( const std::string& fullPath ) override;

  /// Update object identified by its directory entry.
  StatusCode updateObject( IRegistry* pDirectory ) override;

  /// Update object.
  StatusCode updateObject( const std::string& updatePath ) override;

  /// Update object.
  StatusCode updateObject( DataObject* toUpdate ) override;

  /// Update object.
  StatusCode updateObject( const std::string& parentPath, const std::string& updatePath ) override;

  /// Update object.
  StatusCode updateObject( DataObject* pParent, const std::string& updatePath ) override;

  /// Service initialization
  StatusCode initialize() override;

  /// Service initialization
  StatusCode reinitialize() override;

  /// Service initialization
  StatusCode finalize() override;

  /// inherit contructor
  using extends::extends;

  /// Standard Destructor
  ~DataSvc() override;

private:
  /// Fake copy constructor (never implemented).
  DataSvc( const DataSvc& ) = delete;
  /// Fake assignment operator (never implemented).
  DataSvc& operator=( const DataSvc& ) = delete;

protected:
  /// Check if root path is valid
  bool checkRoot() { return 0 != m_root; }

  /** Retrieve customizable data loader according to registry entry to be
   *  retrieved
   */
  virtual IConversionSvc* getDataLoader( IRegistry* pReg );

  /// Create default objects in case forced creation of leaves is requested
  virtual DataObject* createDefaultObject() const;

  /** Invoke Persistency service to create transient object from its
   *  persistent representation
   */
  virtual StatusCode loadObject( IRegistry* pNode );

  /** Invoke Persistency service to create transient object from its
   *  persistent representation
   */
  virtual StatusCode loadObject( IConversionSvc* pLoader, IRegistry* pNode );

  /// Retrieve registry entry from store
  StatusCode retrieveEntry( DataSvcHelpers::RegistryEntry* pNode, const std::string& path,
                            DataSvcHelpers::RegistryEntry*& pEntry );
  /** Invoke data fault handling if enabled
    * @param pReg  [IN]   Pointer to missing registry entry
    * @param path  [IN]   Sub-path of requested object from pReg
    *
    * @return Object corresponding to the specified leaf
    */
  DataObject* handleDataFault( IRegistry* pReg, const std::string& path = "" );

private:
  StatusCode i_retrieveEntry( DataSvcHelpers::RegistryEntry* parentObj, boost::string_ref path,
                              DataSvcHelpers::RegistryEntry*& pEntry );
  DataObject* i_handleDataFault( IRegistry* pReg, boost::string_ref path = boost::string_ref{} );
};
#endif // GAUDIKERNEL_DATASVC_H
