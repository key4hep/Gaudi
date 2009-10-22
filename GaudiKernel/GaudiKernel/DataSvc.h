// $Id: DataSvc.h,v 1.14 2006/11/30 14:43:13 mato Exp $
#ifndef GAUDIKERNEL_DATASVC_H
#define GAUDIKERNEL_DATASVC_H

// Include files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"

// Forward declarations
// Incident service
class IIncidentSvc;
// Generic address
class IOpaqueAddress;
// Generic interface to data object class
class DataObject;
// Data store agent
class IDataStoreAgent;
// Service factory
template <class TYPE> class SvcFactory;

// Do not clutter global namespace for helpers...
namespace DataSvcHelpers    {
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
class GAUDI_API DataSvc: public extends2<Service, IDataProviderSvc, IDataManagerSvc>
{
  /// The class creator is of course a friend!
  friend class SvcFactory<DataSvc>;

public:
  /// Define set of load items
  typedef std::vector<DataStoreItem> LoadItems;

protected:
  /// Integer Property corresponding to CLID of root entry
  CLID                            m_rootCLID;
  /// Name of root event
  std::string                     m_rootName;
  /// Pointer to data loader service
  IConversionSvc*                 m_dataLoader;
  /// Pointer to incident service
  IIncidentSvc*                   m_incidentSvc;
  /// Items to be pre-loaded
  LoadItems                       m_preLoads;
  /// Allow forced creation of default leaves on registerObject
  bool                            m_forceLeaves;
  /// Flag to enable interrupts on data access requests
  bool                            m_enableAccessHdlr;
  /// Flag to enable interrupts on data creation requests
  bool                            m_enableFaultHdlr;
  /// Pointer to root entry
  DataSvcHelpers::RegistryEntry*  m_root;
  /// Map with object paths to be inhibited from loading
  DataSvcHelpers::InhibitMap*     m_inhibitMap;
  /// Property for the inhibited leaves
  std::vector<std::string>        m_inhibitPathes;
  /// Name of the data access incident
  std::string                     m_accessName;
  /// Name of the data fault incident
  std::string                     m_faultName;
public:

  /// IDataManagerSvc: Accessor for root event CLID
  virtual CLID rootCLID() const;

  /// IDataManagerSvc: Accessor for root event name
  virtual std::string rootName() const;

  /// IDataManagerSvc: Register object address with the data store.
  virtual StatusCode registerAddress( const std::string& fullPath,
                                      IOpaqueAddress* pAddress);

  /// IDataManagerSvc: Register object address with the data store.
  virtual StatusCode registerAddress( DataObject* parentObj,
                                      const std::string& objectPath,
                                      IOpaqueAddress* pAddress);

  /// IDataManagerSvc: Register object address with the data store.
  virtual StatusCode registerAddress( IRegistry* parentObj,
                                      const std::string& objectPath,
                                      IOpaqueAddress* pAddress);

  /// IDataManagerSvc: Unregister object address from the data store.
  virtual StatusCode unregisterAddress(const std::string& fullPath);

  /// IDataManagerSvc: Unregister object address from the data store.
  virtual StatusCode unregisterAddress( DataObject* pParent,
                                        const std::string& objPath);

  /// IDataManagerSvc: Unregister object address from the data store.
  virtual StatusCode unregisterAddress( IRegistry* pParent,
                                        const std::string& objPath);

  /** IDataManagerSvc: Explore the object store: retrieve all leaves attached
   *  to the object
   */
  virtual StatusCode objectLeaves( const DataObject*  pObject,
                                  std::vector<IRegistry*>& refLeaves);
  /** IDataManagerSvc: Explore the object store: retrieve all leaves attached
   *  to the object
   */
  virtual StatusCode objectLeaves( const IRegistry*   pRegistry,
                                  std::vector<IRegistry*>& refLeaves);

  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  virtual StatusCode objectParent( const DataObject*  pObject,
                                   IRegistry*& refpParent);
  /// IDataManagerSvc: Explore the object store: retrieve the object's parent
  virtual StatusCode objectParent( const IRegistry*   pRegistry,
                                   IRegistry*& refpParent);

  /** IDataManagerSvc: Remove all data objects below the sub tree identified
   *  by its full path name.
   */
  virtual StatusCode clearSubTree( const std::string& sub_tree_path );

  /** IDataManagerSvc: Remove all data objects below the sub tree identified
   *  by the object.
   */
  virtual StatusCode clearSubTree( DataObject* pObject );

  /// IDataManagerSvc: Remove all data objects in the data store.
  virtual StatusCode clearStore();

