//====================================================================
//	DataSvc.cpp
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: implementation of the Transient data service: DataSvc
//
//  Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/98| Initial version                              | M.Frank
// | 20/2/99 | Automatic data preloading introduced.        | M.Frank
// +---------+----------------------------------------------+---------
//
//====================================================================
#define  DATASVC_DATASVC_CPP

// Framework include files
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IConversionSvc.h"

#include "GaudiKernel/xtoa.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/GaudiException.h"

#include "GaudiKernel/RegistryEntry.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/DataIncident.h"
#include "GaudiKernel/IIncidentSvc.h"

// Include files
#include <cassert>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <sstream>

#include "boost/utility/string_ref.hpp"


namespace {
  /// Helper function to convert item numbers to path strings
  /// i.e. int -> "/" + int
  inline std::string itemToPath(int item) {
    return std::string{'/'} + std::to_string(item);
  }

  inline boost::string_ref::size_type find( boost::string_ref s, char c, size_t o ) {
    if (!s.empty()) s.remove_prefix(o);
    auto r = s.find(c);
    return r==boost::string_ref::npos ? r : ( r + o ) ;
  }
}

// If you absolutely need optimization: switch off dynamic_cast.
// This improves access to the data store roughly by 10 %
// for balanced trees.
//
// M.Frank
#define CAST_REGENTRY(x,y) dynamic_cast<x>(y)
//#define CAST_REGENTRY(x,y) (x)(y)
typedef DataSvcHelpers::RegistryEntry RegEntry;

#define ON_DEBUG if (UNLIKELY(outputLevel() <= MSG::DEBUG))
#define ON_VERBOSE if (UNLIKELY(outputLevel() <= MSG::VERBOSE))

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

/** IDataManagerSvc: Remove all data objects below the sub tree
 *  identified by its full path name.
 */
StatusCode DataSvc::clearSubTree(const std::string& sub_tree_path)    {
  DataObject* pObject = nullptr;
  StatusCode status = findObject(sub_tree_path, pObject);
  if ( status.isSuccess() )   {
    RegEntry* node_entry = CAST_REGENTRY(RegEntry*,pObject->registry());
    if ( node_entry )   {
      RegEntry* parent = node_entry->parentEntry();
      if ( parent )  {
        parent->remove(node_entry);
        return StatusCode::SUCCESS;
      }
      return INVALID_PARENT;
    }
    return INVALID_OBJECT;
  }
  return status;
}

/** IDataManagerSvc: Remove all data objects below the sub tree
 *  identified by the object.
 */
StatusCode DataSvc::clearSubTree(DataObject* pObject)    {
  if ( checkRoot() )  {
    RegEntry* entry = CAST_REGENTRY(RegEntry*,pObject->registry());
    if ( entry )   {
      RegEntry* parent = entry->parentEntry();
      if ( parent )  {
        parent->remove(entry);
        return SUCCESS;
      }
      return INVALID_PARENT;
    }
    return INVALID_OBJECT;
  }
  return INVALID_ROOT;
}

/// IDataManagerSvc: Remove all data objects in the data store.
StatusCode DataSvc::clearStore()   {
  if ( checkRoot() )    {
    m_root->release();
    m_root = nullptr;
    return SUCCESS;
  }
  return INVALID_ROOT;
}

/** IDataManagerSvc: Analyse by traversing all data objects below the
 * sub tree identified by its full path name.
 */
StatusCode DataSvc::traverseSubTree (const std::string& sub_tree_path,
                                     IDataStoreAgent* pAgent)  {
  DataObject* pO = nullptr;
  StatusCode status = findObject(sub_tree_path, pO);
  if ( status.isFailure() )   return status;
  return  traverseSubTree(pO, pAgent);
}

/// IDataManagerSvc: Analyse by traversing all data objects below the sub tree
StatusCode DataSvc::traverseSubTree ( DataObject* pObject,
                                      IDataStoreAgent* pAgent )  {
  if ( !checkRoot() )   return INVALID_ROOT;
  RegEntry* entry = CAST_REGENTRY(RegEntry*,pObject->registry());
  if ( !entry )         return INVALID_OBJECT;
  return entry->traverseTree(pAgent);
}

/// IDataManagerSvc: Analyse by traversing all data objects in the data store.
StatusCode DataSvc::traverseTree(IDataStoreAgent* pAgent)   {
  if ( !checkRoot() )    return INVALID_ROOT;
  return m_root->traverseTree(pAgent);
}

/**
 * Initialize data store for new event by giving new event path and root
 * object. Takes care to clear the store before reinitializing it
 */
StatusCode DataSvc::setRoot(const std::string& root_path,
                            DataObject* pRootObj)    {
  clearStore().ignore();
  return i_setRoot (root_path, pRootObj);
}

/**
 * Initialize data store for new event by giving new event path and root
 * object. Does not clear the store before reinitializing it. This could
 * lead to errors and should be handle with care. Use setRoot if unsure
 */
StatusCode DataSvc::i_setRoot(const std::string& root_path,
                              DataObject* pRootObj)    {
  if ( pRootObj )  {
    m_root = new RegEntry(root_path);
    m_root->makeHard(pRootObj);
    m_root->setDataSvc(this);
    preLoad().ignore();
  }
  return SUCCESS;
}

/**
 * Initialize data store for new event by giving new event path and address
 * of root object. Takes care to clear the store before reinitializing it
 */
StatusCode DataSvc::setRoot(const std::string& root_path,
                            IOpaqueAddress* pRootAddr)    {
  clearStore().ignore();
  return i_setRoot (root_path, pRootAddr);
}

/**
 * Initialize data store for new event by giving new event path and address
 * of root object. Does not clear the store before reinitializing it. This
 * could lead to errors and should be handle with care. Use setRoot if unsure
 */
