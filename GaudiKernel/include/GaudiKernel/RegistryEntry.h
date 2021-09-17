/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_REGISTRYENTRY_H
#define GAUDIKERNEL_REGISTRYENTRY_H

// Framework include files
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/StatusCode.h"

// STL include files
#include <string_view>
#include <vector>

// Forward declarations
class DataSvc;
// DP add this fwd decl for thread safety
class TsDataSvc;
class DataObject;
class IDataProviderSvc;
class IOpaqueAddress;
class IDataStoreAgent;

namespace DataSvcHelpers {
  /**
   * @class RegistryEntry RegistryEntry.h GaudiKernel/RegistryEntry.h
   *
   * Definition of an entry in the transient data store.
   *
   * The RegistryEntry represents an entry of the transient data store.
   * The object holds the recipe how to retrieve objects from the persistent
   * world (member IOpaqueAddress) as well as the backward link to the
   * parent entry and the leaves.
   *
   * @author Markus Frank
   * @author Sebastien Ponce
   */
  class GAUDI_API RegistryEntry final : public IRegistry {
  private:
    /// Definition of datastore type
    typedef std::vector<IRegistry*> Store;

  public:
    friend class ::DataSvc;
    // DP add friend class: the thread safe version of the DataSvc
    friend class ::TsDataSvc;
    /// Iterator definition
    typedef Store::const_iterator Iterator;

  private:
    /// Reference counter
    unsigned long m_refCount = 0;
    /// Is the link soft or hard?
    bool m_isSoft = false;
    /// String containing full path of the object (volatile)
    std::string m_fullpath;
    /// Path name
    std::string m_path;
    /// Pointer to parent
    RegistryEntry* m_pParent = nullptr;
    /// Pointer to opaque address (load info)
    IOpaqueAddress* m_pAddress = nullptr;
    /// Pointer to object
    DataObject* m_pObject = nullptr;
    /// Pointer to hosting transient store
    IDataProviderSvc* m_pDataProviderSvc = nullptr;
    /// Store of leaves
    Store m_store;

  private:
    /** The following entries serve two aspects:
      1) They are faster for recursive calls, because they are non-virtual
      2) They can be re-used for the non-const entry points using a
         const_cast of the result.
    */
    /// Recursive helper to assemble the full path name of the entry
    void assemblePath( std::string& buffer ) const;
    /// Internal method to retrieve data directory
    IRegistry* i_find( const IRegistry* pDirectory ) const;
    /// Internal method to retrieve data directory
    RegistryEntry* i_find( std::string_view path ) const;
    /// Internal method to locate object entry
    RegistryEntry* i_find( const DataObject* pObject ) const;
    /// Internal method to create entries
    RegistryEntry* i_create( std::string name );
    /// Internal method to add entries
    long i_add( RegistryEntry* entry );
    /// Set new parent pointer
    void setParent( RegistryEntry* pParent );
    /// Set the transient data store
    void setDataSvc( IDataProviderSvc* s ) { m_pDataProviderSvc = s; }
    /// Pointer to parent registry entry
    RegistryEntry* parentEntry() { return m_pParent; }
    /// Find identified leaf in this registry node
    RegistryEntry* findLeaf( std::string_view path ) const { return i_find( path ); }
    /// Find identified leaf in this registry node
    RegistryEntry* findLeaf( const DataObject* key ) const { return i_find( key ); }
    /// Initialize link as hard link
    void makeHard( DataObject* pObject );
    /// Initialize link as hard link
    void makeHard( IOpaqueAddress* pAddress );
    /// Initialize link as soft link
    void makeSoft( DataObject* pObject );
    /// Initialize link as soft link
    void makeSoft( IOpaqueAddress* pAddress );

  public:
    /// Standard Constructor
    RegistryEntry( std::string path, RegistryEntry* parent = nullptr );
    /// Standard Destructor
    ~RegistryEntry() override;
    /// IInterface implementation: Reference the object
    unsigned long release() override;
    /// IInterface implementation: Dereference the object
    unsigned long addRef() override { return ++m_refCount; }
    /// Retrieve name of the entry
    const std::string& name() const override { return m_path; }
    /// Full identifier (or key)
    const std::string& identifier() const override { return m_fullpath; }
    /// Retrieve pointer to Transient Store
    IDataProviderSvc* dataSvc() const override { return m_pDataProviderSvc; }
    /// Retrive object behind the link
    DataObject* object() const override { return m_pObject; }
    /// Retrieve opaque storage address
    IOpaqueAddress* address() const override { return m_pAddress; }
    /// Pointer to parent directory entry
    IRegistry* parent() const { return m_pParent; }
    /// Is the link soft or hard
    bool isSoft() const { return m_isSoft; }
    /// Access the leaves of the object
    const Store& leaves() const { return m_store; }
    /// Return the size of the container(=number of objects)
    size_t size() const { return m_store.size(); }
    /// Simple check if the Container is empty
    bool isEmpty() const { return m_store.size() == 0; }
    /// Return starting point for container iteration
    Iterator begin() const { return m_store.begin(); }
    /// Return end elemtn if the container
    Iterator end() const { return m_store.end(); }
    /// Try to find an object identified by its pointer
    IRegistry* find( const IRegistry* obj ) const { return i_find( obj ); }
    /// Try to find an object identified by its relative name to the directory
    IRegistry* find( std::string_view path ) const { return i_find( path ); }
    /// Set/Update Opaque address
    void setAddress( IOpaqueAddress* pAddress ) override;
    /// Set/Update object address
    void setObject( DataObject* obj );

    /// Add entry to data store
    StatusCode add( std::string name, DataObject* pObject, bool is_soft = false );
    /// Add entry to data store
    StatusCode add( std::string name, IOpaqueAddress* pAddress, bool is_soft = false );
    /// Remove an entry from the store
    StatusCode remove( std::string_view name );
    /// Add object to the container
    long add( IRegistry* obj );
    /// Remove an object from the container
    long remove( IRegistry* obj );
    /// Delete all contained elements
    long deleteElements();
    /// traverse data tree
    StatusCode traverseTree( IDataStoreAgent* pAgent, int level = 0 );
  };
} // namespace DataSvcHelpers
#endif // GAUDIKERNEL_REGISTRYENTRY_H
