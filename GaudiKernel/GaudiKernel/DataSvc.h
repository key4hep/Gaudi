#ifndef GAUDIKERNEL_DATASVC_H
#define GAUDIKERNEL_DATASVC_H

// Include files
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/RegistryEntry.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SmartIF.h"

#include <string_view>

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
namespace DataSvcHelpers {
  // Map of objects where loading is inhibited
  class InhibitMap;
} // namespace DataSvcHelpers

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
class GAUDI_API DataSvc : public extends<Service, IDataProviderSvc, IDataManagerSvc> {

  /// Pointer to data loader service
  SmartIF<IConversionSvc> m_dataLoader = nullptr;

protected:
  /// Pointer to incident service
  SmartIF<IIncidentSvc> m_incidentSvc = nullptr;

  Gaudi::Property<CLID>        m_rootCLID{this, "RootCLID", 110 /*CLID_Event*/, "CLID of root entry"};
  Gaudi::Property<std::string> m_rootName{this, "RootName", "/Event", "name of root entry"};

private:
  Gaudi::Property<bool> m_forceLeaves{this, "ForceLeaves", false, "force creation of default leaves on registerObject"};
  Gaudi::Property<std::vector<std::string>> m_inhibitPathes{this, "InhibitPathes", {}, "inhibited leaves"};

  Gaudi::Property<bool>        m_enableFaultHdlr{this, "EnableFaultHandler", false,
                                          "enable incidents on data creation requests"};
  Gaudi::Property<std::string> m_faultName{this, "DataFaultName", "DataFault", "Name of the data fault incident"};

  Gaudi::Property<bool>        m_enableAccessHdlr{this, "EnableAccessHandler", false,
                                           "enable incidents on data access requests"};
  Gaudi::Property<std::string> m_accessName{this, "DataAccessName", "DataAccess", "Name of the data access incident"};

  /// Items to be pre-loaded
  std::vector<DataStoreItem> m_preLoads;

protected:
  /// Pointer to root entry
  std::unique_ptr<DataSvcHelpers::RegistryEntry> m_root;

private:
  /// Map with object paths to be inhibited from loading
  DataSvcHelpers::InhibitMap* m_inhibitMap = nullptr;

public:
  /// IDataManagerSvc: Accessor for root event CLID
  CLID rootCLID() const override;

  /// IDataManagerSvc: Accessor for root event name
  const std::string& rootName() const override;

  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( std::string_view fullPath, IOpaqueAddress* pAddress ) override;

  using IDataManagerSvc::registerAddress;

  /// IDataManagerSvc: Register object address with the data store.
  StatusCode registerAddress( IRegistry* parentObj, std::string_view objectPath, IOpaqueAddress* pAddress ) override;

  using IDataManagerSvc::unregisterAddress;

  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( std::string_view fullPath ) override;

  /// IDataManagerSvc: Unregister object address from the data store.
  StatusCode unregisterAddress( IRegistry* pParent, std::string_view objPath ) override;

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
  StatusCode clearSubTree( std::string_view sub_tree_path ) override;

  /** IDataManagerSvc: Remove all data objects below the sub tree identified
   *  by the object.
   */
  StatusCode clearSubTree( DataObject* pObject ) override;

  /// IDataManagerSvc: Remove all data objects in the data store.
  StatusCode clearStore() override;

  /** IDataManagerSvc: Analyze by traversing all data objects below the sub
   *  tree identified by its full path name.
   */
  StatusCode traverseSubTree( std::string_view sub_tree_path, IDataStoreAgent* pAgent ) override;

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

  /// Remove an item from the preload list
  StatusCode removePreLoadItem( const DataStoreItem& item ) override;

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

  using IDataProviderSvc::registerObject;

  /// Register object with the data store.
  StatusCode registerObject( std::string_view parentPath, std::string_view objPath, DataObject* pObject ) override;

  /// Register object with the data store.
  StatusCode registerObject( DataObject* parentObj, std::string_view objPath, DataObject* pObject ) override;

  /// Unregister object from the data store.
  StatusCode unregisterObject( std::string_view fullPath ) override;

  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObject ) override;

  /// Unregister object from the data store.
  StatusCode unregisterObject( DataObject* pObject, std::string_view objectPath ) override;

  using IDataProviderSvc::retrieveObject;

  /// Retrieve object from data store.
  StatusCode retrieveObject( IRegistry* pDirectory, std::string_view path, DataObject*& pObject ) override;

  using IDataProviderSvc::findObject;

  /// Find object identified by its full path in the data store.
  StatusCode findObject( std::string_view fullPath, DataObject*& pObject ) override;

  /// Find object identified by its full path in the data store.
  StatusCode findObject( IRegistry* pDirectory, std::string_view path, DataObject*& pObject ) override;

  using IDataProviderSvc::linkObject;

  /// Add a link to another object.
  StatusCode linkObject( IRegistry* from, std::string_view objPath, DataObject* to ) override;

  /// Add a link to another object.
  StatusCode linkObject( std::string_view fullPath, DataObject* to ) override;

  using IDataProviderSvc::unlinkObject;

  /// Remove a link to another object.
  StatusCode unlinkObject( IRegistry* from, std::string_view objPath ) override;

  /// Remove a link to another object.
  StatusCode unlinkObject( DataObject* fromObj, std::string_view objPath ) override;

  /// Remove a link to another object.
  StatusCode unlinkObject( std::string_view fullPath ) override;

  /// Update object identified by its directory entry.
  StatusCode updateObject( IRegistry* pDirectory ) override;

  /// Update object.
  StatusCode updateObject( DataObject* toUpdate ) override;

  /// Service initialization
  StatusCode initialize() override;

  /// Service initialization
  StatusCode reinitialize() override;

  /// Service initialization
  StatusCode finalize() override;

  /// inherit contructor
  using extends::extends;

  /// copy constructor disabled
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
  StatusCode retrieveEntry( DataSvcHelpers::RegistryEntry* pNode, std::string_view path,
                            DataSvcHelpers::RegistryEntry*& pEntry );
  /** Invoke data fault handling if enabled
   * @param pReg  [IN]   Pointer to missing registry entry
   * @param path  [IN]   Sub-path of requested object from pReg
   *
   * @return Object corresponding to the specified leaf
   */
  DataObject* handleDataFault( IRegistry* pReg, std::string_view path = "" );

private:
  StatusCode  i_retrieveEntry( DataSvcHelpers::RegistryEntry* parentObj, std::string_view path,
                               DataSvcHelpers::RegistryEntry*& pEntry );
  DataObject* i_handleDataFault( IRegistry* pReg, std::string_view path = std::string_view{} );
};
#endif // GAUDIKERNEL_DATASVC_H