StatusCode DataSvc::i_setRoot(const std::string& root_path,
                              IOpaqueAddress* pRootAddr)    {
  if ( pRootAddr )  {
    m_root = new RegEntry(root_path);
    m_root->makeHard(pRootAddr);
    m_root->setDataSvc(this);
    preLoad().ignore();
  }
  return SUCCESS;
}

/// IDataManagerSvc: Pass a default data loader to the service.
StatusCode DataSvc::setDataLoader(IConversionSvc* pDataLoader)    {
  if ( pDataLoader  ) pDataLoader->addRef();
  if ( m_dataLoader ) m_dataLoader->release();
  if ( pDataLoader  )    {
    pDataLoader->setDataProvider(this).ignore();
  }
  m_dataLoader = pDataLoader;
  return SUCCESS;
}

/// IDataManagerSvc: Explore the object store: retrieve the object's parent
StatusCode DataSvc::objectParent(const DataObject*  pObject,
                                 IRegistry*& refpParent)   {
  if ( !pObject )     return INVALID_OBJECT;
  return objectParent(pObject->registry(), refpParent);
}
/// IDataManagerSvc: Explore the object store: retrieve the object's parent
StatusCode DataSvc::objectParent(const IRegistry*   pRegistry,
                                 IRegistry*& refpParent)  {
  if ( !checkRoot() )    return INVALID_ROOT;
  const RegEntry* node_entry = CAST_REGENTRY(const RegEntry*,pRegistry);
  if ( !node_entry )     return INVALID_OBJECT;
  refpParent = node_entry->parent();
  return StatusCode::SUCCESS;
}

/// IDataManagerSvc: Explore an object identified by its pointer.
StatusCode DataSvc::objectLeaves( const DataObject*  pObject,
                                 std::vector<IRegistry*>& leaves)   {
  if ( !pObject )     return INVALID_OBJECT;
  return objectLeaves(pObject->registry(), leaves);
}

/** IDataManagerSvc: Explore an object identified by the pointer to the
 * registry entry.
 */
StatusCode DataSvc::objectLeaves( const IRegistry*   pRegistry,
                                 std::vector<IRegistry*>& leaves)   {
  if ( !checkRoot() )    return INVALID_ROOT;
  const RegEntry* node_entry = CAST_REGENTRY(const RegEntry*,pRegistry);
  if ( !node_entry )     return INVALID_OBJECT;
  leaves = node_entry->leaves();
  return StatusCode::SUCCESS;
}

///  IDataManagerSvc: Register object address with the data store.
StatusCode DataSvc::registerAddress(const std::string& fullPath,
                                    IOpaqueAddress* pAddress)   {
  if ( fullPath.empty() )     return INVALID_OBJ_PATH;
  return registerAddress( fullPath.front() != SEPARATOR ? m_root : nullptr,
                          fullPath, pAddress);
}

///  IDataManagerSvc: Register object address with the data store.
StatusCode DataSvc::registerAddress(DataObject* parentObj,
                                    const std::string& objectPath,
                                    IOpaqueAddress* pAddress)   {
  IRegistry* pRegistry = ( parentObj ?  parentObj->registry() : nullptr );
  return registerAddress(pRegistry, objectPath, pAddress);
}

///  IDataManagerSvc: Register object address with the data store.
StatusCode DataSvc::registerAddress(IRegistry* parentObj,
                                    const std::string& objPath,
                                    IOpaqueAddress* pAddress)   {
  if ( !checkRoot() )  return INVALID_ROOT;
  if ( objPath.empty() )  return INVALID_OBJ_PATH;

  if ( !parentObj )   {
    if ( objPath.front() != SEPARATOR )    {
      return registerAddress(m_root, objPath, pAddress);
    }
    std::string::size_type sep = objPath.find(SEPARATOR,1);
    if ( sep == std::string::npos )  {
      return INVALID_PARENT;
    }
    if ( objPath.compare(0,sep,m_rootName) == 0 )   {
      return registerAddress(m_root, objPath.substr(sep), pAddress);
    }
  }
  if ( objPath.front() != SEPARATOR )    {
    return registerAddress(parentObj, char(SEPARATOR)+objPath, pAddress);
  }
  RegEntry* par_entry = CAST_REGENTRY(RegEntry*,parentObj);
  if ( !par_entry ) return INVALID_PARENT;

  std::string::size_type sep = objPath.rfind(SEPARATOR);
  if ( sep > 0 && sep != std::string::npos )    {
    auto p_path = objPath.substr( 0, sep);
    auto o_path = objPath.substr( sep );
    RegEntry* p_entry = par_entry->findLeaf(p_path);
    // Create default object leafs if the
    // intermediate nodes are not present
    if ( 0 == p_entry && m_forceLeaves )    {
      DataObject *pLeaf = createDefaultObject();
      StatusCode sc = registerObject(par_entry->identifier(),
                                     p_path,
                                     pLeaf);
      if ( sc.isFailure() )  delete pLeaf;
      p_entry = par_entry->findLeaf(p_path);
    }
    if ( !p_entry )  return INVALID_PARENT;
    return registerAddress(p_entry, o_path, pAddress);
  }
  StatusCode status = par_entry->add(objPath, pAddress);
  return  status.isSuccess() ? status : DOUBL_OBJ_PATH;
}

///  IDataManagerSvc: Unregister object address from the data store.
StatusCode DataSvc::unregisterAddress(const std::string& fullPath)    {
  if ( fullPath.empty() )  return INVALID_OBJ_PATH;
  return unregisterAddress( fullPath.front() != SEPARATOR ? m_root  :  nullptr,
                            fullPath );
}

///  IDataManagerSvc: Unregister object address from the data store.
StatusCode DataSvc::unregisterAddress(DataObject* pParent,
                                      const std::string& objPath)   {
  return unregisterAddress( pParent ? pParent->registry() : nullptr,
                            objPath);
}

