/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_IDATAMANAGERSVC_H
#define GAUDIKERNEL_IDATAMANAGERSVC_H

// Include files
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/IDataStoreAgent.h"
#include "GaudiKernel/IInterface.h"
#include <string>
#include <string_view>
#include <vector>

// Forward declarations
// Generic interface to data object class
#include "GaudiKernel/DataObject.h"
// Interface to persistency service
class IConversionSvc;
// Opaque addresses
class IOpaqueAddress;
// Registry entry definition
class IRegistry;
// Data provider svc
class IDataProviderSvc;

/** @class IDataManagerSvc IDataManagerSvc.h GaudiKernel/IDataManagerSvc.h

    <P>The data manager interface of the service allows to:
    <UL>
    <LI> Discard sub-trees or the entire data stored.
    <LI> Initialize the top level root used for navigation.
    <LI> Access the tree information of the data store:
         Given a reference to a store object, which can be identified by
         path, pointer or reference to the registry entry, it is possible
         to retrieve the parent object and to access iterators over the
         leaf entries.
    <LI> Register addresses to the data store.
         Registered addresses contain the retrieve information for the
         underlying object.
    <LI> Unregister addresses from the data store. The object will no longer be
         accessible and loadable.
    </UL>

   @author Markus Frank
   @version 1.0
*/
struct GAUDI_API IDataManagerSvc : extend_interfaces<IInterface> {
  /// InterfaceID
  DeclareInterfaceID( IDataManagerSvc, 4, 0 );

  /// Get class ID of root Event
  virtual CLID rootCLID() const = 0;

  /// Get Name of root Event
  virtual const std::string& rootName() const = 0;

  /** Pass a default data loader to the service. This service will be
      asked to load non existant data items.
      @param      svc        [IN] Pointer to persistency service instance
      @param      dpsvc      [IN] Pointer to data provider instance (optional)
      @return                     Status code indicating success or failure
  */
  virtual StatusCode setDataLoader( IConversionSvc* svc, IDataProviderSvc* dpsvc = nullptr ) = 0;

  /** IDataManagerSvc: Explore the object store: retrieve the object's parent.
      The object is identified by its pointer.
      @param      pObject     [IN] Pointer to the object.
      @param      refpParent [OUT] Reference to store the parent's registry entry.
      @return                      Status code indicating success or failure.
  */
  virtual StatusCode objectParent( const DataObject* pObject, IRegistry*& refpParent ) = 0;

  /** IDataManagerSvc: Explore the object store: retrieve the object's parent.
      The object is identified by the pointer to the registry entry.
      @param      pRegistry   [IN] Pointer to the object.
      @param      refpParent [OUT] Reference to store the parent's registry entry.
      @return                      Status code indicating success or failure.
  */
  virtual StatusCode objectParent( const IRegistry* pRegistry, IRegistry*& refpParent ) = 0;

  /** Explore the object store: retrieve all leaves attached to the object
      The object is identified by its pointer.
      Allow to access and browse the leaf objects of the identified object.
      @param      pObject    [IN] Pointer to the object.
      @param      refLeaves  [OUT] Reference to storage location, where
                                  the objects leaves should be stored.
      @return                     Status code indicating success or failure.
  */
  virtual StatusCode objectLeaves( const DataObject* pObject, std::vector<IRegistry*>& refLeaves ) = 0;

  /** Explore the object store: retrieve all leaves attached to the object
      The object is identified by the pointer to the registry entry.
      Allow to access and browse the leaf objects of the identified object.
      @param      pRegistry  [IN] Pointer to the registry of the obejct.
      @param      refLeaves  [OUT] Reference to storage location, where
                                  the objects leaves should be stored.
      @return                     Status code indicating success or failure.
  */
  virtual StatusCode objectLeaves( const IRegistry* pRegistry, std::vector<IRegistry*>& refLeaves ) = 0;

  /** Remove all data objects below the sub tree identified by its full path name.
      @param      sub_path   [IN] Path to sub-tree node.
      @return                     Status code indicating success or failure.
  */
  virtual StatusCode clearSubTree( std::string_view sub_path ) = 0;

  /** Remove all data objects below the sub tree
      identified by the object. The object itself is removed as well.
      @param      pObject    [IN] Pointer to object
      @return                     Status code indicating success or failure
  */
  virtual StatusCode clearSubTree( DataObject* pObject ) = 0;

  /** Remove all data objects in the data store.
      @return                     Status code indicating success or failure
  */
  virtual StatusCode clearStore() = 0;

  /** Analyse by traversing all data objects below the sub tree identified by its full path name.
      @param      sub_path   [IN] Path to sub-tree node.
      @return                     Status code indicating success or failure.
  */
  virtual StatusCode traverseSubTree( std::string_view sub_tree_path, IDataStoreAgent* pAgent ) = 0;

  /** Analyse by traversing all data objects below the sub tree identified by its full path name.
      @param      sub_path   [IN] Path to sub-tree node.
      @param      f          [IN] callable which will be called for each data object
                                  it should have the signature bool(IRegistry*,int level)
      @return                     Status code indicating success or failure.
  */
  template <typename F, typename = std::enable_if_t<!std::is_convertible_v<F, IDataStoreAgent*>>>
  StatusCode traverseSubTree( std::string_view sub_path, F&& f ) {
    auto agent = makeDataStoreAgent( std::forward<F>( f ) );
    return traverseSubTree( sub_path, &agent );
  }

