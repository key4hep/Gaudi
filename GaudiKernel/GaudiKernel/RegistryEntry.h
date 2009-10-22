// $Id: RegistryEntry.h,v 1.8 2005/07/18 16:34:05 hmd Exp $
#ifndef GAUDIKERNEL_REGISTRYENTRY_H
#define GAUDIKERNEL_REGISTRYENTRY_H

// STL include files
#include <vector>

// Framework include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IRegistry.h"

// Forward declarations
class DataSvc;
class DataObject;
class IDataProviderSvc;
class IOpaqueAddress;
class IDataStoreAgent;


namespace DataSvcHelpers   {
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
  class GAUDI_API RegistryEntry : public IRegistry  {
  private:
    /// Definition of datastore type
    typedef std::vector<IRegistry*> Store;
  public:
    friend class ::DataSvc;
    /// Iterator definition
    typedef Store::const_iterator Iterator;
  private:
    /// Reference counter
    unsigned long     m_refCount;
    /// Is the link soft or hard?
    bool              m_isSoft;
    /// String containing full path of the object (volatile)
    std::string       m_fullpath;
    /// Path name
    std::string       m_path;
    /// Pointer to parent
    RegistryEntry*    m_pParent;
    /// Pointer to opaque address (load info)
    IOpaqueAddress*   m_pAddress;
    /// Pointer to object
    DataObject*       m_pObject;
    /// Pointer to hosting transient store
    IDataProviderSvc* m_pDataProviderSvc;
    /// Store of leaves
    Store             m_store;

  private:
    /** The following entries serve two aspects:
      1) They are faster for recursive calls, because they are non-virtual
      2) They can be re-used for the non-const entry points using a
         const_cast of the result.
    */
    /// Recursive helper to assemble the full path name of the entry
    void  assemblePath(std::string& buffer)  const;
    /// Internal method to retrieve data directory
    IRegistry* i_find ( const IRegistry* pDirectory )  const;
    /// Internal method to retrieve data directory
    RegistryEntry* i_find ( const std::string& path )  const;
    /// Internal method to locate object entry
    RegistryEntry* i_find ( const DataObject* pObject )  const;
    /// Internal method to add entries
    RegistryEntry* i_add ( const std::string& name );
    /// Set new parent pointer
    void setParent(RegistryEntry* pParent);
    /// Set the transient data store
    void setDataSvc(IDataProviderSvc* s)    {
      m_pDataProviderSvc = s;
    }
    /// Pointer to parent registry entry
    virtual RegistryEntry* parentEntry()   {
      return m_pParent;
    }
    /// Find identified leaf in this registry node
    RegistryEntry* findLeaf(const std::string& path)  const    {
      return i_find(path);
    }
    /// Find identified leaf in this registry node
    RegistryEntry* findLeaf(const DataObject* key)  const  {
      return i_find(key);
    }
    /// Initialize link as hard link
    void makeHard (DataObject* pObject);
    /// Initialize link as hard link
    void makeHard (IOpaqueAddress* pAddress);
    /// Initialize link as soft link
    void makeSoft (DataObject* pObject);
    /// Initialize link as soft link
    void makeSoft (IOpaqueAddress* pAddress);
  public:
    /// Standard Constructor
    RegistryEntry(const std::string& path, RegistryEntry* parent = 0);
    /// Standard Destructor
    virtual ~RegistryEntry();
    /// IInterface implementation: Reference the object
    virtual unsigned long release();
    /// IInterface implementation: Dereference the object
    virtual unsigned long addRef()    {
      return ++m_refCount;
    }
    /// Retrieve name of the entry
    const std::string& name()  const  {
      return m_path;
    }
    /// Full identifier (or key)
    virtual const std::string& identifier()   const  {
      return m_fullpath;
    }
    /// Retrieve pointer to Transient Store
    virtual IDataProviderSvc* dataSvc()  const {
      return m_pDataProviderSvc;
    }
    /// Retrive object behind the link
    virtual DataObject*     object()  const {
      return m_pObject;
    }
    /// Retrieve opaque storage address
    virtual IOpaqueAddress* address()   const  {
      return m_pAddress;
    }
    /// Pointer to parent directory entry
    virtual IRegistry* parent()   const  {
      return m_pParent;
    }
    /// Is the link soft or hard
    virtual bool isSoft()   const     {
      return m_isSoft;
    }
    /// Access the leaves of the object
    const Store& leaves()  const   {
      return m_store;
    }
    /// Return the size of the container(=number of objects)
    virtual int             size()  const   {
      return m_store.size();
    }
    /// Simple check if the Container is empty
    virtual bool            isEmpty()  const    {
      return m_store.size() == 0;
    }
    /// Return starting point for container iteration
    virtual Iterator   begin   ()   const  {
      return m_store.begin();
    }
    /// Return end elemtn if the container
    virtual Iterator   end     ()   const  {
      return m_store.end();
    }
    /// Try to find an object identified by its pointer
    virtual IRegistry* find ( const IRegistry* obj )  const  {
      return i_find(obj);
    }
    /// Try to find an object identified by its relative name to the directory
    virtual IRegistry* find ( const std::string& path )  const  {
      return i_find(path);
    }
    /// Set/Update Opaque address
    void setAddress(IOpaqueAddress* pAddress);
    /// Set/Update object address
    void setObject(DataObject* obj);

    /// Add entry to data store
    virtual long add(const std::string& name,
                     DataObject* pObject,
                     bool is_soft = false);
    /// Add entry to data store
    virtual long add(const std::string& name,
                     IOpaqueAddress* pAddress,
                     bool is_soft = false);
    /// Remove an entry from the store
    virtual long remove(const std::string& name);
    /// Add object to the container
    virtual long add(IRegistry* obj);
    /// Remove an object from the container
    virtual long remove(IRegistry* obj);
    /// Delete all contained elements
    virtual long deleteElements();
    /// traverse data tree
    virtual long traverseTree(IDataStoreAgent* pAgent, int level = 0);
  };
}
#endif  // GAUDIKERNEL_REGISTRYENTRY_H