  /** IDataManagerSvc: Analyze by traversing all data objects below the sub
   *  tree identified by its full path name.
   */
  virtual StatusCode traverseSubTree( const std::string& sub_tree_path,
                                      IDataStoreAgent* pAgent );

  /// IDataManagerSvc: Analyze by traversing all data objects below the sub tree
  virtual StatusCode traverseSubTree( DataObject* pObject,
                                      IDataStoreAgent* pAgent );

  /// IDataManagerSvc: Analyze by traversing all data objects in the data store.
  virtual StatusCode traverseTree( IDataStoreAgent* pAgent );

  /** Initialize data store for new event by giving new event path and root
      object. Takes care to clear the store before reinitializing it  */
  virtual StatusCode setRoot( const std::string& root_name,
                              DataObject* pRootObj);

  /** Initialize data store for new event by giving new event path and root
      object. Does not clear the store before reinitializing it. This could
      lead to errors and should be handle with care. Use setRoot if unsure */
  virtual StatusCode i_setRoot( const std::string& root_name,
                                DataObject* pRootObj);

  /** Initialize data store for new event by giving new event path and address
      of root object. Takes care to clear the store before reinitializing it */
  virtual StatusCode setRoot (const std::string& root_path,
                              IOpaqueAddress* pRootAddr);

  /** Initialize data store for new event by giving new event path and address
   *  of root object. Does not clear the store before reinitializing it. This
   *  could lead to errors and should be handle with care. Use setRoot if unsure
   */
  virtual StatusCode i_setRoot (const std::string& root_path,
                                IOpaqueAddress* pRootAddr);

  /** IDataManagerSvc: IDataManagerSvc: Pass a default data loader to the
   *  service.
   */
  virtual StatusCode setDataLoader( IConversionSvc* svc );

  /// Add an item to the preload list
  virtual StatusCode addPreLoadItem( const DataStoreItem& item );

  /// Add an item to the preload list
  virtual StatusCode addPreLoadItem( const std::string& itemPath );

  /// Remove an item from the preload list
  virtual StatusCode removePreLoadItem( const DataStoreItem& item );

  /// Add an item to the preload list
  virtual StatusCode removePreLoadItem( const std::string& itemPath );

  /// Clear the preload list
  virtual StatusCode resetPreLoad();

  /** Execute one level of preloading and recursively load until the
      final level is reached.
      @param   depth        current level of loading from requested parent
      @param   load_depth   maximum level of object loading
      @param   pObject      pointer to next level root object
      @return  Status code indicating success or failure.
  */
  virtual StatusCode preLoad( int depth,
                              int load_depth,
                              DataObject* pObject );

  /// load all preload items of the list
  virtual StatusCode preLoad();

  /// Register object with the data store.
  virtual StatusCode registerObject(  const std::string& fullPath,
                                      DataObject* pObject );

  /// Register object with the data store.
  virtual StatusCode registerObject(  const std::string& parentPath,
                                      const std::string& objPath,
                                      DataObject* pObject );

  /// Register object with the data store.
  virtual StatusCode registerObject(  const std::string& parentPath,
                                      int item,
                                      DataObject* pObject );

  /// Register object with the data store.
  virtual StatusCode registerObject(  DataObject* parentObj,
                                      const std::string& objPath,
                                      DataObject* pObject );

  /// Register object with the data store.
  virtual StatusCode registerObject(  DataObject* parentObj,
                                      int item,
                                      DataObject* pObject );

  /// Unregister object from the data store.
  virtual StatusCode unregisterObject(const std::string& fullPath );

  /// Unregister object from the data store.
  virtual StatusCode unregisterObject(const std::string& parentPath,
                                      const std::string& objectPath );

  /// Unregister object from the data store.
  virtual StatusCode unregisterObject(const std::string& parentPath,
                                      int item );

  /// Unregister object from the data store.
  virtual StatusCode unregisterObject(DataObject* pObject );

  /// Unregister object from the data store.
  virtual StatusCode unregisterObject(DataObject* pObject,
                                      const std::string& objectPath );

  /// Unregister object from the data store.
  virtual StatusCode unregisterObject(DataObject* pObject,
                                      int item );

  /// Retrieve object from data store.
  virtual StatusCode retrieveObject(  IRegistry* pDirectory,
                                      const std::string& path,
                                      DataObject*& pObject );

  /// Retrieve object identified by its full path from the data store.
  virtual StatusCode retrieveObject(  const std::string& fullPath,
                                      DataObject*& pObject );

  /// Retrieve object from data store.
  virtual StatusCode retrieveObject(  const std::string& parentPath,
                                      const std::string& objPath,
                                      DataObject*& pObject );

  /// Retrieve object from data store.
  virtual StatusCode retrieveObject(  const std::string& parentPath,
                                      int item,
                                      DataObject*& pObject );

