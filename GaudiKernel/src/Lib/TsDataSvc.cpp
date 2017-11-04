//====================================================================
//	TsDataSvc.cpp
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description: implementation of the Transient data service: TsDataSvc
//
//  Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/98| Initial version                              | M.Frank
// | 20/2/99 | Automatic data preloading introduced.        | M.Frank
// | 25/4/13 | Locking for concurrent access introduced.    | D.Piparo
// +---------+----------------------------------------------+---------
//
//====================================================================
#define TSDATASVC_DATASVC_CPP

// Framework include files
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/IOpaqueAddress.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/xtoa.h"

#include "GaudiKernel/DataIncident.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/RegistryEntry.h"
#include "GaudiKernel/TsDataSvc.h"

// Include files
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <mutex>
#include <sstream>
#include <vector>

// Macro to lock a scope
#define STD_LOCK_GUARD_MACRO std::lock_guard<tsDataSvcMutex> lock( m_accessMutex );

namespace
{
  /// Helper function to convert item numbers to path strings
  /// i.e. int -> "/" + int
  std::string itemToPath( int item ) { return '/' + std::to_string( item ); }

  std::string operator+( char c, boost::string_ref sr )
  {
    std::string s{{c}};
    s.append( sr.data(), sr.size() );
    return s;
  }

  boost::string_ref::size_type find( boost::string_ref s, char c, size_t o )
  {
    if ( !s.empty() ) s.remove_prefix( o );
    auto r = s.find( c );
    return r == boost::string_ref::npos ? r : ( r + o );
  }

  std::string to_string( boost::string_ref sr ) { return {sr.data(), sr.size()}; }
}

// If you absolutely need optimization: switch off dynamic_cast.
// This improves access to the data store roughly by 10 %
// for balanced trees.
//
// M.Frank
#define CAST_REGENTRY( x, y ) dynamic_cast<x>( y )
//#define CAST_REGENTRY(x,y) (x)(y)
typedef DataSvcHelpers::RegistryEntry RegEntry;

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

#define DEBMSG ON_DEBUG debug()
#define VERMSG ON_VERBOSE verbose()

/** IDataManagerSvc: Remove all data objects below the sub tree
 *  identified by its full path name.
 */