///  IDataManagerSvc: Unregister object address from the data store.
StatusCode DataSvc::unregisterAddress(IRegistry* pParent,
                                      const std::string& objPath)   {
  if ( !checkRoot() )  return INVALID_ROOT;
  if ( objPath.empty() )   return INVALID_OBJ_PATH;

  if ( !pParent )   {
    if ( objPath.front() != SEPARATOR )    {
      return unregisterAddress(m_root, objPath);
    }
    std::string::size_type sep = objPath.find(SEPARATOR,1);
    if ( sep != std::string::npos &&
         objPath.compare( 0, sep, m_rootName) == 0 )   {
        return unregisterAddress(m_root, objPath.substr(sep));
    }
    return INVALID_PARENT;
  }
  if ( objPath.front() != SEPARATOR )    {
    return unregisterAddress(pParent, char(SEPARATOR)+objPath);
  }
  RegEntry* node_entry = CAST_REGENTRY(RegEntry*,pParent);
  if ( node_entry )   {
    RegEntry* leaf_entry = node_entry->findLeaf(objPath);
    if ( leaf_entry )    {
      std::string::size_type sep = objPath.rfind(SEPARATOR);
      if ( sep > 0 && sep != std::string::npos )    {
        return unregisterAddress(leaf_entry->parent(), objPath.substr(sep));
      }
      StatusCode status = node_entry->remove(objPath);
      if ( status.isSuccess() )   return status;
    }
  }
  return INVALID_PARENT;
}

/// Register object with the data store.
StatusCode DataSvc::registerObject (const std::string& fullPath,
                                    DataObject* pObject)   {
  return registerObject(nullptr, fullPath, pObject);
}


/// Register object with the data store.
StatusCode DataSvc::registerObject (const std::string& parentPath,
                                    const std::string& objPath,
                                    DataObject* pObject)   {
  DataObject* pO = nullptr;
  StatusCode status = retrieveObject(parentPath, pO);
  if ( !status.isSuccess() && m_forceLeaves )   {
    pO = createDefaultObject();
    status = registerObject(parentPath, pO);
    if ( !status.isSuccess() )   pO->release();
  }
  if ( status.isFailure() )  return status;
  return registerObject(pO, objPath, pObject);
}

/// Register object with the data store.
StatusCode DataSvc::registerObject(const std::string& parentPath,
                                   int item,
                                   DataObject* pObject)   {
  return registerObject(parentPath, itemToPath(item), pObject);
}

/// Register object with the data store.
StatusCode DataSvc::registerObject(DataObject* parentObj,
                                   int item,
                                   DataObject* pObject)   {
  return registerObject(parentObj, itemToPath(item), pObject);
}

