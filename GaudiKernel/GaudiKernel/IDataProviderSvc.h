// $Id: IDataProviderSvc.h,v 1.6 2005/01/19 08:23:16 mato Exp $
#ifndef GAUDIKERNEL_IDATAPROVIDERSVC_H
#define GAUDIKERNEL_IDATAPROVIDERSVC_H

// Framework include files
#include "GaudiKernel/IInterface.h"

// C/C++ include files
#include <string>

// Forward declarations
class DataObject;
class IOpaqueAddress;
class IRegistry;
class DataStoreItem;

/** @class IDataProviderSvc IDataProviderSvc.h GaudiKernel/IDataProviderSvc.h
 *
 *  Data provider interface definition.
 *  The data provider interface allows to:
 *  <UL>
 *  <LI> Define lists of items to be loaded automatically when a new root node
 *       is assigned to the data store.
 *  <LI> Register objects from the data store. Once objects are registered
 *       with the data store the client gives up ownership.
 *  <LI> Unregister objects from the data store. Unregistering an object
 *       means to actually delete all entries hanging below, not the object
 *       itself - since the client claims back ownership. Note that this is a
 *       very delicate operation - any reference to the object will be invalid.
 *       Even worse: invalid references cannot be detected.
 *  <LI> Retrieve objects to the data store. Depending ont he
 *       availibility of the requested object in the data store the
 *       represented object will be loaded if possible.
 *  <LI> Find objects beeing present in the store without actually creating
 *       the representation if the object is not present.
 *  <LI> Create 'soft' links between objects in the transient store.
 *  <LI> Request object updates.
 *  </UL>
 *
 *- Removed call to update registry entry state in the data service
 *
 *  @author Markus Frank
 *  @author Sebastien Ponce
 *  @version 1.2
 */