  /// Retrieve object from data store.
  virtual StatusCode retrieveObject(  DataObject* parentObj,
                                      const std::string& objPath,
                                      DataObject*& pObject );

  /// Retrieve object from data store.
  virtual StatusCode retrieveObject(  DataObject* parentObj,
                                      int item,
                                      DataObject*& pObject );

  /// Find object identified by its full path in the data store.
  virtual StatusCode findObject(  const std::string& fullPath,
                                  DataObject*& pObject );

  /// Find object identified by its full path in the data store.
  virtual StatusCode findObject(  IRegistry* pDirectory,
                                  const std::string& path,
                                  DataObject*& pObject );

  /// Find object in the data store.
  virtual StatusCode findObject(  const std::string& parentPath,
                                  const std::string& objPath,
                                  DataObject*& pObject );

  /// Find object in the data store.
  virtual StatusCode findObject(  const std::string& parentPath,
                                  int item,
                                  DataObject*& pObject );

  /// Find object in the data store.
  virtual StatusCode findObject(  DataObject* parentObj,
                                  const std::string& objPath,
                                  DataObject*& pObject );

  /// Find object in the data store.
  virtual StatusCode findObject(  DataObject* parentObj,
                                  int item,
                                  DataObject*& pObject );

  /// Add a link to another object.
  virtual StatusCode linkObject(  IRegistry* from,
                                  const std::string& objPath,
                                  DataObject* to );

  /// Add a link to another object.
  virtual StatusCode linkObject(  const std::string& fromPath,
                                  const std::string& objPath,
                                  DataObject* to );

  /// Add a link to another object.
  virtual StatusCode linkObject(  DataObject* from,
                                  const std::string& objPath,
                                  DataObject* to );

  /// Add a link to another object.
  virtual StatusCode linkObject(  const std::string& fullPath,
                                  DataObject* to );

  /// Remove a link to another object.
  virtual StatusCode unlinkObject(IRegistry* from,
                                  const std::string& objPath );

  /// Remove a link to another object.
  virtual StatusCode unlinkObject(const std::string& fromPath,
                                  const std::string& objPath );

  /// Remove a link to another object.
  virtual StatusCode unlinkObject(DataObject* fromObj,
                                  const std::string& objPath );

  /// Remove a link to another object.
  virtual StatusCode unlinkObject(const std::string& fullPath );

  /// Update object identified by its directory entry.
  virtual StatusCode updateObject(IRegistry* pDirectory );

  /// Update object.
  virtual StatusCode updateObject(const std::string& updatePath );

  /// Update object.
  virtual StatusCode updateObject(DataObject* toUpdate );

  /// Update object.
  virtual StatusCode updateObject(const std::string& parentPath,
                                  const std::string& updatePath );

  /// Update object.
  virtual StatusCode updateObject(DataObject* pParent,
                                  const std::string& updatePath );

  /// Service initialization
  virtual StatusCode initialize();

  /// Service initialization
  virtual StatusCode reinitialize();

  /// Service initialization
  virtual StatusCode finalize();

  /// Standard Constructor
  DataSvc( const std::string& name, ISvcLocator* svc );

  /// Standard Destructor
  virtual ~DataSvc();

private:
  /// Fake copy constructor (never implemented).
  DataSvc(const DataSvc&);
  /// Fake assignment operator (never implemented).
  DataSvc& operator= (const DataSvc&);

protected:
  /// Check if root path is valid
  bool checkRoot()    {
    return 0 != m_root;
  }

  /** Retrieve customizable data loader according to registry entry to be
   *  retrieved
   */
  virtual IConversionSvc* getDataLoader(IRegistry* pReg);

  /// Create default objects in case forced creation of leaves is requested
  virtual DataObject* createDefaultObject()   const;

  /** Invoke Persistency service to create transient object from its
   *  persistent representation
   */
  virtual StatusCode loadObject( IRegistry* pNode );

  /** Invoke Persistency service to create transient object from its
   *  persistent representation
   */
  virtual StatusCode loadObject( IConversionSvc* pLoader,
                                 IRegistry* pNode );

  /// Retrieve registry entry from store
  StatusCode retrieveEntry( DataSvcHelpers::RegistryEntry* pNode,
                            const std::string& path,
                            DataSvcHelpers::RegistryEntry*& pEntry );
  /** Invoke data fault handling if enabled
    * @param pReg  [IN]   Pointer to missing registry entry
    * @param path  [IN]   Sub-path of requested object from pReg
    *
    * @return Object corresponding to the specified leaf
    */
  DataObject* handleDataFault(IRegistry* pReg, const std::string& path="");
};
#endif // GAUDIKERNEL_DATASVC_H