/// Register object with the data store.
StatusCode DataSvc::registerObject(DataObject* parentObj,
                                   const std::string& objPath,
                                   DataObject* pObject)   {
  if ( !checkRoot() )  return INVALID_ROOT;

  if ( !parentObj )   {
    if ( !objPath.empty() )   {
      if ( objPath.front() == SEPARATOR )    {
        std::string::size_type sep = objPath.find(SEPARATOR,1);
        if ( sep != std::string::npos )    {
          return registerObject(objPath.substr( 0, sep), objPath.substr( sep ), pObject);
        }
      }
      else  {
        return registerObject(m_rootName, objPath, pObject);
      }
    }
    return INVALID_OBJ_PATH;
  }
  RegEntry* node_entry = CAST_REGENTRY(RegEntry*,parentObj->registry());
  if ( node_entry )   {
    StatusCode status = INVALID_PARENT;
    std::string::size_type sep = objPath.find(SEPARATOR,1);
    if ( sep != std::string::npos )    {
      auto p_path = objPath.substr( 0, sep );
      auto o_path = objPath.substr( sep );
      RegEntry* par_entry = node_entry->findLeaf(p_path);
      // Create default object leafs if the
      // intermediate nodes are not present
      if ( 0 == par_entry && m_forceLeaves )    {
        DataObject *pLeaf = createDefaultObject();
        StatusCode sc = registerObject(parentObj, p_path, pLeaf);
        if ( ! sc.isSuccess() )   {
          delete pLeaf;
        }
        par_entry = node_entry->findLeaf(p_path);
      }
      else if ( 0 != par_entry && par_entry->object() == 0 )  {
        status = i_retrieveEntry( node_entry, p_path, par_entry);
        if ( !status.isSuccess() && !par_entry->address() && m_forceLeaves )  {
          DataObject *pLeaf = createDefaultObject();
          StatusCode sc = registerObject(parentObj, p_path, pLeaf);
          if ( ! sc.isSuccess() )   {
            delete pLeaf;
          }
          par_entry = node_entry->findLeaf(p_path);
        }
      }
      node_entry = par_entry;
      if ( node_entry )    {
        DataObject* obj = node_entry->object();
        if ( obj )   {
          status = registerObject( obj, o_path, pObject );
        }
      }
    }
    else   {
      RegEntry* leaf = node_entry->findLeaf(objPath);
      if ( !leaf )    {
        status = node_entry->add( objPath, pObject );
      }
      else    {
        DataObject* obj = leaf->object();
        if ( !obj )    {
          if ( !pObject ) {
            error() << "registerObject: trying to register null DataObject" << endmsg;
            return StatusCode::FAILURE;
          }
          else  {
            pObject->setRegistry(leaf);
          }
          leaf->setAddress(0);
          leaf->setObject(pObject);
          status = StatusCode::SUCCESS;
        }
        else  {
          status = DOUBL_OBJ_PATH;
        }
      }
    }
    return status;
  }
  return INVALID_PARENT;
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject(const std::string& fullPath)   {
  DataObject* pObject = nullptr;
  StatusCode status = findObject(fullPath, pObject);
  if ( status.isFailure() )    return status;
  RegEntry* pEntry = CAST_REGENTRY(RegEntry*,pObject->registry());
  if ( !pEntry )    return INVALID_ROOT;

  if ( !pEntry->isEmpty() )    return DIR_NOT_EMPTY;

  RegEntry* pParent = pEntry->parentEntry();
  if ( !pParent )   return INVALID_PARENT;

  if ( pObject )    pObject->addRef();
  pParent->remove(pEntry);
  return StatusCode::SUCCESS;
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject(const std::string& parentPath,
                                     const std::string& objPath)   {
  DataObject* pO = nullptr;
  StatusCode status = findObject(parentPath, pO);
  if ( status.isFailure() )   return status;
  return  unregisterObject(pO, objPath);
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject(const std::string& parentPath, int item) {
  return unregisterObject(parentPath, itemToPath(item));
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject(DataObject* pObject)    {
  if ( !checkRoot() )  return INVALID_ROOT;
  RegEntry* entry = m_root->findLeaf(pObject);
  if ( !entry )   return INVALID_OBJECT;
  RegEntry* parent = entry->parentEntry();
  if ( !parent )     return INVALID_PARENT;
  if ( !entry->isEmpty() )   return DIR_NOT_EMPTY;
  if ( entry->object() ) entry->object()->addRef();
  if ( parent )  parent->remove(entry);
  return SUCCESS;
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject (DataObject* pParentObj,
                                      const std::string& objectPath) {
  if ( checkRoot() )  {
    try   {
      RegEntry* parent = CAST_REGENTRY(RegEntry*,pParentObj->registry());
      if ( parent )   {
        RegEntry* entry = parent->findLeaf(objectPath);
        if ( entry )   {
          if ( entry->isEmpty() )    {
            if ( entry->object() )    {
              entry->object()->addRef();
            }
            parent->remove(entry);
            return SUCCESS;
          }
          return DIR_NOT_EMPTY;
        }
        return INVALID_OBJECT;
      }
    }
    catch(...) {
    }
    return INVALID_PARENT;
  }
  return INVALID_ROOT;
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject(DataObject* pParentObj, int item)    {
  return unregisterObject(pParentObj, itemToPath(item));
}

/** Invoke data fault handling if enabled
  */
DataObject* DataSvc::handleDataFault(IRegistry* pReg, const std::string& path) {
    return i_handleDataFault(pReg,path);
}

DataObject* DataSvc::i_handleDataFault(IRegistry* pReg, boost::string_ref path)
{
  if ( m_enableFaultHdlr )  {
    IRegistry* pLeaf = nullptr;
    if ( pReg && path.empty() )   {
      DataIncident incident(name(), m_faultName, pReg->identifier());
      m_incidentSvc->fireIncident(incident);
      return pReg->object();
    }
    else if ( pReg )  {
      std::string p = pReg->identifier();
      if (path.front() != SEPARATOR ) p += SEPARATOR;
      p.append(path.data(),path.size());
      DataIncident incident(name(), m_faultName, p);
      m_incidentSvc->fireIncident(incident);
      pLeaf = m_root->findLeaf(p);
    }
    else  {
      std::string p = m_root->identifier();
      if (path.front() != SEPARATOR ) p += SEPARATOR;
      p.append(path.data(),path.size());
      DataIncident incident(name(), m_faultName, p);
      m_incidentSvc->fireIncident(incident);
      pLeaf = m_root->findLeaf(p);
    }
    if ( pLeaf )  {
      return pLeaf->object();
    }
  }
  return nullptr;
}

/** Invoke Persistency service to create transient object from its
 *  persistent representation
 */
StatusCode DataSvc::loadObject(IRegistry* pRegistry)   {
  IConversionSvc* pLoader = getDataLoader(pRegistry);
  return loadObject(pLoader, pRegistry);
}

/** Invoke Persistency service to create transient object from its
 *  persistent representation
 */
StatusCode DataSvc::loadObject(IConversionSvc* pLoader, IRegistry* pRegistry) {
  StatusCode status = INVALID_OBJ_ADDR;
  DataObject* pObject = nullptr;
  if ( !pLoader )   {         // Precondition: Data loader must be present
    if (handleDataFault(pRegistry)) return  SUCCESS;
    else                            return  NO_DATA_LOADER;
  }
  if ( !pRegistry )    {      // Precondition: Directory must be valid
    if (handleDataFault(pRegistry)) return  SUCCESS;
    else                            return  INVALID_OBJ_ADDR;
  }

  VERMSG << "Requested object " << pRegistry->identifier() << endmsg;

  if ( m_enableAccessHdlr )  {
    // Fire data access incident
    DataIncident incident(name(), m_accessName, pRegistry->identifier());
    m_incidentSvc->fireIncident(incident);
  }
  if ( !m_inhibitPathes.empty() )   {
    auto inhibit = std::find(m_inhibitPathes.begin(), m_inhibitPathes.end(),
                             pRegistry->identifier());
    if ( inhibit != m_inhibitPathes.end() )   {
      return NO_ACCESS;
    }
  }
  IOpaqueAddress* pAddress = pRegistry->address();
  if ( !pAddress )  {         // Precondition:
    return INVALID_OBJ_ADDR;      // Address must be valid
  }
  try   {
    status = pLoader->createObj(pAddress, pObject);  // Call data loader
    if ( status.isSuccess() )    {

      VERMSG << "Object " << pRegistry->identifier() << " created" << endmsg;

      RegEntry *pEntry = CAST_REGENTRY(RegEntry*,pRegistry);
      pEntry->setObject(pObject);

      VERMSG << "Filling object " << pRegistry->identifier() << endmsg;
      status = pLoader->fillObjRefs(pAddress, pObject);
    }
  }
  catch( const GaudiException& exc ) {
    if ( handleDataFault(pRegistry) )  {
      return SUCCESS;
    }
    throw GaudiException("GaudiException in loadObject() " + pRegistry->identifier(),
        name(), StatusCode::FAILURE, exc);
  }
  catch( const std::exception& x) {
    if ( handleDataFault(pRegistry) )  {
      return SUCCESS;
    }
    throw GaudiException("std::exception in loadObject() " + pRegistry->identifier() +
        ": " + System::typeinfoName(typeid(x)) + ", " + x.what(),
      name(), StatusCode::FAILURE);
  }
  catch(...) {
    if ( handleDataFault(pRegistry) )  {
      return SUCCESS;
    }
    throw GaudiException("UNKN exception in loadObject() " + pRegistry->identifier(),
      name(), StatusCode::FAILURE);
  }
  if ( !status.isSuccess() )  {
    if ( handleDataFault(pRegistry) )  {
      return StatusCode::SUCCESS;
    }
  }
  ON_VERBOSE if ( status.isSuccess() ) {
    verbose() << "Object " << pRegistry->identifier() << " successfully loaded" << endmsg;
  }
  return status;
}

/// Retrieve registry entry from store
StatusCode DataSvc::retrieveEntry(RegEntry* parentObj,
                                  const std::string& path,
                                  RegEntry*& pEntry)  {
    return i_retrieveEntry(parentObj,path,pEntry);
}

StatusCode DataSvc::i_retrieveEntry(RegEntry* parentObj,
                                  boost::string_ref path,
                                  RegEntry*& pEntry)  {
  // A.Valassi 16.08.2001 avoid core dump if store is empty
  if ( !checkRoot() )  return StatusCode(INVALID_ROOT,true);

  static const auto empty = boost::string_ref{};
  auto sep    = find(path,SEPARATOR,1);
  pEntry = nullptr;

  if ( !parentObj )    {
    if ( path.empty() || path == m_rootName )   {
      parentObj = m_root;
      path = empty;
    } else if ( path.front() != SEPARATOR )  {
      parentObj = m_root;
    } else if ( sep != boost::string_ref::npos )    {
      if ( !m_root->object() )  {
        RegEntry* r = nullptr;
        auto status = i_retrieveEntry(m_root, empty, r);
        if ( !status.isSuccess() )  return status;
      }
      parentObj = m_root;
      path = path.substr(sep);
    } else {
      return INVALID_OBJ_PATH;
    }
    sep = find(path,SEPARATOR,1);
  }

  StatusCode status = StatusCode(INVALID_ROOT,true);
  if ( sep != boost::string_ref::npos )  { // the string contains a separator (after pos 0)
    if (!parentObj->object()) { // if the parent object has not been loaded yet, load it now
      status = loadObject(parentObj);
      if ( !status.isSuccess() )  return status;
    }
    auto p_path = path.substr(0,sep);
    RegEntry* root_entry = parentObj->findLeaf(p_path);
    if ( !root_entry && m_enableFaultHdlr )    {
      // If not even the parent is there, an incident
      // to load the parent must be fired...
      i_handleDataFault(parentObj, p_path);
      root_entry = parentObj->findLeaf(p_path);
    }
    if ( root_entry )    {
      DataObject* pO = root_entry->object();
      if ( !pO )  {
        // Object is not loaded: load the object if at all possible
        status = loadObject(root_entry);
        if ( !status.isSuccess() )  return status;
      }
      if ( root_entry->isSoft() ) {
        root_entry = CAST_REGENTRY(RegEntry*,pO->registry());
      }
      return i_retrieveEntry(root_entry, path.substr(sep), pEntry);
    }
    return status;
  } else if ( path.empty() )    {
    pEntry = parentObj;
  } else {
    if (!parentObj->object()) { // if the parent object has not been loaded yet, load it now
      status = loadObject(parentObj);
      if ( !status.isSuccess() )  return status;
    }
    // last leave in search: find leaf and load
    pEntry = parentObj->findLeaf(path);
    // If no registry entry was found, trigger incident for action-on-demand
    if ( !pEntry && m_enableFaultHdlr )  {
      i_handleDataFault(parentObj, path);
      pEntry = (path.empty() ? parentObj : parentObj->findLeaf(path) );
    }
  }

  // Check results and return
  if ( !pEntry )            return INVALID_OBJ_PATH;
  if ( !pEntry->object() )  return loadObject(pEntry);

  if ( m_enableAccessHdlr )  {
    // Fire data access incident
    // I do not know if this is a good idea....
    // This fires too often!
    //
    //DataIncident incident(name(), m_accessName, pEntry->identifier());
    //m_incidentSvc->fireIncident(incident);
    return SUCCESS;
  }
  return SUCCESS;
}

/// Retrieve object identified by its directory from the data store.
StatusCode DataSvc::retrieveObject(IRegistry* pRegistry,
                                   const std::string& path,
                                   DataObject*& pObject)   {
  pObject = nullptr;
  RegEntry *result = nullptr, *parent = CAST_REGENTRY(RegEntry*,pRegistry);
  StatusCode status = i_retrieveEntry(parent, path, result);
  if ( status.isSuccess() )   {
    pObject = result->object();
  }
  return status;
}

/// Retrieve object identified by its full path from the data store.
StatusCode DataSvc::retrieveObject(const std::string& fullPath,
                                   DataObject*& pObject)   {
  IRegistry* nullDir = nullptr;
  return retrieveObject(nullDir, fullPath, pObject);
}

/// Retrieve object identified by its full path from the data store.
StatusCode DataSvc::retrieveObject(const std::string& parentPath,
                                   const std::string& objectPath,
                                   DataObject*& pObject)   {
  DataObject* parent = nullptr;
  StatusCode status = retrieveObject(parentPath, parent);
  if ( status.isSuccess() )   {
    status = retrieveObject (parent, objectPath, pObject);
  }
  return status;
}

/// Retrieve object identified by its full path from the data store.
StatusCode DataSvc::retrieveObject(const std::string& parentPath,
                                   int item,
                                   DataObject*& pObject)   {
  return retrieveObject(parentPath, itemToPath(item), pObject);
}

/// Retrieve object from data store.
StatusCode DataSvc::retrieveObject(DataObject* parentObj,
                                   const std::string& path,
                                   DataObject*& pObject)  {
  IRegistry* pRegistry = (parentObj ?  parentObj->registry() : nullptr);
  return retrieveObject(pRegistry, path, pObject);
}

/// Retrieve registry entry from store
StatusCode DataSvc::retrieveObject(DataObject* parentObj,
                                   int item,
                                   DataObject*& pObject)  {
  return retrieveObject(parentObj, itemToPath(item), pObject);
}

/// Retrieve object identified by its directory from the data store.
StatusCode DataSvc::findObject(IRegistry* pRegistry,
                               const std::string& path,
                               DataObject*& pObject)   {
  pObject = nullptr;
  IRegistry* pReg = ( pRegistry ? pRegistry : m_root );
  RegEntry* root_entry = CAST_REGENTRY(RegEntry*, pReg);
  if ( root_entry )    {
    if ( !path.empty() )  {
      pReg = root_entry->find(path);
    }
    if ( !pReg )    {
      return INVALID_OBJ_PATH;
    }
    pObject = pReg->object();
  }
  return (!pObject) ? OBJ_NOT_LOADED : IDataProviderSvc_NO_ERROR;
}

/// Find object identified by its full path in the data store.
StatusCode DataSvc::findObject(const std::string& path,
                               DataObject*& pObject)   {
  pObject = nullptr;
  if ( checkRoot() )  {
    if ( path.empty() || path == m_rootName )   {
      pObject = m_root->object();
      return (0 == pObject) ? OBJ_NOT_LOADED : IDataProviderSvc_NO_ERROR;
    }
    else if ( path.front() != SEPARATOR )    {
      return findObject(m_rootName, path, pObject);
    }
    return findObject((IRegistry*)nullptr, path, pObject);
  }
  return INVALID_ROOT;
}

/// Retrieve object identified by its full path from the data store.
StatusCode DataSvc::findObject(const std::string& parentPath,
                               const std::string& objectPath,
                               DataObject*& pObject)   {
  DataObject* parent = nullptr;
  StatusCode status = findObject(parentPath, parent);
  if ( status.isSuccess() )   {
    status = findObject (parent, objectPath, pObject);
  }
  return status;
}

/// Retrieve object identified by its full path from the data store.
StatusCode DataSvc::findObject(const std::string& parentPath,
                               int item, DataObject*& pObject)   {
  return findObject(parentPath, itemToPath(item), pObject);
}

/// Find object identified by its full path in the data store.
StatusCode DataSvc::findObject(DataObject* parentObj,
                               int item,
                               DataObject*& pObject)    {
  return findObject(parentObj, itemToPath(item), pObject);
}

/// Find object identified by its full path in the data store.
StatusCode DataSvc::findObject(DataObject* parentObj,
                               const std::string& path,
                               DataObject*& pObject)    {
  IRegistry* pDir = ( parentObj ? parentObj->registry() : nullptr );
  return findObject(pDir, path, pObject);
}

/// Update object.
StatusCode DataSvc::updateObject(const std::string& updatePath)   {
  DataObject* pO = nullptr;
  StatusCode status = findObject(updatePath, pO);
  if ( status.isSuccess() )   {
    return updateObject(pO);
  }
  return retrieveObject(updatePath, pO);
}

/// Update object.
StatusCode DataSvc::updateObject(IRegistry* pRegistry)    {
  if ( !pRegistry )  {                     // Precondition:
    return INVALID_OBJ_ADDR;               // Addres must be valid
  }
  DataObject* toUpdate = pRegistry->object();
  if ( !toUpdate )  {                      // Try first to load
    return loadObject(pRegistry);
  }
  return updateObject(toUpdate);
}

/// Update object.
StatusCode DataSvc::updateObject(DataObject* toUpdate)    {
  StatusCode status = INVALID_OBJ_ADDR;
  if ( !toUpdate )  {                          // Precondition:
    return INVALID_OBJECT;                     // Address must be valid
  }
  IRegistry* pRegistry = toUpdate->registry(); // Precondition:
  if ( !pRegistry )    {                       // Need valid registry
    return INVALID_OBJECT;
  }
  IOpaqueAddress* pAddress = pRegistry->address(); // Precondition:
  if ( !pAddress )    {                            // Need valid address
    return INVALID_OBJ_ADDR;
  }
  IConversionSvc* pLoader = getDataLoader(pRegistry);
  if ( !pLoader )   {                          // Precondition:
    return NO_DATA_LOADER;                     // Data loader must be present
  }
  if ( !m_inhibitPathes.empty() )   {
    auto inhibit = std::find( m_inhibitPathes.begin(), m_inhibitPathes.end(),
                              pRegistry->identifier());
    if ( inhibit != m_inhibitPathes.end() )   {
      return NO_ACCESS;
    }
  }
  try   {
    status = pLoader->updateObj(pAddress, toUpdate);  // Call data loader
    if ( status.isSuccess() )   {
      status = pLoader->updateObjRefs(pAddress, toUpdate);
    }
  }
  catch( const GaudiException& exc ) {
    throw GaudiException("GaudiException in updateObject() " +
                         pRegistry->name(),
                         name(),
                         StatusCode::FAILURE, exc);
  }
  catch( const std::exception& x) {
    throw GaudiException("std::exception in updateObject() " +
                         pRegistry->name() + ": " +
                         System::typeinfoName(typeid(x)) + ", " +
                         x.what(),
                         name(), StatusCode::FAILURE);
  }
  catch(...) {
    throw GaudiException("UNKN exception in updateObject() " +
                         pRegistry->name(),
      name(), StatusCode::FAILURE);
  }
  return status;
}

/// Update object.
StatusCode DataSvc::updateObject(const std::string& parentPath,
                                 const std::string& updatePath)    {
  DataObject* pParent = nullptr;
  StatusCode status = findObject(parentPath, pParent);
  if ( status.isSuccess() )   {
    status = updateObject( pParent, updatePath);
  }
  return status;
}

/// Update object.
StatusCode DataSvc::updateObject(DataObject* parent,
                                 const std::string& updatePath)   {
  DataObject* pObject = nullptr;
  StatusCode status = findObject(parent, updatePath, pObject);
  if ( status.isSuccess() )   {
    status = updateObject(pObject);
  }
  return status;
}

// Link object
StatusCode DataSvc::linkObject(IRegistry* from,
                               const std::string& objPath, DataObject* to) {
  if ( checkRoot() )  {
    try   {
      RegEntry* from_entry = CAST_REGENTRY(RegEntry*,from);
      if ( from_entry )    {
        // First check if both objects are already registered to the store
        RegEntry* to_entry   = m_root->findLeaf(to);
        if ( !to_entry )   {
          return INVALID_OBJECT;
        }
        else  {
          std::string::size_type sep = objPath.rfind(SEPARATOR);
          if ( sep > 0 && sep != std::string::npos )    {   // in case the objPath is a sub-directory itself
            DataObject* pO = nullptr;
            StatusCode sc = retrieveObject(from, objPath.substr(0,sep), pO);
            if ( sc.isSuccess() )    {
              sc = linkObject(pO->registry(), objPath.substr(sep), to);
            }
            return sc;
          }
          // Now register the soft link
          StatusCode status = from_entry->add( objPath, to, true);
          return status.isSuccess() ?
            IDataProviderSvc_NO_ERROR : DOUBL_OBJ_PATH;
        }
      }
    }
    catch (...) {
    }
    return INVALID_PARENT;
  }
  return INVALID_ROOT;
}

/// Add a link to another object.
StatusCode DataSvc::linkObject(const std::string& fullPath,
                               DataObject* to)    {
  if ( !fullPath.empty() )    {
    if ( fullPath.front() != SEPARATOR )   {
      return linkObject(m_rootName, fullPath, to);
    }
    auto sep = fullPath.rfind(SEPARATOR);
    return linkObject( fullPath.substr( 0, sep), fullPath.substr(sep), to);
  }
  return INVALID_OBJ_PATH;
}

/// Add a link to another object.
StatusCode DataSvc::linkObject(const std::string& from,
                               const std::string& objPath,
                               DataObject* to)    {
  DataObject* pO = nullptr;
  StatusCode status = retrieveObject(from, pO);
  if ( status.isSuccess() )   {
    return linkObject(pO->registry(), objPath, to);
  }
  return status;
}

/// Add a link to another object.
StatusCode DataSvc::linkObject(DataObject* from,
                               const std::string& objPath,
                               DataObject* to)    {
  if ( from )    {
    IRegistry* from_entry = from->registry();
    if ( from_entry )   {
      return linkObject( from_entry, objPath, to);
    }
  }
  return INVALID_PARENT;
}

/// Remove a link to another object.
StatusCode DataSvc::unlinkObject(IRegistry* from,
                                 const std::string& objPath)    {
  if ( checkRoot() )  {
    try   {
      RegEntry* from_entry = CAST_REGENTRY(RegEntry*,from);
      if ( from_entry )    {
        std::string::size_type sep = objPath.rfind(SEPARATOR);
        if ( sep > 0 && sep != std::string::npos )    {  // in case the objPath is a sub-directory itself
          DataObject* pO = nullptr;
          StatusCode sc = findObject(from, objPath.substr(0,sep), pO);
          if ( sc.isSuccess() )    {
            sc = unlinkObject(pO->registry(), objPath.substr(sep));
          }
          return sc;
        }
        StatusCode status = from_entry->remove( objPath );
        if ( status.isSuccess() )     {
          return status;
        }
        return INVALID_OBJ_PATH;
      }
    }
    catch (...)   {
    }
    return INVALID_PARENT;
  }
  return INVALID_ROOT;
}

/// Remove a link to another object.
StatusCode DataSvc::unlinkObject(const std::string& fullPath)    {
  if ( !fullPath.empty() )    {
    if ( fullPath.front() != SEPARATOR )   {
      return unlinkObject(m_rootName, fullPath);
    }
    auto sep = fullPath.rfind(SEPARATOR);
    return unlinkObject(fullPath.substr(0,sep), fullPath.substr(sep));
  }
  return INVALID_OBJ_PATH;
}

/// Remove a link to another object.
StatusCode DataSvc::unlinkObject(const std::string& from,
                                 const std::string& objPath)    {
  DataObject* pObject = nullptr;
  StatusCode status = findObject(from, pObject);
  if ( status.isSuccess() )   {
    status = unlinkObject(pObject->registry(), objPath);
  }
  return status;
}

/// Remove a link to another object.
StatusCode DataSvc::unlinkObject(DataObject* from,
                                 const std::string& objPath)    {
  if ( checkRoot() )  {
    IRegistry* from_entry = m_root->findLeaf(from);
    return unlinkObject(from_entry, objPath);
  }
  return INVALID_ROOT;
}

/// Add an item to the preload list
StatusCode DataSvc::addPreLoadItem(const DataStoreItem& item)    {
  auto i = std::find(std::begin(m_preLoads), std::end(m_preLoads), item);
  if ( i == std::end(m_preLoads) )    {
    m_preLoads.push_back(item);
  }
  return StatusCode::SUCCESS;
}

/// Add an item to the preload list
StatusCode DataSvc::addPreLoadItem(const std::string& itemPath)    {
  return addPreLoadItem( DataStoreItem(itemPath,1) );
}

/// Remove an item from the preload list
StatusCode DataSvc::removePreLoadItem(const DataStoreItem& item)   {
  m_preLoads.erase(std::remove(std::begin(m_preLoads), std::end(m_preLoads), item),
                   std::end(m_preLoads));
  return StatusCode::SUCCESS;
}

/// Add an item to the preload list
StatusCode DataSvc::removePreLoadItem(const std::string& itemPath)    {
  return removePreLoadItem( DataStoreItem(itemPath,1) );
}

/// Clear the preload list
StatusCode DataSvc::resetPreLoad()    {
  m_preLoads.erase(std::begin(m_preLoads), std::end(m_preLoads));
  return StatusCode::SUCCESS;
}

/// Preload one level deep, then recursively call the next level
StatusCode DataSvc::preLoad(int depth, int load_depth, DataObject* pObject)    {
  //unused:  StatusCode sc = StatusCode::FAILURE;
  if ( pObject && depth++ < load_depth )    {
    RegEntry* dir = CAST_REGENTRY(RegEntry*,pObject->registry());
    if ( dir )   {
      for (const auto& i : *dir) {
        DataObject *pObj = nullptr;
        StatusCode status = retrieveObject(pObject, i->name(), pObj);
        if ( status.isSuccess() && depth < load_depth )   {
          preLoad(depth, load_depth, pObj).ignore();
        }
      }
    }
  }
  return StatusCode::SUCCESS;
}

/// load all preload items of the list
StatusCode DataSvc::preLoad()   {
  DataObject* pObj = nullptr;
  for (const auto& i : m_preLoads ) {
    StatusCode sc = retrieveObject( i.path(), pObj);
    int load_depth = i.depth();
    if ( sc.isSuccess() && load_depth > 1 )   {
      preLoad(1, load_depth, pObj).ignore();
    }
  }
  return StatusCode::SUCCESS;
}

/// Service initialization
StatusCode DataSvc::initialize()    {
  // Nothing to do: just call base class initialisation
  StatusCode sc = Service::initialize();
  if ( !sc.isSuccess() )  {
    return sc;
  }
  sc = service("IncidentSvc", m_incidentSvc, true);
  if ( UNLIKELY(!sc.isSuccess()) )  {
    error() << "Failed to access incident service." << endmsg;
  }
  return sc;
}

/// Service reinitialization
StatusCode DataSvc::reinitialize()    {
  // the finalize part is copied here
  setDataLoader(nullptr).ignore();
  resetPreLoad().ignore();
  clearStore().ignore();
  if ( m_incidentSvc )  {
    m_incidentSvc->release();
    m_incidentSvc = nullptr;
  }
  // re-initialize the base class
  StatusCode sc = Service::reinitialize();
  if ( UNLIKELY(!sc.isSuccess()) ) {
    error() << "Unable to reinitialize base class" << endmsg;
    return sc;
  }
  // the initialize part is copied here
  sc = service("IncidentSvc", m_incidentSvc, true);
  if ( UNLIKELY(!sc.isSuccess()) )  {
    error() << "Failed to access incident service." << endmsg;
    return sc;
  }
  // return
  return StatusCode::SUCCESS;
}

/// Service initialisation
StatusCode DataSvc::finalize()    {
  // Nothing to do: just call base class initialisation
  setDataLoader(nullptr).ignore();
  resetPreLoad().ignore();
  clearStore().ignore();
  if ( m_incidentSvc )  {
    m_incidentSvc->release();
    m_incidentSvc = nullptr;
  }
  return Service::finalize();
}

/// CLID for root Event
CLID DataSvc::rootCLID() const {
  return( (CLID)m_rootCLID );
}

/// Name for root Event
std::string DataSvc::rootName() const {
  return( m_rootName );
}

/// Create default objects in case forced creation of leaves is requested
DataObject* DataSvc::createDefaultObject()   const    {
  return new DataObject();
}

/** Retrieve customizable data loader according to registry entry to
 *  be retrieved
 */
IConversionSvc* DataSvc::getDataLoader(IRegistry* /* pReg */)   {
  return m_dataLoader;
}

/// Standard Constructor
DataSvc::DataSvc(const std::string& name,ISvcLocator* svc)
: base_class(name,svc), m_rootCLID( /*CLID_Event*/ 110),
  m_rootName( "/Event"), m_root(nullptr)
{
  m_dataLoader = nullptr;
  m_inhibitMap = 0;
  m_incidentSvc = nullptr;
  m_forceLeaves = false;
  m_enableFaultHdlr = false;
  m_enableAccessHdlr = false;
  m_faultName  = "DataFault";
  m_accessName = "DataAccess";
  declareProperty("RootCLID",            m_rootCLID);
  declareProperty("RootName",            m_rootName);
  declareProperty("ForceLeaves",         m_forceLeaves);
  declareProperty("InhibitPathes",       m_inhibitPathes);
  declareProperty("DataFaultName",       m_faultName);
  declareProperty("DataAccessName",      m_accessName);
  declareProperty("EnableFaultHandler",  m_enableFaultHdlr);
  declareProperty("EnableAccessHandler", m_enableAccessHdlr);
}

/// Standard Destructor
DataSvc::~DataSvc()  {
  setDataLoader(nullptr).ignore();
  resetPreLoad().ignore();
  clearStore().ignore();
}