StatusCode TsDataSvc::clearSubTree( boost::string_ref sub_tree_path )
{
  DataObject* pObject = 0;
  StatusCode status   = findObject( sub_tree_path, pObject );
  if ( status.isSuccess() ) {
    RegEntry* node_entry = CAST_REGENTRY( RegEntry*, pObject->registry() );
    if ( 0 != node_entry ) {
      RegEntry* parent = node_entry->parentEntry();
      if ( 0 != parent ) {
        parent->remove( node_entry );
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
StatusCode TsDataSvc::clearSubTree( DataObject* pObject )
{
  if ( checkRoot() ) {
    RegEntry* entry = CAST_REGENTRY( RegEntry*, pObject->registry() );
    if ( 0 != entry ) {
      RegEntry* parent = entry->parentEntry();
      if ( 0 != parent ) {
        parent->remove( entry );
        return SUCCESS;
      }
      return INVALID_PARENT;
    }
    return INVALID_OBJECT;
  }
  return INVALID_ROOT;
}

/// IDataManagerSvc: Remove all data objects in the data store.
StatusCode TsDataSvc::clearStore()
{
  if ( !checkRoot() ) return INVALID_ROOT;
  m_root->release();
  m_root = nullptr;
  return SUCCESS;
}

/** IDataManagerSvc: Analyse by traversing all data objects below the
 * sub tree identified by its full path name.
 */
StatusCode TsDataSvc::traverseSubTree( boost::string_ref sub_tree_path, IDataStoreAgent* pAgent )
{
  STD_LOCK_GUARD_MACRO
  DataObject* pO    = nullptr;
  StatusCode status = findObject( sub_tree_path, pO );
  return status.isSuccess() ? traverseSubTree( pO, pAgent ) : status;
}

/// IDataManagerSvc: Analyse by traversing all data objects below the sub tree
StatusCode TsDataSvc::traverseSubTree( DataObject* pObject, IDataStoreAgent* pAgent )
{
  STD_LOCK_GUARD_MACRO
  if ( !checkRoot() ) return INVALID_ROOT;
  RegEntry* entry = CAST_REGENTRY( RegEntry*, pObject->registry() );
  if ( !entry ) return INVALID_OBJECT;
  return entry->traverseTree( pAgent );
}

/// IDataManagerSvc: Analyse by traversing all data objects in the data store.
StatusCode TsDataSvc::traverseTree( IDataStoreAgent* pAgent )
{
  STD_LOCK_GUARD_MACRO
  if ( !checkRoot() ) return INVALID_ROOT;
  return m_root->traverseTree( pAgent );
}

/**
 * Initialize data store for new event by giving new event path and root
 * object. Takes care to clear the store before reinitializing it
 */
StatusCode TsDataSvc::setRoot( std::string root_path, DataObject* pRootObj )
{
  clearStore().ignore();
  return i_setRoot( std::move( root_path ), pRootObj );
}

/**
 * Initialize data store for new event by giving new event path and root
 * object. Does not clear the store before reinitializing it. This could
 * lead to errors and should be handle with care. Use setRoot if unsure
 */
StatusCode TsDataSvc::i_setRoot( std::string root_path, DataObject* pRootObj )
{
  if ( 0 != pRootObj ) {
    m_root = new RegEntry( std::move( root_path ) );
    m_root->makeHard( pRootObj );
    m_root->setDataSvc( this );
    // No done with GaudiHive. preLoad().ignore();
  }
  return SUCCESS;
}

/**
 * Initialize data store for new event by giving new event path and address
 * of root object. Takes care to clear the store before reinitializing it
 */
StatusCode TsDataSvc::setRoot( std::string root_path, IOpaqueAddress* pRootAddr )
{
  clearStore().ignore();
  return i_setRoot( std::move( root_path ), pRootAddr );
}

/**
 * Initialize data store for new event by giving new event path and address
 * of root object. Does not clear the store before reinitializing it. This
 * could lead to errors and should be handle with care. Use setRoot if unsure
 */
StatusCode TsDataSvc::i_setRoot( std::string root_path, IOpaqueAddress* pRootAddr )
{
  if ( 0 != pRootAddr ) {
    m_root = new RegEntry( std::move( root_path ) );
    m_root->makeHard( pRootAddr );
    m_root->setDataSvc( this );
    // Not done with GaudiHive. preLoad().ignore();
  }
  return SUCCESS;
}

/// IDataManagerSvc: Pass a default data loader to the service.
StatusCode TsDataSvc::setDataLoader( IConversionSvc* pDataLoader, IDataProviderSvc* dpsvc )
{
  if ( 0 != pDataLoader ) pDataLoader->addRef();
  if ( 0 != m_dataLoader ) m_dataLoader->release();
  if ( 0 != pDataLoader ) {
    pDataLoader->setDataProvider( dpsvc == nullptr ? this : dpsvc ).ignore();
  }
  m_dataLoader = pDataLoader;
  return SUCCESS;
}

/// IDataManagerSvc: Explore the object store: retrieve the object's parent
StatusCode TsDataSvc::objectParent( const DataObject* pObject, IRegistry*& refpParent )
{
  STD_LOCK_GUARD_MACRO
  if ( pObject ) {
    return objectParent( pObject->registry(), refpParent );
  }
  return INVALID_OBJECT;
}
/// IDataManagerSvc: Explore the object store: retrieve the object's parent
StatusCode TsDataSvc::objectParent( const IRegistry* pRegistry, IRegistry*& refpParent )
{
  STD_LOCK_GUARD_MACRO
  if ( checkRoot() ) {
    const RegEntry* node_entry = CAST_REGENTRY( const RegEntry*, pRegistry );
    if ( node_entry ) {
      refpParent = node_entry->parent();
      return StatusCode::SUCCESS;
    }
    return INVALID_OBJECT;
  }
  return INVALID_ROOT;
}

/// IDataManagerSvc: Explore an object identified by its pointer.
StatusCode TsDataSvc::objectLeaves( const DataObject* pObject, std::vector<IRegistry*>& leaves )
{
  STD_LOCK_GUARD_MACRO
  if ( pObject ) {
    return objectLeaves( pObject->registry(), leaves );
  }
  return INVALID_OBJECT;
}

/** IDataManagerSvc: Explore an object identified by the pointer to the
 * registry entry.
 */
StatusCode TsDataSvc::objectLeaves( const IRegistry* pRegistry, std::vector<IRegistry*>& leaves )
{
  STD_LOCK_GUARD_MACRO
  if ( checkRoot() ) {
    const RegEntry* node_entry = CAST_REGENTRY( const RegEntry*, pRegistry );
    if ( node_entry ) {
      std::copy( node_entry->leaves().begin(), node_entry->leaves().end(), back_inserter( leaves ) );
      // leaves = node_entry->leaves();
      return StatusCode::SUCCESS;
    }
    return INVALID_OBJECT;
  }
  return INVALID_ROOT;
}

///  IDataManagerSvc: Register object address with the data store.
StatusCode TsDataSvc::registerAddress( boost::string_ref fullPath, IOpaqueAddress* pAddress )
{
  if ( fullPath.length() > 0 ) {
    if ( fullPath[0] != SEPARATOR ) {
      return registerAddress( m_root, fullPath, pAddress );
    }
    IRegistry* pRegistry = 0;
    return registerAddress( pRegistry, fullPath, pAddress );
  }
  return INVALID_OBJ_PATH;
}

///  IDataManagerSvc: Register object address with the data store.
StatusCode TsDataSvc::registerAddress( DataObject* parentObj, boost::string_ref objectPath, IOpaqueAddress* pAddress )
{
  IRegistry* pRegistry = ( 0 == parentObj ) ? 0 : parentObj->registry();
  return registerAddress( pRegistry, objectPath, pAddress );
}

///  IDataManagerSvc: Register object address with the data store.
StatusCode TsDataSvc::registerAddress( IRegistry* parentObj, boost::string_ref objPath, IOpaqueAddress* pAddress )
{
  if ( !checkRoot() ) return INVALID_ROOT;
  if ( objPath.empty() ) return INVALID_OBJ_PATH;
  if ( !parentObj ) {
    if ( objPath.front() != SEPARATOR ) {
      return registerAddress( m_root, objPath, pAddress );
    }
    auto sep = find( objPath, SEPARATOR, 1 );
    if ( sep == boost::string_ref::npos || objPath.substr( 0, sep ) != m_rootName.value() ) {
      return INVALID_PARENT;
    }
    return registerAddress( m_root, objPath.substr( sep ), pAddress );
  }
  if ( objPath.front() != SEPARATOR ) {
    return registerAddress( parentObj, SEPARATOR + objPath, pAddress );
  }
  RegEntry* par_entry = CAST_REGENTRY( RegEntry*, parentObj );
  if ( !par_entry ) return INVALID_PARENT;
  auto sep = objPath.rfind( SEPARATOR );
  if ( sep > 0 && sep != std::string::npos ) {
    auto p_path       = objPath.substr( 0, sep );
    auto o_path       = objPath.substr( sep );
    RegEntry* p_entry = par_entry->findLeaf( p_path );
    // Create default object leafs if the
    // intermediate nodes are not present
    if ( !p_entry && m_forceLeaves ) {
      DataObject* pLeaf = createDefaultObject();
      StatusCode sc     = registerObject( par_entry->identifier(), p_path, pLeaf );
      if ( !sc.isSuccess() ) delete pLeaf;
      p_entry = par_entry->findLeaf( p_path );
    }
    if ( !p_entry ) return INVALID_PARENT;
    return registerAddress( p_entry, o_path, pAddress );
  }
  StatusCode status = par_entry->add( to_string( objPath ), pAddress );
  return status.isSuccess() ? status : DOUBL_OBJ_PATH;
}

///  IDataManagerSvc: Unregister object address from the data store.
StatusCode TsDataSvc::unregisterAddress( boost::string_ref fullPath )
{
  if ( fullPath.empty() ) return INVALID_OBJ_PATH;
  IRegistry* pRegistry = nullptr;
  if ( fullPath.front() != SEPARATOR ) {
    return unregisterAddress( m_root, fullPath );
  }
  return unregisterAddress( pRegistry, fullPath );
}

///  IDataManagerSvc: Unregister object address from the data store.
StatusCode TsDataSvc::unregisterAddress( DataObject* pParent, boost::string_ref objPath )
{
  IRegistry* pRegistry = ( 0 == pParent ) ? 0 : pParent->registry();
  return unregisterAddress( pRegistry, objPath );
}

///  IDataManagerSvc: Unregister object address from the data store.
StatusCode TsDataSvc::unregisterAddress( IRegistry* pParent, boost::string_ref objPath )
{
  if ( !checkRoot() ) return INVALID_ROOT;

  if ( objPath.empty() ) return INVALID_OBJ_PATH;
  if ( !pParent ) {
    if ( objPath.front() != SEPARATOR ) {
      return unregisterAddress( m_root, objPath );
    }
    auto sep = find( objPath, SEPARATOR, 1 );
    if ( sep == boost::string_ref::npos || objPath.substr( 0, sep ) != m_rootName.value() ) {
      return INVALID_PARENT;
    }
    return unregisterAddress( m_root, objPath.substr( sep ) );
  }
  if ( objPath.front() != SEPARATOR ) {
    return unregisterAddress( pParent, SEPARATOR + objPath );
  }
  RegEntry* node_entry = CAST_REGENTRY( RegEntry*, pParent );
  if ( node_entry ) {
    RegEntry* leaf_entry = node_entry->findLeaf( objPath );
    if ( leaf_entry ) {
      auto sep = objPath.rfind( SEPARATOR );
      if ( sep > 0 && sep != boost::string_ref::npos ) {
        return unregisterAddress( leaf_entry->parent(), objPath.substr( sep ) );
      }
      StatusCode status = node_entry->remove( objPath );
      if ( status.isSuccess() ) return status;
    }
  }
  return INVALID_PARENT;
}

/// Register object with the data store.
StatusCode TsDataSvc::registerObject( boost::string_ref fullPath, DataObject* pObject )
{
  return registerObject( 0, fullPath, pObject );
}

/// Register object with the data store.
StatusCode TsDataSvc::registerObject( boost::string_ref parentPath, boost::string_ref objPath, DataObject* pObject )
{
  DataObject* pO    = 0;
  StatusCode status = retrieveObject( parentPath, pO );
  if ( !status.isSuccess() && m_forceLeaves ) {
    pO     = createDefaultObject();
    status = registerObject( parentPath, pO );
    if ( !status.isSuccess() ) {
      pO->release();
    }
  }
  if ( status.isSuccess() ) {
    status = registerObject( pO, objPath, pObject );
  }
  return status;
}

/// Register object with the data store.
StatusCode TsDataSvc::registerObject( boost::string_ref parentPath, int item, DataObject* pObject )
{
  return registerObject( parentPath, itemToPath( item ), pObject );
}

/// Register object with the data store.
StatusCode TsDataSvc::registerObject( DataObject* parentObj, int item, DataObject* pObject )
{
  return registerObject( parentObj, itemToPath( item ), pObject );
}

/// Register object with the data store.
StatusCode TsDataSvc::registerObject( DataObject* parentObj, boost::string_ref objPath, DataObject* pObject )
{
  if ( !checkRoot() ) return INVALID_ROOT;
  if ( !parentObj ) {
    if ( !objPath.empty() ) {
      if ( objPath.front() != SEPARATOR ) {
        return registerObject( m_rootName.value(), objPath, pObject );
      }
      auto sep = find( objPath, SEPARATOR, 1 );
      if ( sep != boost::string_ref::npos ) {
        return registerObject( objPath.substr( 0, sep ), objPath.substr( sep ), pObject );
      }
    }
    return INVALID_OBJ_PATH;
  }
  RegEntry* node_entry = CAST_REGENTRY( RegEntry*, parentObj->registry() );
  if ( node_entry ) {
    StatusCode status = INVALID_PARENT;
    auto sep          = find( objPath, SEPARATOR, 1 );
    if ( sep != boost::string_ref::npos ) {
      auto p_path         = objPath.substr( 0, sep );
      auto o_path         = objPath.substr( sep );
      RegEntry* par_entry = node_entry->findLeaf( p_path );
      // Create default object leafs if the
      // intermediate nodes are not present
      if ( !par_entry && m_forceLeaves ) {
        DataObject* pLeaf = createDefaultObject();
        StatusCode sc     = registerObject( parentObj, p_path, pLeaf );
        if ( !sc.isSuccess() ) delete pLeaf;
        par_entry = node_entry->findLeaf( p_path );
      } else if ( 0 != par_entry && par_entry->object() == 0 ) {
        status = retrieveEntry( node_entry, p_path, par_entry );
        if ( !status.isSuccess() && !par_entry->address() && m_forceLeaves ) {
          DataObject* pLeaf = createDefaultObject();
          StatusCode sc     = registerObject( parentObj, p_path, pLeaf );
          if ( !sc.isSuccess() ) delete pLeaf;
          par_entry = node_entry->findLeaf( p_path );
        }
      }
      node_entry = par_entry;
      if ( node_entry ) {
        DataObject* obj   = node_entry->object();
        if ( obj ) status = registerObject( obj, o_path, pObject );
      }
    } else {
      RegEntry* leaf = node_entry->findLeaf( objPath );
      if ( !leaf ) {
        status = node_entry->add( to_string( objPath ), pObject );
      } else {
        DataObject* obj = leaf->object();
        if ( !obj ) {
          if ( !pObject ) {
            error() << "registerObject: trying to register null DataObject" << endmsg;
            return StatusCode::FAILURE;
          } else {
            pObject->setRegistry( leaf );
          }
          leaf->setAddress( nullptr );
          leaf->setObject( pObject );
          status = StatusCode::SUCCESS;
        } else {
          status = DOUBL_OBJ_PATH;
        }
      }
    }
    return status;
  }
  return INVALID_PARENT;
}

/// Unregister object from the data store.
StatusCode TsDataSvc::unregisterObject( boost::string_ref fullPath )
{
  DataObject* pObject = 0;
  StatusCode status   = findObject( fullPath, pObject );
  if ( status.isSuccess() ) {
    RegEntry* pEntry = CAST_REGENTRY( RegEntry*, pObject->registry() );
    if ( 0 != pEntry ) {
      if ( pEntry->isEmpty() ) {
        RegEntry* pParent = pEntry->parentEntry();
        if ( 0 != pParent ) {
          if ( 0 != pObject ) {
            pObject->addRef();
          }
          pParent->remove( pEntry );
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
StatusCode TsDataSvc::unregisterObject( boost::string_ref parentPath, boost::string_ref objPath )
{
  DataObject* pO    = 0;
  StatusCode status = findObject( parentPath, pO );
  if ( status.isSuccess() ) {
    status = unregisterObject( pO, objPath );
  }
  return status;
}

/// Unregister object from the data store.
StatusCode TsDataSvc::unregisterObject( boost::string_ref parentPath, int item )
{
  return unregisterObject( parentPath, itemToPath( item ) );
}

/// Unregister object from the data store.
StatusCode TsDataSvc::unregisterObject( DataObject* pObject )
{
  if ( checkRoot() ) {
    RegEntry* entry = m_root->findLeaf( pObject );
    if ( 0 != entry ) {
      RegEntry* parent = entry->parentEntry();
      if ( 0 != parent ) {
        if ( entry->isEmpty() ) {
          if ( 0 != entry->object() ) {
            entry->object()->addRef();
          }
          if ( 0 != parent ) {
            parent->remove( entry );
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
StatusCode TsDataSvc::unregisterObject( DataObject* pParentObj, boost::string_ref objectPath )
{
  if ( checkRoot() ) {
    try {
      RegEntry* parent = CAST_REGENTRY( RegEntry*, pParentObj->registry() );
      if ( 0 != parent ) {
        RegEntry* entry = parent->findLeaf( objectPath );
        if ( 0 != entry ) {
          if ( entry->isEmpty() ) {
            if ( 0 != entry->object() ) {
              entry->object()->addRef();
            }
            parent->remove( entry );
            return SUCCESS;
          }
          return DIR_NOT_EMPTY;
        }
        return INVALID_OBJECT;
      }
    } catch ( ... ) {
    }
    return INVALID_PARENT;
  }
  return INVALID_ROOT;
}

/// Unregister object from the data store.
StatusCode TsDataSvc::unregisterObject( DataObject* pParentObj, int item )
{
  return unregisterObject( pParentObj, itemToPath( item ) );
}

/** Invoke data fault handling if enabled
  */
DataObject* TsDataSvc::handleDataFault( IRegistry* pReg, boost::string_ref path )
{
  if ( m_enableFaultHdlr ) {
    IRegistry* pLeaf = nullptr;
    if ( pReg && path.empty() ) {
      DataIncident incident( name(), m_faultName, pReg->identifier() );
      m_incidentSvc->fireIncident( incident );
      return pReg->object();
    } else if ( pReg ) {
      std::string p = pReg->identifier();
      if ( path.front() != SEPARATOR ) p += SEPARATOR;
      p.append( path.data(), path.size() );
      DataIncident incident( name(), m_faultName, p );
      m_incidentSvc->fireIncident( incident );
      pLeaf = m_root->findLeaf( p );
    } else {
      std::string p = m_root->identifier();
      if ( path[0] != SEPARATOR ) p += SEPARATOR;
      p.append( path.data(), path.size() );
      DataIncident incident( name(), m_faultName, p );
      m_incidentSvc->fireIncident( incident );
      pLeaf = m_root->findLeaf( p );
    }
    if ( pLeaf ) return pLeaf->object();
  }
  return nullptr;
}

/** Invoke Persistency service to create transient object from its
 *  persistent representation
 */
StatusCode TsDataSvc::loadObject( IRegistry* pRegistry )
{
  IConversionSvc* pLoader = getDataLoader( pRegistry );
  return loadObject( pLoader, pRegistry );
}

/** Invoke Persistency service to create transient object from its
 *  persistent representation
 */
StatusCode TsDataSvc::loadObject( IConversionSvc* pLoader, IRegistry* pRegistry )
{
  STD_LOCK_GUARD_MACRO
  StatusCode status   = INVALID_OBJ_ADDR;
  DataObject* pObject = 0;
  if ( 0 == pLoader ) { // Precondition: Data loader must be present
    if ( handleDataFault( pRegistry ) != 0 )
      return SUCCESS;
    else
      return NO_DATA_LOADER;
  }
  if ( 0 == pRegistry ) { // Precondition: Directory must be valid
    if ( handleDataFault( pRegistry ) != 0 )
      return SUCCESS;
    else
      return INVALID_OBJ_ADDR;
  }

  VERMSG << "Requested object " << pRegistry->identifier() << endmsg;

  if ( m_enableAccessHdlr ) {
    // Fire data access incident
    DataIncident incident( name(), m_accessName, pRegistry->identifier() );
    m_incidentSvc->fireIncident( incident );
  }
  if ( m_inhibitPathes.size() > 0 ) {
    const auto& ident = pRegistry->identifier();
    auto inhibit      = std::find( m_inhibitPathes.begin(), m_inhibitPathes.end(), ident );
    if ( inhibit != m_inhibitPathes.end() ) {
      return NO_ACCESS;
    }
  }
  IOpaqueAddress* pAddress = pRegistry->address();
  if ( 0 == pAddress ) {     // Precondition:
    return INVALID_OBJ_ADDR; // Address must be valid
  }
  try {
    status = pLoader->createObj( pAddress, pObject ); // Call data loader
    if ( status.isSuccess() ) {

      VERMSG << "Object " << pRegistry->identifier() << " created" << endmsg;

      RegEntry* pEntry = CAST_REGENTRY( RegEntry*, pRegistry );
      pEntry->setObject( pObject );

      VERMSG << "Filling object " << pRegistry->identifier() << endmsg;
      status = pLoader->fillObjRefs( pAddress, pObject );
    }
  } catch ( const GaudiException& exc ) {
    if ( handleDataFault( pRegistry ) != 0 ) {
      return SUCCESS;
    }
    throw GaudiException( "GaudiException in loadObject() " + pRegistry->identifier(), name(), StatusCode::FAILURE,
                          exc );
  } catch ( const std::exception& x ) {
    if ( handleDataFault( pRegistry ) != 0 ) {
      return SUCCESS;
    }
    throw GaudiException( "std::exception in loadObject() " + pRegistry->identifier() + ": " +
                              System::typeinfoName( typeid( x ) ) + ", " + x.what(),
                          name(), StatusCode::FAILURE );
  } catch ( ... ) {
    if ( handleDataFault( pRegistry ) != 0 ) {
      return SUCCESS;
    }
    throw GaudiException( "UNKN exception in loadObject() " + pRegistry->identifier(), name(), StatusCode::FAILURE );
  }
  if ( !status.isSuccess() ) {
    if ( handleDataFault( pRegistry ) != 0 ) {
      return StatusCode::SUCCESS;
    }
  }
  ON_VERBOSE if ( status.isSuccess() )
  {
    verbose() << "Object " << pRegistry->identifier() << " successfully loaded" << endmsg;
  }
  return status;
}

/// Retrieve registry entry from store
StatusCode TsDataSvc::retrieveEntry( RegEntry* parentObj, boost::string_ref path, RegEntry*& pEntry )
{
  STD_LOCK_GUARD_MACRO
  auto sep          = find( path, SEPARATOR, 1 );
  StatusCode status = StatusCode( INVALID_ROOT, true );
  pEntry            = nullptr;
  // A.Valassi 16.08.2001 avoid core dump if store is empty
  if ( checkRoot() ) {
    if ( 0 == parentObj ) {
      if ( path.length() == 0 || path == m_rootName ) {
        return retrieveEntry( m_root, "", pEntry );
      } else if ( path[0] != SEPARATOR ) {
        return retrieveEntry( m_root, path, pEntry );
      } else if ( sep != std::string::npos ) {
        if ( m_root->object() == 0 ) {
          RegEntry* r = 0;
          status      = retrieveEntry( m_root, "", r );
          if ( !status.isSuccess() ) {
            return status;
          }
        }
        return retrieveEntry( m_root, path.substr( sep ), pEntry );
      }
      return INVALID_OBJ_PATH;
    }
    if ( sep != boost::string_ref::npos ) { // the string contains a separator (after pos 0)
      auto p_path = path.substr( 0, sep );
      auto o_path = path.substr( sep );
      if ( !parentObj->object() ) { // if the parent object has not been loaded yet, load it now
        status = loadObject( parentObj );
        if ( !status.isSuccess() ) {
          return status;
        }
      }
      RegEntry* root_entry = parentObj->findLeaf( p_path );
      if ( !root_entry && m_enableFaultHdlr ) {
        // If not even the parent is there, an incident
        // to load the parent must be fired...
        handleDataFault( parentObj, p_path );
        root_entry = parentObj->findLeaf( p_path );
      }
      if ( root_entry ) {
        DataObject* pO = root_entry->object();
        if ( 0 == pO ) {
          // Object is not loaded: load the object if at all possible
          status = loadObject( root_entry );
          if ( !status.isSuccess() ) {
            return status;
          }
        }
        if ( root_entry->isSoft() ) {
          root_entry = CAST_REGENTRY( RegEntry*, pO->registry() );
        }
        return retrieveEntry( root_entry, o_path, pEntry );
      }
      return status;
    } else if ( path.length() == 0 ) {
      pEntry = parentObj;
    } else {
      if ( !parentObj->object() ) { // if the parent object has not been loaded yet, load it now
        status = loadObject( parentObj );
        if ( !status.isSuccess() ) {
          return status;
        }
      }
      // last leave in search: find leaf and load
      pEntry = parentObj->findLeaf( path );
      // If no registry entry was found, trigger incident for action-on-demand
      if ( !pEntry && m_enableFaultHdlr ) {
        handleDataFault( parentObj, path );
        pEntry = ( 0 == path.length() ) ? parentObj : parentObj->findLeaf( path );
      }
    }
    // Check results and return
    if ( 0 == pEntry ) {
      status = INVALID_OBJ_PATH;
    } else if ( 0 == pEntry->object() ) {
      status = loadObject( pEntry );
    } else if ( m_enableAccessHdlr ) {
      // Fire data access incident
      // I do not know if this is a good idea....
      // This fires too often!
      //
      // DataIncident incident(name(), m_accessName, pEntry->identifier());
      // m_incidentSvc->fireIncident(incident);
      status = SUCCESS;
    } else {
      status = SUCCESS;
    }
  }
  return status;
}

/// Retrieve object identified by its directory from the data store.
StatusCode TsDataSvc::retrieveObject( IRegistry* pRegistry, boost::string_ref path, DataObject*& pObject )
{
  STD_LOCK_GUARD_MACRO
  pObject          = 0;
  RegEntry *result = 0, *parent = CAST_REGENTRY( RegEntry *, pRegistry );
  StatusCode status = retrieveEntry( parent, path, result );
  if ( status.isSuccess() ) {
    pObject = result->object();
  }
  return status;
}

/// Retrieve object identified by its full path from the data store.
StatusCode TsDataSvc::retrieveObject( boost::string_ref fullPath, DataObject*& pObject )
{
  IRegistry* nullDir = 0;
  return retrieveObject( nullDir, fullPath, pObject );
}

/// Retrieve object identified by its full path from the data store.
StatusCode TsDataSvc::retrieveObject( boost::string_ref parentPath, boost::string_ref objectPath, DataObject*& pObject )
{
  DataObject* parent = 0;
  StatusCode status  = retrieveObject( parentPath, parent );
  if ( status.isSuccess() ) {
    status = retrieveObject( parent, objectPath, pObject );
  }
  return status;
}

/// Retrieve object identified by its full path from the data store.
StatusCode TsDataSvc::retrieveObject( boost::string_ref parentPath, int item, DataObject*& pObject )
{
  return retrieveObject( parentPath, itemToPath( item ), pObject );
}

/// Retrieve object from data store.
StatusCode TsDataSvc::retrieveObject( DataObject* parentObj, boost::string_ref path, DataObject*& pObject )
{
  IRegistry* pRegistry = ( 0 == parentObj ) ? 0 : parentObj->registry();
  return retrieveObject( pRegistry, path, pObject );
}

/// Retrieve registry entry from store
StatusCode TsDataSvc::retrieveObject( DataObject* parentObj, int item, DataObject*& pObject )
{
  return retrieveObject( parentObj, itemToPath( item ), pObject );
}

/// Retrieve object identified by its directory from the data store.
StatusCode TsDataSvc::findObject( IRegistry* pRegistry, boost::string_ref path, DataObject*& pObject )
{
  STD_LOCK_GUARD_MACRO
  pObject              = 0;
  IRegistry* pReg      = ( 0 == pRegistry ) ? m_root : pRegistry;
  RegEntry* root_entry = CAST_REGENTRY( RegEntry*, pReg );
  if ( 0 != root_entry ) {
    if ( path.length() > 0 ) {
      pReg = root_entry->find( path );
    }
    if ( 0 == pReg ) {
      return INVALID_OBJ_PATH;
    }
    pObject = pReg->object();
  }
  return ( 0 == pObject ) ? OBJ_NOT_LOADED : IDataProviderSvc_NO_ERROR;
}

/// Find object identified by its full path in the data store.
StatusCode TsDataSvc::findObject( boost::string_ref path, DataObject*& pObject )
{
  STD_LOCK_GUARD_MACRO
  pObject = nullptr;
  if ( !checkRoot() ) return INVALID_ROOT;
  if ( path.empty() || path == m_rootName ) {
    pObject = m_root->object();
    return ( !pObject ) ? OBJ_NOT_LOADED : IDataProviderSvc_NO_ERROR;
  }
  if ( path.front() != SEPARATOR ) {
    return findObject( m_rootName.value(), path, pObject );
  }
  return findObject( static_cast<IRegistry*>( nullptr ), path, pObject );
}

/// Retrieve object identified by its full path from the data store.
StatusCode TsDataSvc::findObject( boost::string_ref parentPath, boost::string_ref objectPath, DataObject*& pObject )
{
  DataObject* parent = 0;
  StatusCode status  = findObject( parentPath, parent );
  if ( status.isSuccess() ) {
    status = findObject( parent, objectPath, pObject );
  }
  return status;
}

/// Retrieve object identified by its full path from the data store.
StatusCode TsDataSvc::findObject( boost::string_ref parentPath, int item, DataObject*& pObject )
{
  return findObject( parentPath, itemToPath( item ), pObject );
}

/// Find object identified by its full path in the data store.
StatusCode TsDataSvc::findObject( DataObject* parentObj, int item, DataObject*& pObject )
{
  return findObject( parentObj, itemToPath( item ), pObject );
}

/// Find object identified by its full path in the data store.
StatusCode TsDataSvc::findObject( DataObject* parentObj, boost::string_ref path, DataObject*& pObject )
{
  IRegistry* pDir = ( 0 == parentObj ) ? 0 : parentObj->registry();
  return findObject( pDir, path, pObject );
}

/// Update object.
StatusCode TsDataSvc::updateObject( boost::string_ref updatePath )
{
  DataObject* pO    = 0;
  StatusCode status = findObject( updatePath, pO );
  if ( status.isSuccess() ) {
    return updateObject( pO );
  }
  return retrieveObject( updatePath, pO );
}

/// Update object.
StatusCode TsDataSvc::updateObject( IRegistry* pRegistry )
{
  if ( 0 == pRegistry ) {    // Precondition:
    return INVALID_OBJ_ADDR; // Addres must be valid
  }
  DataObject* toUpdate = pRegistry->object();
  if ( 0 == toUpdate ) { // Try first to load
    return loadObject( pRegistry );
  }
  return updateObject( toUpdate );
}

/// Update object.
StatusCode TsDataSvc::updateObject( DataObject* toUpdate )
{
  STD_LOCK_GUARD_MACRO
  StatusCode status = INVALID_OBJ_ADDR;
  if ( 0 == toUpdate ) {   // Precondition:
    return INVALID_OBJECT; // Address must be valid
  }
  IRegistry* pRegistry = toUpdate->registry(); // Precondition:
  if ( 0 == pRegistry ) {                      // Need valid registry
    return INVALID_OBJECT;
  }
  IOpaqueAddress* pAddress = pRegistry->address(); // Precondition:
  if ( 0 == pAddress ) {                           // Need valid address
    return INVALID_OBJ_ADDR;
  }
  IConversionSvc* pLoader = getDataLoader( pRegistry );
  if ( 0 == pLoader ) {    // Precondition:
    return NO_DATA_LOADER; // Data loader must be present
  }
  if ( m_inhibitPathes.size() > 0 ) {
    auto& ident  = pRegistry->identifier();
    auto inhibit = std::find( m_inhibitPathes.begin(), m_inhibitPathes.end(), ident );
    if ( inhibit != m_inhibitPathes.end() ) {
      return NO_ACCESS;
    }
  }
  try {
    status = pLoader->updateObj( pAddress, toUpdate ); // Call data loader
    if ( status.isSuccess() ) {
      status = pLoader->updateObjRefs( pAddress, toUpdate );
    }
  } catch ( const GaudiException& exc ) {
    throw GaudiException( "GaudiException in updateObject() " + pRegistry->name(), name(), StatusCode::FAILURE, exc );
  } catch ( const std::exception& x ) {
    throw GaudiException( "std::exception in updateObject() " + pRegistry->name() + ": " +
                              System::typeinfoName( typeid( x ) ) + ", " + x.what(),
                          name(), StatusCode::FAILURE );
  } catch ( ... ) {
    throw GaudiException( "UNKN exception in updateObject() " + pRegistry->name(), name(), StatusCode::FAILURE );
  }
  return status;
}

/// Update object.
StatusCode TsDataSvc::updateObject( boost::string_ref parentPath, boost::string_ref updatePath )
{
  DataObject* pParent = 0;
  StatusCode status   = findObject( parentPath, pParent );
  if ( status.isSuccess() ) {
    status = updateObject( pParent, updatePath );
  }
  return status;
}

/// Update object.
StatusCode TsDataSvc::updateObject( DataObject* parent, boost::string_ref updatePath )
{
  DataObject* pObject = 0;
  StatusCode status   = findObject( parent, updatePath, pObject );
  if ( status.isSuccess() ) {
    status = updateObject( pObject );
  }
  return status;
}

// Link object
StatusCode TsDataSvc::linkObject( IRegistry* from, boost::string_ref objPath, DataObject* to )
{
  STD_LOCK_GUARD_MACRO
  if ( checkRoot() ) {
    try {
      RegEntry* from_entry = CAST_REGENTRY( RegEntry*, from );
      if ( 0 != from_entry ) {
        // First check if both objects are already registered to the store
        RegEntry* to_entry = m_root->findLeaf( to );
        if ( !to_entry ) return INVALID_OBJECT;
        auto sep = objPath.rfind( SEPARATOR );
        if ( sep > 0 && sep != std::string::npos ) { // in case the objPath is a sub-directory itself
          DataObject* pO = nullptr;
          StatusCode sc  = retrieveObject( from, objPath.substr( 0, sep ), pO );
          if ( sc.isSuccess() ) {
            sc = linkObject( pO->registry(), objPath.substr( sep ), to );
          }
          return sc;
        }
        // Now register the soft link
        StatusCode status = from_entry->add( to_string( objPath ), to, true );
        return status.isSuccess() ? IDataProviderSvc_NO_ERROR : DOUBL_OBJ_PATH;
      }
    } catch ( ... ) {
    }
    return INVALID_PARENT;
  }
  return INVALID_ROOT;
}

/// Add a link to another object.
StatusCode TsDataSvc::linkObject( boost::string_ref fullPath, DataObject* to )
{
  STD_LOCK_GUARD_MACRO
  if ( fullPath.length() > 0 ) {
    if ( fullPath[0] != SEPARATOR ) {
      return linkObject( m_rootName.value(), fullPath, to );
    }
    auto sep = fullPath.rfind( SEPARATOR );
    return linkObject( fullPath.substr( 0, sep ), fullPath.substr( sep ), to );
  }
  return INVALID_OBJ_PATH;
}

/// Add a link to another object.
StatusCode TsDataSvc::linkObject( boost::string_ref from, boost::string_ref objPath, DataObject* to )
{
  STD_LOCK_GUARD_MACRO
  DataObject* pO    = 0;
  StatusCode status = retrieveObject( from, pO );
  if ( status.isSuccess() ) {
    return linkObject( pO->registry(), objPath, to );
  }
  return status;
}

/// Add a link to another object.
StatusCode TsDataSvc::linkObject( DataObject* from, boost::string_ref objPath, DataObject* to )
{
  STD_LOCK_GUARD_MACRO
  if ( 0 != from ) {
    IRegistry* from_entry = from->registry();
    if ( 0 != from_entry ) {
      return linkObject( from_entry, objPath, to );
    }
  }
  return INVALID_PARENT;
}

/// Remove a link to another object.
StatusCode TsDataSvc::unlinkObject( IRegistry* from, boost::string_ref objPath )
{
  STD_LOCK_GUARD_MACRO
  if ( checkRoot() ) {
    try {
      RegEntry* from_entry = CAST_REGENTRY( RegEntry*, from );
      if ( 0 != from_entry ) {
        std::string::size_type sep = objPath.rfind( SEPARATOR );
        if ( sep > 0 && sep != std::string::npos ) { // in case the objPath is a sub-directory itself
          DataObject* pO = nullptr;
          StatusCode sc  = findObject( from, objPath.substr( 0, sep ), pO );
          if ( sc.isSuccess() ) {
            sc = unlinkObject( pO->registry(), objPath.substr( sep ) );
          }
          return sc;
        }
        StatusCode status = from_entry->remove( objPath );
        if ( status.isSuccess() ) {
          return status;
        }
        return INVALID_OBJ_PATH;
      }
    } catch ( ... ) {
    }
    return INVALID_PARENT;
  }
  return INVALID_ROOT;
}

/// Remove a link to another object.
StatusCode TsDataSvc::unlinkObject( boost::string_ref fullPath )
{
  if ( fullPath.empty() ) return INVALID_OBJ_PATH;
  if ( fullPath.front() != SEPARATOR ) {
    return unlinkObject( m_rootName.value(), fullPath );
  }
  auto sep = fullPath.rfind( SEPARATOR );
  return unlinkObject( fullPath.substr( 0, sep ), fullPath.substr( sep ) );
}

/// Remove a link to another object.
StatusCode TsDataSvc::unlinkObject( boost::string_ref from, boost::string_ref objPath )
{
  DataObject* pObject = nullptr;
  StatusCode status   = findObject( from, pObject );
  if ( status.isSuccess() ) {
    status = unlinkObject( pObject->registry(), objPath );
  }
  return status;
}

/// Remove a link to another object.
StatusCode TsDataSvc::unlinkObject( DataObject* from, boost::string_ref objPath )
{
  if ( checkRoot() ) {
    IRegistry* from_entry = m_root->findLeaf( from );
    return unlinkObject( from_entry, objPath );
  }
  return INVALID_ROOT;
}

/// Add an item to the preload list
StatusCode TsDataSvc::addPreLoadItem( const DataStoreItem& item )
{
  LoadItems::iterator i = std::find( m_preLoads.begin(), m_preLoads.end(), item );
  if ( i == m_preLoads.end() ) {
    m_preLoads.push_back( item );
  }
  return StatusCode::SUCCESS;
}

/// Add an item to the preload list
StatusCode TsDataSvc::addPreLoadItem( std::string itemPath )
{
  return addPreLoadItem( DataStoreItem( std::move( itemPath ), 1 ) );
}

/// Remove an item from the preload list
StatusCode TsDataSvc::removePreLoadItem( const DataStoreItem& item )
{
  LoadItems::iterator i = std::remove( m_preLoads.begin(), m_preLoads.end(), item );
  if ( i != m_preLoads.end() ) {
    m_preLoads.erase( i, m_preLoads.end() );
  }
  return StatusCode::SUCCESS;
}

/// Add an item to the preload list
StatusCode TsDataSvc::removePreLoadItem( std::string itemPath )
{
  return removePreLoadItem( DataStoreItem( std::move( itemPath ), 1 ) );
}

/// Clear the preload list
StatusCode TsDataSvc::resetPreLoad()
{
  m_preLoads.erase( m_preLoads.begin(), m_preLoads.end() );
  return StatusCode::SUCCESS;
}

/// Preload one level deep, then recursively call the next level
StatusCode TsDataSvc::preLoad( int depth, int load_depth, DataObject* pObject )
{
  // unused:  StatusCode sc = StatusCode::FAILURE;
  if ( 0 != pObject && depth++ < load_depth ) {
    RegEntry* dir = CAST_REGENTRY( RegEntry*, pObject->registry() );
    if ( 0 != dir ) {
      for ( RegEntry::Iterator i = dir->begin(); i != dir->end(); i++ ) {
        DataObject* pObj  = 0;
        StatusCode status = retrieveObject( pObject, ( *i )->name(), pObj );
        if ( status.isSuccess() && depth < load_depth ) {
          preLoad( depth, load_depth, pObj ).ignore();
        }
      }
    }
  }
  return StatusCode::SUCCESS;
}

/// load all preload items of the list
StatusCode TsDataSvc::preLoad()
{
  DataObject* pObj = 0;
  for ( LoadItems::iterator i = m_preLoads.begin(); i != m_preLoads.end(); i++ ) {
    StatusCode sc  = retrieveObject( ( *i ).path(), pObj );
    int load_depth = ( *i ).depth();
    if ( sc.isSuccess() && load_depth > 1 ) {
      preLoad( 1, load_depth, pObj ).ignore();
    }
  }
  return StatusCode::SUCCESS;
}

/// Service initialization
StatusCode TsDataSvc::initialize()
{
  // Nothing to do: just call base class initialisation
  StatusCode sc = Service::initialize();
  if ( !sc.isSuccess() ) {
    return sc;
  }
  sc = service( "IncidentSvc", m_incidentSvc, true );
  if ( UNLIKELY( !sc.isSuccess() ) ) {
    error() << "Failed to access incident service." << endmsg;
  }
  return sc;
}

/// Service reinitialization
StatusCode TsDataSvc::reinitialize()
{
  StatusCode sc;
  // the finalize part is copied here
  setDataLoader( 0 ).ignore();
  resetPreLoad().ignore();
  clearStore().ignore();
  if ( m_incidentSvc ) {
    m_incidentSvc->release();
    m_incidentSvc = 0;
  }
  // re-initialize the base class
  sc = Service::reinitialize();
  if ( UNLIKELY( !sc.isSuccess() ) ) {
    error() << "Unable to reinitialize base class" << endmsg;
    return sc;
  }
  // the initialize part is copied here
  sc = service( "IncidentSvc", m_incidentSvc, true );
  if ( UNLIKELY( !sc.isSuccess() ) ) {
    error() << "Failed to access incident service." << endmsg;
    return sc;
  }
  // return
  return StatusCode::SUCCESS;
}

/// Service initialisation
StatusCode TsDataSvc::finalize()
{
  // Nothing to do: just call base class initialisation
  setDataLoader( 0 ).ignore();
  resetPreLoad().ignore();
  clearStore().ignore();
  if ( m_incidentSvc ) {
    m_incidentSvc->release();
    m_incidentSvc = 0;
  }
  return Service::finalize();
}

/// CLID for root Event
CLID TsDataSvc::rootCLID() const { return ( (CLID)m_rootCLID ); }

/// Name for root Event
const std::string& TsDataSvc::rootName() const { return ( m_rootName ); }

/// Create default objects in case forced creation of leaves is requested
DataObject* TsDataSvc::createDefaultObject() const { return new DataObject(); }

/** Retrieve customizable data loader according to registry entry to
 *  be retrieved
 */
IConversionSvc* TsDataSvc::getDataLoader( IRegistry* /* pReg */ ) { return m_dataLoader; }

/// Standard Destructor
TsDataSvc::~TsDataSvc()
{
  setDataLoader( 0 ).ignore();
  resetPreLoad().ignore();
  clearStore().ignore();
}