class GAUDI_API IDataProviderSvc: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IDataProviderSvc,3,0);

  enum { SEPARATOR = '/' };

  /** Register object with the data store.
   *  Connect the object identified by its pointer to the node object
   *  identified by its path.
   *  @param      fullPath    Path to parent node of the object.
   *  @param      pObject     Pointer to the object to be registered.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode registerObject(const std::string& fullPath,
                                    DataObject* pObject) = 0;

  /** Register object with the data store.
   *  Connect the object identified by the path to the parent object
   *  and the path of the object itself relative to the specified parent.
   *  @param      parentPath  Path to parent node of the object.
   *  @param      objectPath  Path of the object relative to the parent node
   *  @param      pObject     Pointer to the object to be registered.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode registerObject(const std::string& parentPath,
                                    const std::string& objectPath,
                                    DataObject* pObject) = 0;

  /** Register object with the data store.
   *  Connect the object identified by its pointer to the parent object
   *  and an integer identifier.
   *  @param      parentPath  Path to parent node of the object.
   *  @param      item        item number of the object linked to the parent
   *  @param      pObject     Pointer to the object to be registered.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode registerObject(const std::string& parentPath,
                                    int item,
                                    DataObject* pObject) = 0;

  /** Register object with the data store.
   *  Connect the object identified by its pointer to the parent object
   *  and the relative path of the object with respect to the parent.
   *  @param      parentObj   Pointer to parent object.
   *  @param      objectPath  Path of the object relative to the parent node
   *  @param      pObject     Pointer to the object to be connected.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode registerObject(DataObject* parentObj,
                                    const std::string& objectPath,
                                    DataObject* pObject) = 0;

  /** Register object with the data store.
   *  Connect the object identified by its pointer to the node object
   *  identified by its pointer.
   *  @param      parentObj   Pointer to parent object.
   *  @param      item        item number of the object linked to the parent
   *  @param      pObject     Pointer to the object to be connected.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode registerObject(DataObject* parentObj,
                                    int item,
                                    DataObject* pObject) = 0;

  /** Unregister object from the data store.
   *  On registration the client gives up ownership of the object and may no
   *  longer delete the object. unregistering the object is the opposite:
   *  ownership is claimed back by the user. But note:
   *  - All depending objects will be deleted, ie. all leaves "below" the
   *    entry in question; NOT the object itself, the object itself must be
   *    destroyed by the user.
   *
   *  The object is identified by full path name.
   *
   *  @param      fullPath    Path name of the object.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode unregisterObject(const std::string& fullPath) = 0;

  /** Unregister object from the data store.
   *  On registration the client gives up ownership of the object and may no
   *  longer delete the object. unregistering the object is the opposite:
   *  ownership is claimed back by the user. But note:
   *  - All depending objects will be deleted, ie. all leaves "below" the
   *    entry in question; NOT the object itself, the object itself must be
   *    destroyed by the user.
   *
   *  The object is identified by parent path name and the path of the
   *  object relative to the parent.
   *  @param      parentPath  Path name of the parent object.
   *  @param      objPath     Path name of the object relative to the parent.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode unregisterObject(const std::string& parentPath,
                                      const std::string& objPath) = 0;

  /** Unregister object from the data store.
   *  On registration the client gives up ownership of the object and may no
   *  longer delete the object. unregistering the object is the opposite:
   *  ownership is claimed back by the user. But note:
   *  - All depending objects will be deleted, ie. all leaves "below" the
   *    entry in question; NOT the object itself, the object itself must be
   *    destroyed by the user.
   *
   *  The object is identified by parent path name and
   *  an integer identifier of the object itself.
   *  @param      parentPath  Path name of the parent object.
   *  @param      item        Integer identifier of the object item.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode unregisterObject(const std::string& parentPath,
                                      int item) = 0;

  /** Unregister object from the data store.
   *  On registration the client gives up ownership of the object and may no
   *  longer delete the object. unregistering the object is the opposite:
   *  ownership is claimed back by the user. But note:
   *  - All depending objects will be deleted, ie. all leaves "below" the
   *    entry in question; NOT the object itself, the object itself must be
   *    destroyed by the user.
   *
   *  The object is identified by its pointer.
   *  The object must previously have been registered with the data store.
   *  @param      pObject     Pointer to the object.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode unregisterObject(DataObject* pObject) = 0;

  /** Unregister object from the data store.
   *  On registration the client gives up ownership of the object and may no
   *  longer delete the object. unregistering the object is the opposite:
   *  ownership is claimed back by the user. But note:
   *  - All depending objects will be deleted, ie. all leaves "below" the
   *    entry in question; NOT the object itself, the object itself must be
   *    destroyed by the user.
   *
   *  The object is identified by parent object and the path of the
   *  object relative to the parent.
   *  @param      pParent     Pointer to parent object.
   *  @param      objPath     Path name of the object relative to the parent.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode unregisterObject(DataObject* pParent,
                                      const std::string& objPath) = 0;

  /** Unregister object from the data store.
   *  On registration the client gives up ownership of the object and may no
   *  longer delete the object. unregistering the object is the opposite:
   *  ownership is claimed back by the user. But note:
   *  - All depending objects will be deleted, ie. all leaves "below" the
   *    entry in question; NOT the object itself, the object itself must be
   *    destroyed by the user.
   *
   *  The object is identified by parent object and
   *  an integer identifier of the object itself.
   *  @param      pParent     Pointer to parent object.
   *  @param      item        Integer identifier of the object item.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode unregisterObject(DataObject* pParent, int item) = 0;

  /** Retrieve object identified by its directory entry.
   *  The result will be returned in the second argument.
   *  In case the object is not present it will be loaded and converted
   *  if possible.
   *  ** FASTEST ACCESS TO THE DATA STORE **
   *  @param  pDirectory  Pointer to the object.
   *  @param  path        String with relative path to directory. Ideally ""!
   *  @param  pObject     Reference to the pointer of the object to be returned.
   *  @return             Status code indicating success or failure.
   */
  virtual StatusCode retrieveObject(IRegistry* pDirectory,
                                    const std::string& path,
                                    DataObject*& pObject) = 0;

  /** Retrieve object identified by its full path from the data store.
   *  The result will be returned in the second argument.
   *  In case the object is not present it will be loaded and converted
   *  if possible.
   *  @param  fullPath    Path name of the object.
   *  @param  pObject     Reference to the pointer of the object to be returned.
   *  @return             Status code indicating success or failure.
   */
  virtual StatusCode retrieveObject(const std::string& fullPath,
                                    DataObject*& pObject) = 0;

  /** Retrieve object from data store.
   *  The object to be retrieved is identified by the path of the parent object
   *  and the relative path with respect to the node.
   *  In case the object is not present it will be loaded and converted
   *  if possible.
   *  @param  parentPath  Path to parent node of the object.
   *  @param  objectPath  Path of the object relative to the parent.
   *  @param  pObject     Reference to the pointer of the object to be returned.
   *  @return             Status code indicating success or failure.
   */
  virtual StatusCode retrieveObject(const std::string& parentPath,
                                    const std::string& objectPath,
                                    DataObject*& pObject) = 0;

  /** Retrieve object from data store.
   *  The object to be retrieved is identified by the path to the parent object
   *  and an integer identifier.
   *  In case the object is not present it will be loaded and converted
   *  if possible.
   *  @param  parentPath  Path to parent node of the object.
   *  @param  item        Item identifier.
   *  @param  pObject     Reference to the pointer of the object to be returned.
   *  @return             Status code indicating success or failure.
   */
  virtual StatusCode retrieveObject(const std::string& parentPath,
                                    int item, DataObject*& pObject) = 0;

  /** Retrieve object from data store.
   *  The object to be retrieved is identified by the pointer to the parent
   *  object and the relative path with respect to the node.
   *  In case the object is not present it will be loaded and converted
   *  if possible.
   *  @param  parentObj   Pointer to parent node of the object.
   *  @param  objectPath  Path of the object relative to the parent.
   *  @param  pObject     Reference to the pointer of the object to be returned.
   *  @return             Status code indicating success or failure.
   */
  virtual StatusCode retrieveObject(DataObject* parentObj,
                                    const std::string& objectPath,
                                    DataObject*& pObject) = 0;

  /** Retrieve object from data store.
   *  The object to be retrieved is identified by the pointer to the parent
   *  object and an integer identifier.
   *  In case the object is not present it will be loaded and converted
   *  if possible.
   *  @param  parentObj   Pointer to parent node of the object.
   *  @param  item        Item identifier.
   *  @param  pObject     Reference to the pointer of the object to be returned.
   *  @return             Status code indicating success or failure.
   */
  virtual StatusCode retrieveObject(DataObject* parentObj,
                                    int item,
                                    DataObject*& pObject) = 0;

  /** Find object identified by its directory entry.
   *  The result will be returned in the second argument.
   *  In case the object is not present the pointer will be set to NULL.
   *  ** FASTEST ACCESS TO THE DATA STORE **
   *  @param  pDirectory  Pointer to the object.
   *  @param  path        String with relative path to directory. Ideally ""!
   *  @param  pObject     Reference to the pointer of the object to be returned.
   *  @return             Status code indicating success or failure.
   */
  virtual StatusCode findObject(IRegistry* pDirectory,
                                const std::string& path,
                                DataObject*& pObject) = 0;

  /** Find object identified by its full path in the data store.
   *  The result will be returned in the second argument.
   *  In case the object is not present the pointer will be set to NULL.
   *  @param      fullPath    Path name of the object.
   *  @param      pObject     Pointer to the object to be connected.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode findObject(const std::string& fullPath,
                                DataObject*& pObject) = 0;

  /** Find object identified by its parent object and the path to the object
   *  relative to the parent.
   *  The result will be returned in the second argument.
   *  In case the object is not present the pointer will be set to NULL.
   *  @param  parentPath  Path to parent node of the object.
   *  @param  objectPath  Relative path name of the object.
   *  @param  pObject     Reference to the pointer of the object to be returned.
   *  @return             Status code indicating success or failure.
   */
  virtual StatusCode findObject(const std::string& parentPath,
                                const std::string& objectPath,
                                DataObject*& pObject) = 0;

  /** Find object identified by its parent object and an integer identifier
   *  in the data store.
   *  In case the object is not present the pointer will be set to NULL.
   *  @param  parentPath  Path to parent node of the object.
   *  @param  item        Item identifier.
   *  @param  pObject     Reference to the pointer of the object to be returned.
   *  @return             Status code indicating success or failure.
   */
  virtual StatusCode findObject(const std::string& parentPath,
                                int item,
                                DataObject*& pObject) = 0;

  /** Find object identified by its parent object and the path to the object
   *  relative to the parent.
   *  The result will be returned in the second argument.
   *  In case the object is not present the pointer will be set to NULL.
   *  @param  parentObj   Pointer to parent node of the object.
   *  @param  objectPath  Relative path name of the object.
   *  @param  pObject     Reference to the pointer of the object to be returned.
   *  @return             Status code indicating success or failure.
   */
  virtual StatusCode findObject(DataObject* pNode,
                                const std::string& objectPath,
                                DataObject*& pObject) = 0;

  /** Find object identified by its parent object and an integer identifier
   *  in the data store.
   *  In case the object is not present the pointer will be set to NULL.
   *  @param  parentObj   Pointer to parent node of the object.
   *  @param  item        Item identifier.
   *  @param  pObject     Reference to the pointer of the object to be returned.
   *  @return             Status code indicating success or failure.
   */
  virtual StatusCode findObject(DataObject* parentObj,
                                int item,
                                DataObject*& pObject) = 0;

  /** Update object identified by its directory entry.
   *  ** FASTEST ACCESS TO THE DATA STORE **
   *  @param      pDirectory  Pointer to the directory entry.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode updateObject(IRegistry* pDirectory) = 0;

  /** Update object identified by its full path in the data store.
   *  If found, the object update will be initiated.
   *  In case the object is not present the entry point returns an error.
   *  @param      fullPath    Path name of the object.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode updateObject(const std::string& fullPath) = 0;

  /** Update object identified by its pointer.
   *  If found, the object update will be initiated.
   *  In case the object is not present the entry point returns an error.
   *  @param      toUpdate    Pointer to the object.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode updateObject(DataObject* toUpdate) = 0;

  /** Update object identified by its parent's path and the path relative
   *  to the parent.
   *  If found, the object update will be initiated.
   *  In case the object is not present the entry point returns an error.
   *  @param      parentPath  Path name of the parent object.
   *  @param      updatePath  Path to the object relative to the parent.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode updateObject(const std::string& parentPath,
                                  const std::string& updatePath) = 0;

  /** Update object identified by its parent's pointer and the path relative
   *  to the parent.
   *  If found, the object update will be initiated.
   *  In case the object is not present the entry point returns an error.
   *  @param      pParent     Pointer to the parent object.
   *  @param      updatePath  Path to the object relative to the parent.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode updateObject(DataObject* pParent,
                                  const std::string& updatePath) = 0;

  /** Add an item to the preload list
   *  @param      item        Specs of item to be preloaded
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode addPreLoadItem(const DataStoreItem& item) = 0;

  /** Add an item to the preload list. The item is identified by the path
   *  to the object.
   *  @param      itemPath    Path to the item to be preloaded.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode addPreLoadItem(const std::string& itemPath) = 0;

  /** Remove an item from the preload list.
   *  @param      item        Specs of item to be removed from the preload list
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode removePreLoadItem(const DataStoreItem& item) = 0;

  /** Remove an item from the preload list.
   *  @param      itemPath    Path to the item to be preloaded.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode removePreLoadItem(const std::string& itemPath) = 0;

  /** Clear the preload list.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode resetPreLoad() = 0;

  /** Load all preload items of the list.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode preLoad() = 0;

  /** Add a link to another object.
   *  Both objects must already be registered with the data store.
   *  Once linked, the object can only be unregistered after unlinking.
   *  @param      from        Pointer to data directory the link originates.
   *  @param      objPath     Path of the entry to be linked relative to from.
   *  @param      toObj       Pointer to the object the link points to.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode linkObject(IRegistry* from,
                                const std::string& objPath,
                                DataObject* toObj) = 0;

  /** Add a link to another object.
   *  Both objects must already be registered with the data store.
   *  Once linked, the object can only be unregistered after unlinking.
   *  @param      fromPath    Path to the object the link originates.
   *  @param      objPath     Path of the entry to be linked relative to from.
   *  @param      toObj       Pointer to the object the link points to.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode linkObject(const std::string& fromPath,
                                const std::string& objPath,
                                DataObject* toObj) = 0;

  /** Add a link to another object.
   *  Both objects must already be registered with the data store.
   *  Once linked, the object can only be unregistered after unlinking.
   *  @param      fromObj     Pointer to the object the link originates.
   *  @param      objPath     Path of the entry to be linked relative to from.
   *  @param      toObj       Pointer to the object the link points to.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode linkObject(DataObject* fromObj,
                                const std::string& objPath,
                                DataObject* toObj) = 0;

  /** Add a link to another object.
   *  Both objects must already be registered with the data store.
   *  Once linked, the object can only be unregistered after unlinking.
   *  @param      fullPath    Full path of the entry to be linked.
   *  @param      toObj       Pointer to the object the link points to.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode linkObject(const std::string& fullPath,
                                DataObject* toObj) = 0;

  /** Remove a link to another object.
   *  Both objects must be registered with the data store.
   *  This entry point can be used to unlink objects e.g. in order to unregister
   *  them.
   *  @param      from        Pointer to data directory the link originates.
   *  @param      objPath     Path of the entry to be linked relative to from.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode unlinkObject(IRegistry* from,
                                  const std::string& objPath) = 0;

  /** Remove a link to another object.
   *  Both objects must be registered with the data store.
   *  This entry point can be used to unlink objects e.g. in order to unregister
   *  them.
   *  @param      fromPath    Path to the object the link originates.
   *  @param      objPath     Path of the entry to be linked relative to from.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode unlinkObject(const std::string& fromPath,
                                  const std::string& objPath) = 0;

  /** Remove a link to another object.
   *  Both objects must be registered with the data store.
   *  This entry point can be used to unlink objects e.g. in order to unregister
   *  them.
   *  @param      fromObj     Pointer to the object the link originates.
   *  @param      objPath     Path of the entry to be linked relative to from.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode unlinkObject(DataObject* fromObj,
                                  const std::string& objPath) = 0;

  /** Remove a link to another object.
   *  Both objects must be registered with the data store.
   *  This entry point can be used to unlink objects e.g. in order to unregister
   *  them.
   *  @param      fullPath    Full path of the entry to be linked.
   *  @return                 Status code indicating success or failure.
   */
  virtual StatusCode unlinkObject(const std::string& fullPath) = 0;

  /// Status code definitions
  enum Status  {
    /// Success
    IDataProviderSvc_NO_ERROR = 1,
    /// The path for this objects is already in use
    DOUBL_OBJ_PATH,
    /// Invalid path from root to object request failed.
    INVALID_OBJ_PATH,
    /// Invalid root path object cannot be retrieved or stored.
    INVALID_ROOT,
    /// Object pointer is invalid
    INVALID_OBJECT,
    /// Pointer to parent object is not valid
    INVALID_PARENT,
    /// Sorry, the requested object is not loaded
    OBJ_NOT_LOADED,
    /// No data loader available
    NO_DATA_LOADER,
    /// Invalid object address
    INVALID_OBJ_ADDR,
    /// Directory entry is NOT empty
    DIR_NOT_EMPTY,
    /// Automatic data loading had to stop: maximum depth
    NO_MORE_LEVELS,
    /// Access to the requested leaf is inhibited
    NO_ACCESS,
    /// Terminator
    LAST
  };
};

#endif // GAUDIKERNEL_IDATAPROVIDERSVC_H
