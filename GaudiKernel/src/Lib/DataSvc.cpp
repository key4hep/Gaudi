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

namespace {
  /// Helper function to convert item numbers to path strings
  /// i.e. int -> "/" + int
  inline std::string itemToPath(int item) {
    std::ostringstream path;
    path << '/' << item;
    return path.str();
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
  DataObject* pObject = 0;
  StatusCode status = findObject(sub_tree_path, pObject);
  if ( status.isSuccess() )   {
    RegEntry* node_entry = CAST_REGENTRY(RegEntry*,pObject->registry());
    if ( 0 != node_entry )   {
      RegEntry* parent = node_entry->parentEntry();
      if ( 0 != parent )  {
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
    if ( 0 != entry )   {
      RegEntry* parent = entry->parentEntry();
      if ( 0 != parent )  {
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
    m_root = 0;
    return SUCCESS;
  }
  return INVALID_ROOT;
}

/** IDataManagerSvc: Analyse by traversing all data objects below the
 * sub tree identified by its full path name.
 */
StatusCode DataSvc::traverseSubTree (const std::string& sub_tree_path,
                                     IDataStoreAgent* pAgent)  {
  DataObject* pO = 0;
  StatusCode status = findObject(sub_tree_path, pO);
  if ( status.isSuccess() )   {
    status = traverseSubTree(pO, pAgent);
  }
  return status;
}

/// IDataManagerSvc: Analyse by traversing all data objects below the sub tree
StatusCode DataSvc::traverseSubTree ( DataObject* pObject,
                                      IDataStoreAgent* pAgent )  {
  if ( checkRoot() )    {
    RegEntry* entry = CAST_REGENTRY(RegEntry*,pObject->registry());
    if ( 0 != entry )   {
      return entry->traverseTree(pAgent);
    }
    return INVALID_OBJECT;
  }
  return INVALID_ROOT;
}

/// IDataManagerSvc: Analyse by traversing all data objects in the data store.
StatusCode DataSvc::traverseTree(IDataStoreAgent* pAgent)   {
  if ( checkRoot() )    {
    return m_root->traverseTree(pAgent);
  }
  return INVALID_ROOT;
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
  if ( 0 != pRootObj )  {
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
  if ( 0 != pRootAddr )  {
    m_root = new RegEntry(root_path);
    m_root->makeHard(pRootAddr);
    m_root->setDataSvc(this);
    preLoad().ignore();
  }
  return SUCCESS;
}

/// IDataManagerSvc: Pass a default data loader to the service.
StatusCode DataSvc::setDataLoader(IConversionSvc* pDataLoader)    {
  if ( 0 != pDataLoader  ) pDataLoader->addRef();
  if ( 0 != m_dataLoader ) m_dataLoader->release();
  if ( 0 != pDataLoader  )    {
    pDataLoader->setDataProvider(this).ignore();
  }
  m_dataLoader = pDataLoader;
  return SUCCESS;
}

/// IDataManagerSvc: Explore the object store: retrieve the object's parent
StatusCode DataSvc::objectParent(const DataObject*  pObject,
                                 IRegistry*& refpParent)   {
  if ( pObject )    {
    return objectParent(pObject->registry(), refpParent);
  }
  return INVALID_OBJECT;
}
/// IDataManagerSvc: Explore the object store: retrieve the object's parent
StatusCode DataSvc::objectParent(const IRegistry*   pRegistry,
                                 IRegistry*& refpParent)  {
  if ( checkRoot() )    {
    const RegEntry* node_entry = CAST_REGENTRY(const RegEntry*,pRegistry);
    if ( node_entry )   {
      refpParent = node_entry->parent();
      return StatusCode::SUCCESS;
    }
    return INVALID_OBJECT;
  }
  return INVALID_ROOT;
}

/// IDataManagerSvc: Explore an object identified by its pointer.
StatusCode DataSvc::objectLeaves( const DataObject*  pObject,
                                 std::vector<IRegistry*>& leaves)   {
  if ( pObject )    {
    return objectLeaves(pObject->registry(), leaves);
  }
  return INVALID_OBJECT;
}

/** IDataManagerSvc: Explore an object identified by the pointer to the
 * registry entry.
 */
StatusCode DataSvc::objectLeaves( const IRegistry*   pRegistry,
                                 std::vector<IRegistry*>& leaves)   {
  if ( checkRoot() )    {
    const RegEntry* node_entry = CAST_REGENTRY(const RegEntry*,pRegistry);
    if ( node_entry )   {
      leaves = node_entry->leaves();
      return StatusCode::SUCCESS;
    }
    return INVALID_OBJECT;
  }
  return INVALID_ROOT;
}

///  IDataManagerSvc: Register object address with the data store.
StatusCode DataSvc::registerAddress(const std::string& fullPath,
                                    IOpaqueAddress* pAddress)   {
  if ( fullPath.length() > 0 )    {
    if ( fullPath[0] != SEPARATOR )   {
      return registerAddress(m_root, fullPath, pAddress);
    }
    IRegistry* pRegistry = 0;
    return registerAddress(pRegistry, fullPath, pAddress);
  }
  return INVALID_OBJ_PATH;
}

///  IDataManagerSvc: Register object address with the data store.
StatusCode DataSvc::registerAddress(DataObject* parentObj,
                                    const std::string& objectPath,
                                    IOpaqueAddress* pAddress)   {
  IRegistry* pRegistry = (0 == parentObj) ? 0 : parentObj->registry();
  return registerAddress(pRegistry, objectPath, pAddress);
}

///  IDataManagerSvc: Register object address with the data store.
StatusCode DataSvc::registerAddress(IRegistry* parentObj,
                                    const std::string& objPath,
                                    IOpaqueAddress* pAddress)   {
  if ( checkRoot() )  {
    if ( objPath.length() > 0 )   {
      if ( 0 == parentObj )   {
        if ( objPath[0] != SEPARATOR )    {
          return registerAddress(m_root, objPath, pAddress);
        }
        std::string::size_type sep = objPath.find(SEPARATOR,1);
        if ( sep != std::string::npos )    {
          std::string p_path (objPath, 0, sep);
          if ( p_path == m_rootName )   {
            std::string o_path (objPath, sep, objPath.length());
            return registerAddress(m_root, o_path, pAddress);
          }
        }
        return INVALID_PARENT;
      }
      if ( objPath[0] != SEPARATOR )    {
        std::string path;
        path  = SEPARATOR;
        path += objPath;
        return registerAddress(parentObj, path, pAddress);
      }
      RegEntry* par_entry = CAST_REGENTRY(RegEntry*,parentObj);
      if ( 0 != par_entry )   {
        std::string::size_type sep = objPath.rfind(SEPARATOR);
        if ( sep > 0 && sep != std::string::npos )    {
          std::string p_path (objPath, 0, sep);
          std::string o_path (objPath, sep, objPath.length());
          RegEntry* p_entry = par_entry->findLeaf(p_path);
          // Create default object leafs if the
          // intermediate nodes are not present
          if ( 0 == p_entry && m_forceLeaves )    {
            DataObject *pLeaf = createDefaultObject();
            StatusCode sc = registerObject(par_entry->identifier(),
                                           p_path,
                                           pLeaf);
            if ( ! sc.isSuccess() )   {
              delete pLeaf;
            }
            p_entry = par_entry->findLeaf(p_path);
          }
          if ( 0 != p_entry )  {
            return registerAddress(p_entry, o_path, pAddress);
          }
          return INVALID_PARENT;
        }
        StatusCode status = par_entry->add(objPath, pAddress);
        if ( status.isSuccess() )   {
          return status;
        }
        return DOUBL_OBJ_PATH;
      }
      return INVALID_PARENT;
    }
    return INVALID_OBJ_PATH;
  }
  return INVALID_ROOT;
}

///  IDataManagerSvc: Unregister object address from the data store.
StatusCode DataSvc::unregisterAddress(const std::string& fullPath)    {
  if ( fullPath.length() > 0 )    {
    IRegistry* pRegistry = 0;
    if ( fullPath[0] != SEPARATOR )   {
      return unregisterAddress(m_root, fullPath);
    }
    return unregisterAddress(pRegistry, fullPath);
  }
  return INVALID_OBJ_PATH;
}

///  IDataManagerSvc: Unregister object address from the data store.
StatusCode DataSvc::unregisterAddress(DataObject* pParent,
                                      const std::string& objPath)   {
  IRegistry* pRegistry = (0 == pParent) ? 0 : pParent->registry();
  return unregisterAddress(pRegistry, objPath);
}

///  IDataManagerSvc: Unregister object address from the data store.
StatusCode DataSvc::unregisterAddress(IRegistry* pParent,
                                      const std::string& objPath)   {
  if ( checkRoot() )  {
    if ( objPath.length() > 0 )   {
      if ( 0 == pParent )   {
        if ( objPath[0] != SEPARATOR )    {
          return unregisterAddress(m_root, objPath);
        }
        std::string::size_type sep = objPath.find(SEPARATOR,1);
        if ( sep != std::string::npos )    {
          std::string p_path (objPath, 0, sep);
          if ( p_path == m_rootName )   {
            std::string o_path (objPath, sep, objPath.length());
            return unregisterAddress(m_root, o_path);
          }
        }
        return INVALID_PARENT;
      }
      if ( objPath[0] != SEPARATOR )    {
        std::string path;
        path  = SEPARATOR;
        path += objPath;
        return unregisterAddress(pParent, path);
      }
      RegEntry* node_entry = CAST_REGENTRY(RegEntry*,pParent);
      if ( 0 != node_entry )   {
        RegEntry* leaf_entry = node_entry->findLeaf(objPath);
        if ( 0 != leaf_entry )    {
          std::string::size_type sep = objPath.rfind(SEPARATOR);
          if ( sep > 0 && sep != std::string::npos )    {
            std::string path = objPath.substr(sep);
            return unregisterAddress(leaf_entry->parent(), path);
          }
          StatusCode status = node_entry->remove(objPath);
          if ( status.isSuccess() )   {
            return status;
          }
        }
      }
      return INVALID_PARENT;
    }
    return INVALID_OBJ_PATH;
  }
  return INVALID_ROOT;
}

/// Register object with the data store.
StatusCode DataSvc::registerObject (const std::string& fullPath,
                                    DataObject* pObject)   {
  return registerObject(0, fullPath, pObject);
}


/// Register object with the data store.
StatusCode DataSvc::registerObject (const std::string& parentPath,
                                    const std::string& objPath,
                                    DataObject* pObject)   {
  DataObject* pO = 0;
  StatusCode status = retrieveObject(parentPath, pO);
  if ( !status.isSuccess() && m_forceLeaves )   {
    pO = createDefaultObject();
    status = registerObject(parentPath, pO);
    if ( !status.isSuccess() )   {
      pO->release();
    }
  }
  if ( status.isSuccess() )   {
    status = registerObject(pO, objPath, pObject);
  }
  return status;
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
  if ( checkRoot() )  {
    if ( 0 == parentObj )   {
      if ( objPath.length() > 0 )   {
        if ( objPath[0] == SEPARATOR )    {
          std::string::size_type sep = objPath.find(SEPARATOR,1);
          if ( sep != std::string::npos )    {
            std::string p_path (objPath, 0, sep);
            std::string o_path (objPath, sep, objPath.length());
            return registerObject(p_path, o_path, pObject);
          }
        }
        else  {
          return registerObject(m_rootName, objPath, pObject);
        }
      }
      return INVALID_OBJ_PATH;
    }
    RegEntry* node_entry = CAST_REGENTRY(RegEntry*,parentObj->registry());
    if ( 0 != node_entry )   {
      StatusCode status = INVALID_PARENT;
      std::string::size_type sep = objPath.find(SEPARATOR,1);
      if ( sep != std::string::npos )    {
        std::string p_path (objPath, 0, sep);
        std::string o_path (objPath, sep, objPath.length());
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
          status = retrieveEntry( node_entry, p_path, par_entry);
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
        if ( 0 != node_entry )    {
          DataObject* obj = node_entry->object();
          if ( 0 != obj )   {
            status = registerObject( obj, o_path, pObject );
          }
        }
      }
      else   {
        RegEntry* leaf = node_entry->findLeaf(objPath);
        if ( 0 == leaf )    {
          status = node_entry->add( objPath, pObject );
        }
        else    {
          DataObject* obj = leaf->object();
          if ( 0 == obj )    {
            if (0 == pObject) {
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
  return INVALID_ROOT;
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject(const std::string& fullPath)   {
  DataObject* pObject = 0;
  StatusCode status = findObject(fullPath, pObject);
  if ( status.isSuccess() )   {
    RegEntry* pEntry = CAST_REGENTRY(RegEntry*,pObject->registry());
    if ( 0 != pEntry )    {
      if ( pEntry->isEmpty() )    {
        RegEntry* pParent = pEntry->parentEntry();
        if ( 0 != pParent )   {
          if ( 0 != pObject )    {
            pObject->addRef();
          }
          pParent->remove(pEntry);
          return StatusCode::SUCCESS;
        }
        return INVALID_PARENT;
      }
      return DIR_NOT_EMPTY;
    }
    return INVALID_ROOT;
  }
  return status;
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject(const std::string& parentPath,
                                     const std::string& objPath)   {
  DataObject* pO = 0;
  StatusCode status = findObject(parentPath, pO);
  if ( status.isSuccess() )   {
    status = unregisterObject(pO, objPath);
  }
  return status;
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject(const std::string& parentPath, int item) {
  return unregisterObject(parentPath, itemToPath(item));
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject(DataObject* pObject)    {
  if ( checkRoot() )  {
    RegEntry* entry = m_root->findLeaf(pObject);
    if ( 0 != entry )   {
      RegEntry* parent = entry->parentEntry();
      if ( 0 != parent )    {
        if ( entry->isEmpty() )    {
          if ( 0 != entry->object() )    {
            entry->object()->addRef();
          }
          if ( 0 != parent )  {
            parent->remove(entry);
          }
          return SUCCESS;
        }
        return INVALID_PARENT;
      }
      return DIR_NOT_EMPTY;
    }
    return INVALID_OBJECT;
  }
  return INVALID_ROOT;
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject (DataObject* pParentObj,
                                      const std::string& objectPath) {
  if ( checkRoot() )  {
    try   {
      RegEntry* parent = CAST_REGENTRY(RegEntry*,pParentObj->registry());
      if ( 0 != parent )   {
        RegEntry* entry = parent->findLeaf(objectPath);
        if ( 0 != entry )   {
          if ( entry->isEmpty() )    {
            if ( 0 != entry->object() )    {
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
DataObject* DataSvc::handleDataFault(IRegistry* pReg, const std::string& path)
{
  if ( m_enableFaultHdlr )  {
    IRegistry* pLeaf = 0;
    if ( pReg && path.length() == 0 )   {
      DataIncident incident(name(), m_faultName, pReg->identifier());
      m_incidentSvc->fireIncident(incident);
      return pReg->object();
    }
    else if ( pReg )  {
      std::string p = pReg->identifier();
      if (path[0] != SEPARATOR ) p += SEPARATOR;
      p += path;
      DataIncident incident(name(), m_faultName, p);
      m_incidentSvc->fireIncident(incident);
      pLeaf = m_root->findLeaf(p);
    }
    else  {
      std::string p = m_root->identifier();
      if (path[0] != SEPARATOR ) p += SEPARATOR;
      p += path;
      DataIncident incident(name(), m_faultName, p);
      m_incidentSvc->fireIncident(incident);
      pLeaf = m_root->findLeaf(p);
    }
    if ( pLeaf )  {
      return pLeaf->object();
    }
  }
  return 0;
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
  DataObject* pObject = 0;
  if ( 0 == pLoader )   {         // Precondition: Data loader must be present
    if (handleDataFault(pRegistry) != 0) return  SUCCESS;
    else                                 return  NO_DATA_LOADER;
  }
  if ( 0 == pRegistry )    {      // Precondition: Directory must be valid
    if (handleDataFault(pRegistry) != 0) return  SUCCESS;
    else                                 return  INVALID_OBJ_ADDR;
  }

  VERMSG << "Requested object " << pRegistry->identifier() << endmsg;

  if ( m_enableAccessHdlr )  {
    // Fire data access incident
    DataIncident incident(name(), m_accessName, pRegistry->identifier());
    m_incidentSvc->fireIncident(incident);
  }
  if ( m_inhibitPathes.size() > 0 )   {
    const std::string& ident = pRegistry->identifier();
    std::vector<std::string>::iterator inhibit =
      std::find(m_inhibitPathes.begin(), m_inhibitPathes.end(), ident);
    if ( inhibit != m_inhibitPathes.end() )   {
      return NO_ACCESS;
    }
  }
  IOpaqueAddress* pAddress = pRegistry->address();
  if ( 0 == pAddress )  {         // Precondition:
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
    if ( handleDataFault(pRegistry) != 0 )  {
      return SUCCESS;
    }
    throw GaudiException("GaudiException in loadObject() " + pRegistry->identifier(),
        name(), StatusCode::FAILURE, exc);
  }
  catch( const std::exception& x) {
    if ( handleDataFault(pRegistry) != 0 )  {
      return SUCCESS;
    }
    throw GaudiException("std::exception in loadObject() " + pRegistry->identifier() +
        ": " + System::typeinfoName(typeid(x)) + ", " + x.what(),
      name(), StatusCode::FAILURE);
  }
  catch(...) {
    if ( handleDataFault(pRegistry) != 0 )  {
      return SUCCESS;
    }
    throw GaudiException("UNKN exception in loadObject() " + pRegistry->identifier(),
      name(), StatusCode::FAILURE);
  }
  if ( !status.isSuccess() )  {
    if ( handleDataFault(pRegistry) != 0 )  {
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
  std::string::size_type        sep    = path.find(SEPARATOR,1);
  StatusCode status = StatusCode(INVALID_ROOT,true);
  pEntry = 0;
  // A.Valassi 16.08.2001 avoid core dump if store is empty
  if ( checkRoot() )  {
    if ( 0 == parentObj )    {
      if ( path.length() == 0 || path == m_rootName )   {
        return retrieveEntry(m_root, "", pEntry);
      }
      else if ( path[0] != SEPARATOR )  {
        return retrieveEntry(m_root, path, pEntry);
      }
      else if ( sep != std::string::npos )    {
        if ( m_root->object() == 0 )  {
          RegEntry* r = 0;
          status = retrieveEntry(m_root, "", r);
          if ( !status.isSuccess() )  {
            return status;
          }
        }
        std::string o_path (path, sep, path.length());
        return retrieveEntry(m_root, o_path, pEntry);
      }
      return INVALID_OBJ_PATH;
    }
    if ( sep != std::string::npos )  { // the string contains a separator (after pos 0)
      std::string p_path (path,0,sep);
      std::string o_path (path,sep,path.length());
      if (!parentObj->object()) { // if the parent object has not been loaded yet, load it now
        status = loadObject(parentObj);
        if ( !status.isSuccess() )  {
          return status;
        }
      }
      RegEntry* root_entry = parentObj->findLeaf(p_path);
      if ( !root_entry && m_enableFaultHdlr )    {
        // If not even the parent is there, an incident
        // to load the parent must be fired...
        handleDataFault(parentObj, p_path);
        root_entry = parentObj->findLeaf(p_path);
      }
      if ( root_entry )    {
        DataObject* pO = root_entry->object();
        if ( 0 == pO )  {
          // Object is not loaded: load the object if at all possible
          status = loadObject(root_entry);
          if ( !status.isSuccess() )  {
            return status;
          }
        }
        if ( root_entry->isSoft() )    {
          root_entry = CAST_REGENTRY(RegEntry*,pO->registry());
        }
        return retrieveEntry (root_entry, o_path, pEntry);
      }
      return status;
    }
    else if ( path.length() == 0 )    {
      pEntry = parentObj;
    }
    else {
      if (!parentObj->object()) { // if the parent object has not been loaded yet, load it now
        status = loadObject(parentObj);
        if ( !status.isSuccess() )  {
          return status;
        }
      }
      // last leave in search: find leaf and load
      pEntry = parentObj->findLeaf(path);
      // If no registry entry was found, trigger incident for action-on-demand
      if ( !pEntry && m_enableFaultHdlr )  {
        handleDataFault(parentObj, path);
        pEntry = (0==path.length()) ? parentObj : parentObj->findLeaf(path);
      }
    }
    // Check results and return
    if ( 0 == pEntry )  {
      status = INVALID_OBJ_PATH;
    }
    else if ( 0 == pEntry->object() )  {
      status = loadObject(pEntry);
    }
    else if ( m_enableAccessHdlr )  {
      // Fire data access incident
      // I do not know if this is a good idea....
      // This fires too often!
      //
      //DataIncident incident(name(), m_accessName, pEntry->identifier());
      //m_incidentSvc->fireIncident(incident);
      status = SUCCESS;
    }
    else  {
      status = SUCCESS;
    }
  }
  return status;
}

/// Retrieve object identified by its directory from the data store.
StatusCode DataSvc::retrieveObject(IRegistry* pRegistry,
                                   const std::string& path,
                                   DataObject*& pObject)   {
  pObject = 0;
  RegEntry *result = 0, *parent = CAST_REGENTRY(RegEntry*,pRegistry);
  StatusCode status = retrieveEntry(parent, path, result);
  if ( status.isSuccess() )   {
    pObject = result->object();
  }
  return status;
}

/// Retrieve object identified by its full path from the data store.
StatusCode DataSvc::retrieveObject(const std::string& fullPath,
                                   DataObject*& pObject)   {
  IRegistry* nullDir = 0;
  return retrieveObject(nullDir, fullPath, pObject);
}

/// Retrieve object identified by its full path from the data store.
StatusCode DataSvc::retrieveObject(const std::string& parentPath,
                                   const std::string& objectPath,
                                   DataObject*& pObject)   {
  DataObject* parent = 0;
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
  IRegistry* pRegistry = (0==parentObj) ? 0 : parentObj->registry();
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
  pObject = 0;
  IRegistry* pReg = (0==pRegistry) ? m_root : pRegistry;
  RegEntry* root_entry = CAST_REGENTRY(RegEntry*, pReg);
  if ( 0 != root_entry )    {
    if ( path.length() > 0 )  {
      pReg = root_entry->find(path);
    }
    if ( 0 == pReg )    {
      return INVALID_OBJ_PATH;
    }
    pObject = pReg->object();
  }
  return (0 == pObject) ? OBJ_NOT_LOADED : IDataProviderSvc_NO_ERROR;
}

/// Find object identified by its full path in the data store.
StatusCode DataSvc::findObject(const std::string& path,
                               DataObject*& pObject)   {
  pObject = 0;
  if ( checkRoot() )  {
    if ( path.length() == 0 || path == m_rootName )   {
      pObject = m_root->object();
      return (0 == pObject) ? OBJ_NOT_LOADED : IDataProviderSvc_NO_ERROR;
    }
    else if ( path[0] != SEPARATOR )    {
      return findObject(m_rootName, path, pObject);
    }
    return findObject((IRegistry*)0, path, pObject);
  }
  return INVALID_ROOT;
}

/// Retrieve object identified by its full path from the data store.
StatusCode DataSvc::findObject(const std::string& parentPath,
                               const std::string& objectPath,
                               DataObject*& pObject)   {
  DataObject* parent = 0;
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
  IRegistry* pDir = (0==parentObj) ? 0 : parentObj->registry();
  return findObject(pDir, path, pObject);
}

/// Update object.
StatusCode DataSvc::updateObject(const std::string& updatePath)   {
  DataObject* pO = 0;
  StatusCode status = findObject(updatePath, pO);
  if ( status.isSuccess() )   {
    return updateObject(pO);
  }
  return retrieveObject(updatePath, pO);
}

/// Update object.
StatusCode DataSvc::updateObject(IRegistry* pRegistry)    {
  if ( 0 == pRegistry )  {                     // Precondition:
    return INVALID_OBJ_ADDR;                   // Addres must be valid
  }
  DataObject* toUpdate = pRegistry->object();
  if ( 0 == toUpdate )  {                      // Try first to load
    return loadObject(pRegistry);
  }
  return updateObject(toUpdate);
}

/// Update object.
StatusCode DataSvc::updateObject(DataObject* toUpdate)    {
  StatusCode status = INVALID_OBJ_ADDR;
  if ( 0 == toUpdate )  {                          // Precondition:
    return INVALID_OBJECT;                         // Address must be valid
  }
  IRegistry* pRegistry = toUpdate->registry();     // Precondition:
  if ( 0 == pRegistry )    {                       // Need valid registry
    return INVALID_OBJECT;
  }
  IOpaqueAddress* pAddress = pRegistry->address(); // Precondition:
  if ( 0 == pAddress )    {                        // Need valid address
    return INVALID_OBJ_ADDR;
  }
  IConversionSvc* pLoader = getDataLoader(pRegistry);
  if ( 0 == pLoader )   {                      // Precondition:
    return NO_DATA_LOADER;                     // Data loader must be present
  }
  if ( m_inhibitPathes.size() > 0 )   {
    const std::string& ident = pRegistry->identifier();
    std::vector<std::string>::iterator inhibit =
      std::find(m_inhibitPathes.begin(), m_inhibitPathes.end(), ident);
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
  DataObject* pParent = 0;
  StatusCode status = findObject(parentPath, pParent);
  if ( status.isSuccess() )   {
    status = updateObject( pParent, updatePath);
  }
  return status;
}

/// Update object.
StatusCode DataSvc::updateObject(DataObject* parent,
                                 const std::string& updatePath)   {
  DataObject* pObject = 0;
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
      if ( 0 != from_entry )    {
        // First check if both objects are already registered to the store
        RegEntry* to_entry   = m_root->findLeaf(to);
        if ( 0 == to_entry )   {
          return INVALID_OBJECT;
        }
        else  {
          std::string::size_type sep = objPath.rfind(SEPARATOR);
          if ( sep > 0 && sep != std::string::npos )    {   // in case the objPath is a sub-directory itself
            DataObject* pO = 0;
            std::string fromPath(objPath, 0, sep);
            StatusCode sc = retrieveObject(from, fromPath, pO);
            if ( sc.isSuccess() )    {
              std::string toPath(objPath, sep, objPath.length());
              sc = linkObject(pO->registry(), toPath, to);
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
  if ( fullPath.length() > 0 )    {
    if ( fullPath[0] != SEPARATOR )   {
      return linkObject(m_rootName, fullPath, to);
    }
    std::string::size_type sep = fullPath.rfind(SEPARATOR);
    std::string objPath(fullPath, sep, fullPath.length());
    std::string fromPath(fullPath, 0, sep);
    return linkObject( fromPath, objPath, to);
  }
  return INVALID_OBJ_PATH;
}

/// Add a link to another object.
StatusCode DataSvc::linkObject(const std::string& from,
                               const std::string& objPath,
                               DataObject* to)    {
  DataObject* pO = 0;
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
  if ( 0 != from )    {
    IRegistry* from_entry = from->registry();
    if ( 0 != from_entry )   {
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
      if ( 0 != from_entry )    {
        std::string::size_type sep = objPath.rfind(SEPARATOR);
        if ( sep > 0 && sep != std::string::npos )    {  // in case the objPath is a sub-directory itself
          DataObject* pO = 0;
          std::string fromPath(objPath, 0, sep);
          StatusCode sc = findObject(from, fromPath, pO);
          if ( sc.isSuccess() )    {
            std::string toPath(objPath, sep, objPath.length());
            sc = unlinkObject(pO->registry(), toPath);
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
  if ( fullPath.length() > 0 )    {
    if ( fullPath[0] != SEPARATOR )   {
      return unlinkObject(m_rootName, fullPath);
    }
    std::string::size_type sep = fullPath.rfind(SEPARATOR);
    std::string objPath(fullPath, sep, fullPath.length());
    std::string fromPath(fullPath, 0, sep);
    return unlinkObject(fromPath, objPath);
  }
  return INVALID_OBJ_PATH;
}

/// Remove a link to another object.
StatusCode DataSvc::unlinkObject(const std::string& from,
                                 const std::string& objPath)    {
  DataObject* pObject = 0;
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
  LoadItems::iterator i = std::find(m_preLoads.begin(), m_preLoads.end(), item);
  if ( i == m_preLoads.end() )    {
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
  LoadItems::iterator i =
    std::remove(m_preLoads.begin(), m_preLoads.end(), item);
  if ( i != m_preLoads.end() )  {
    m_preLoads.erase(i, m_preLoads.end());
  }
  return StatusCode::SUCCESS;
}

/// Add an item to the preload list
StatusCode DataSvc::removePreLoadItem(const std::string& itemPath)    {
  return removePreLoadItem( DataStoreItem(itemPath,1) );
}

/// Clear the preload list
StatusCode DataSvc::resetPreLoad()    {
  m_preLoads.erase(m_preLoads.begin(), m_preLoads.end());
  return StatusCode::SUCCESS;
}

/// Preload one level deep, then recursively call the next level
StatusCode DataSvc::preLoad(int depth, int load_depth, DataObject* pObject)    {
  //unused:  StatusCode sc = StatusCode::FAILURE;
  if ( 0 != pObject && depth++ < load_depth )    {
    RegEntry* dir = CAST_REGENTRY(RegEntry*,pObject->registry());
    if ( 0 != dir )   {
      for (RegEntry::Iterator i = dir->begin(); i != dir->end(); i++ )    {
        DataObject *pObj = 0;
        StatusCode status = retrieveObject(pObject, (*i)->name(), pObj);
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
  DataObject* pObj = 0;
  for (LoadItems::iterator i = m_preLoads.begin(); i != m_preLoads.end(); i++) {
    StatusCode sc = retrieveObject( (*i).path(), pObj);
    int load_depth = (*i).depth();
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
  StatusCode sc;
  // the finalize part is copied here
  setDataLoader(0).ignore();
  resetPreLoad().ignore();
  clearStore().ignore();
  if ( m_incidentSvc )  {
    m_incidentSvc->release();
    m_incidentSvc = 0;
  }
  // re-initialize the base class
  sc = Service::reinitialize();
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
  setDataLoader(0).ignore();
  resetPreLoad().ignore();
  clearStore().ignore();
  if ( m_incidentSvc )  {
    m_incidentSvc->release();
    m_incidentSvc = 0;
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
  m_rootName( "/Event"), m_root(0)
{
  m_dataLoader = 0;
  m_inhibitMap = 0;
  m_incidentSvc = 0;
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
  setDataLoader(0).ignore();
  resetPreLoad().ignore();
  clearStore().ignore();
}