  /** Analyse by traversing all data objects below the sub tree
      identified by the object. The object itself is removed as well.
      @param      pObject    [IN] Pointer to object
      @param      pAgent     [IN] Pointer to the datastore agent traversing the store
      @return                     Status code indicating success or failure
  */
  virtual StatusCode traverseSubTree( DataObject* pObject, IDataStoreAgent* pAgent ) = 0;

  /** Analyse by traversing all data objects below the sub tree
      identified by the object. The object itself is removed as well.
      @param      pObject    [IN] Pointer to object
      @param      f          [IN] Callable which will be called on each data object
                                  it should have the signature bool(IRegistry*,int level)
      @return                     Status code indicating success or failure
  */
  template <typename F, typename = std::enable_if_t<!std::is_convertible_v<F, IDataStoreAgent*>>>
  StatusCode traverseSubTree( DataObject* pObject, F&& f ) {
    auto agent = makeDataStoreAgent( std::forward<F>( f ) );
    return traverseSubTree( pObject, &agent );
  }

  /** Analyse by traversing all data objects in the data store.
      @return     Status code indicating success or failure
  */
  virtual StatusCode traverseTree( IDataStoreAgent* pAgent ) = 0;

  /** Analyse by traversing all data objects in the data store.
      @param      f          [IN] callable which will be called for each data object
                                  it should have the signature bool(IRegistry*,int level)
      @return     Status code indicating success or failure
  */
  template <typename F, typename = std::enable_if_t<!std::is_convertible_v<F, IDataStoreAgent*>>>
  StatusCode traverseTree( F&& f ) {
    auto agent = makeDataStoreAgent( std::forward<F>( f ) );
    return traverseTree( &agent );
  }

  /** Initialize data store for new event by giving new event path.
      Implicitly this clears the entire data store.
      @param      root_name  [IN] String containing root path name
      @param      pObject    [IN] Pointer to root node object
      @return                     Status code indicating success or failure
  */
  virtual StatusCode setRoot( std::string root_name, DataObject* pObject ) = 0;

  /** Initialize data store for new event by giving new event path.
      Implicitly this clears the entire data store.
      @param      root_name  [IN] String containing root path name
      @param      pRootAddr  [IN] Pointer to opaque root node address
      @return                     Status code indicating success or failure
  */
  virtual StatusCode setRoot( std::string root_path, IOpaqueAddress* pRootAddr ) = 0;

  /** Register object address with the data store.
     Connect the object identified by its pointer to the node object
     identified by its path.
     @param      fullPath    [IN] Path to parent node of the object.
     @param      pAddress    [IN] Pointer to the object to be registered.
      @return                     Status code indicating success or failure.
  */
  virtual StatusCode registerAddress( std::string_view fullPath, IOpaqueAddress* pAddress ) = 0;

  /** Register object address with the data store.
      Connect the object identified by its pointer to the parent object
      and the relative path of the object with respect to the parent.
      @param      parentObj  [IN] Pointer to parent object.
      @param      objectPath [IN] Path of the object relative to the parent node
      @param      pAddress   [IN] Pointer to the object to be connected.
      @return                     Status code indicating success or failure.
  */
  StatusCode registerAddress( DataObject* parentObj, std::string_view objectPath, IOpaqueAddress* pAddress ) {
    return registerAddress( parentObj ? parentObj->registry() : nullptr, objectPath, pAddress );
  }

  /** Register object address with the data store.
      Connect the object identified by its pointer to the parent object
      and the relative path of the object with respect to the parent.
      @param      parentObj  [IN] Pointer to parent object.
      @param      objectPath [IN] Path of the object relative to the parent node
      @param      pAddress   [IN] Pointer to the object to be connected.
      @return                     Status code indicating success or failure.
  */
  virtual StatusCode registerAddress( IRegistry* parentObj, std::string_view objectPath, IOpaqueAddress* pAddress ) = 0;

  /** Unregister object address from the data store.
      The object is identified by full path name.
      @param      fullPath [IN] Path name of the object.
      @return                   Status code indicating success or failure.
  */
  virtual StatusCode unregisterAddress( std::string_view fullPath ) = 0;

  /** Unregister object address from the data store.
      The object is identified by parent object and the path of the
      object relative to the parent.
      @param      pParent  [IN] Pointer to parent object.
      @param      objPath  [IN] Path name of the object relative to the parent.
      @return                   Status code indicating success or failure.
  */
  StatusCode unregisterAddress( DataObject* pParent, std::string_view objPath ) {
    return unregisterAddress( pParent ? pParent->registry() : nullptr, objPath );
  }

  /** Unregister object address from the data store.
      The object is identified by parent object and the path of the
      object relative to the parent.
      @param      pParent  [IN] Pointer to parent object.
      @param      objPath  [IN] Path name of the object relative to the parent.
      @return                   Status code indicating success or failure.
  */
  virtual StatusCode unregisterAddress( IRegistry* pParent, std::string_view objPath ) = 0;
};

#endif // GAUDIKERNEL_IDATAMANAGERSVC_H
