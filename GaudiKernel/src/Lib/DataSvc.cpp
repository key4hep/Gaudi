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
#define DATASVC_DATASVC_CPP

// Framework include files
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/IOpaqueAddress.h"

#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/GaudiException.h"

#include "GaudiKernel/DataIncident.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/RegistryEntry.h"

// Include files
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <sstream>
#include <vector>

namespace {
  std::string operator+( char c, std::string_view sr ) {
    std::string s{c};
    s.append( sr.data(), sr.size() );
    return s;
  }

  std::string_view::size_type find( std::string_view s, char c, size_t o ) {
    if ( !s.empty() ) s.remove_prefix( o );
    auto r = s.find( c );
    return r == std::string_view::npos ? r : ( r + o );
  }

  std::string to_string( std::string_view sr ) { return {sr.data(), sr.size()}; }
} // namespace

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
StatusCode DataSvc::clearSubTree( std::string_view sub_tree_path ) {
  DataObject* pObject = nullptr;
  StatusCode  status  = findObject( sub_tree_path, pObject );
  if ( !status.isSuccess() ) return status;
  RegEntry* node_entry = CAST_REGENTRY( RegEntry*, pObject->registry() );
  if ( !node_entry ) return Status::INVALID_OBJECT;
  RegEntry* parent = node_entry->parentEntry();
  if ( !parent ) return Status::INVALID_PARENT;
  parent->remove( node_entry );
  return StatusCode::SUCCESS;
}

/** IDataManagerSvc: Remove all data objects below the sub tree
 *  identified by the object.
 */
StatusCode DataSvc::clearSubTree( DataObject* pObject ) {
  if ( !checkRoot() ) return Status::INVALID_ROOT;
  RegEntry* entry = CAST_REGENTRY( RegEntry*, pObject->registry() );
  if ( !entry ) return Status::INVALID_OBJECT;
  RegEntry* parent = entry->parentEntry();
  if ( !parent ) return Status::INVALID_PARENT;
  parent->remove( entry );
  return StatusCode::SUCCESS;
}

/// IDataManagerSvc: Remove all data objects in the data store.
StatusCode DataSvc::clearStore() {
  if ( !checkRoot() ) return Status::INVALID_ROOT;
  m_root.reset();
  return StatusCode::SUCCESS;
}

/** IDataManagerSvc: Analyse by traversing all data objects below the
 * sub tree identified by its full path name.
 */
StatusCode DataSvc::traverseSubTree( std::string_view sub_tree_path, IDataStoreAgent* pAgent ) {
  DataObject* pO     = nullptr;
  StatusCode  status = findObject( sub_tree_path, pO );
  return status.isSuccess() ? traverseSubTree( pO, pAgent ) : status;
}

/// IDataManagerSvc: Analyse by traversing all data objects below the sub tree
StatusCode DataSvc::traverseSubTree( DataObject* pObject, IDataStoreAgent* pAgent ) {
  RegEntry* entry = CAST_REGENTRY( RegEntry*, pObject->registry() );
  return entry ? entry->traverseTree( pAgent ) : Status::INVALID_OBJECT;
}

/// IDataManagerSvc: Analyse by traversing all data objects in the data store.
StatusCode DataSvc::traverseTree( IDataStoreAgent* pAgent ) {
  if ( !checkRoot() ) return Status::INVALID_ROOT;
  return m_root->traverseTree( pAgent );
}

/**
 * Initialize data store for new event by giving new event path and root
 * object. Takes care to clear the store before reinitializing it
 */
StatusCode DataSvc::setRoot( std::string root_path, DataObject* pRootObj ) {
  clearStore().ignore();
  return i_setRoot( std::move( root_path ), pRootObj );
}

/**
 * Initialize data store for new event by giving new event path and root
 * object. Does not clear the store before reinitializing it. This could
 * lead to errors and should be handle with care. Use setRoot if unsure
 */
StatusCode DataSvc::i_setRoot( std::string root_path, DataObject* pRootObj ) {
  if ( pRootObj ) {
    m_root = std::make_unique<RegEntry>( std::move( root_path ) );
    m_root->makeHard( pRootObj );
    m_root->setDataSvc( this );
    // No done with GaudiHive. preLoad().ignore();
  }
  return StatusCode::SUCCESS;
}

/**
 * Initialize data store for new event by giving new event path and address
 * of root object. Takes care to clear the store before reinitializing it
 */
StatusCode DataSvc::setRoot( std::string root_path, IOpaqueAddress* pRootAddr ) {
  clearStore().ignore();
  return i_setRoot( std::move( root_path ), pRootAddr );
}

/**
 * Initialize data store for new event by giving new event path and address
 * of root object. Does not clear the store before reinitializing it. This
 * could lead to errors and should be handle with care. Use setRoot if unsure
 */
StatusCode DataSvc::i_setRoot( std::string root_path, IOpaqueAddress* pRootAddr ) {
  if ( pRootAddr ) {
    m_root = std::make_unique<RegEntry>( std::move( root_path ) );
    m_root->makeHard( pRootAddr );
    m_root->setDataSvc( this );
    // Not done with GaudiHive. preLoad().ignore();
  }
  return StatusCode::SUCCESS;
}

/// IDataManagerSvc: Pass a default data loader to the service.
StatusCode DataSvc::setDataLoader( IConversionSvc* pDataLoader, IDataProviderSvc* dpsvc ) {
  m_dataLoader = pDataLoader;
  if ( m_dataLoader ) m_dataLoader->setDataProvider( dpsvc ? dpsvc : this ).ignore();
  return StatusCode::SUCCESS;
}

/// IDataManagerSvc: Explore the object store: retrieve the object's parent
StatusCode DataSvc::objectParent( const DataObject* pObject, IRegistry*& refpParent ) {
  if ( !pObject ) return Status::INVALID_OBJECT;
  return objectParent( pObject->registry(), refpParent );
}
/// IDataManagerSvc: Explore the object store: retrieve the object's parent
StatusCode DataSvc::objectParent( const IRegistry* pRegistry, IRegistry*& refpParent ) {
  if ( !checkRoot() ) return Status::INVALID_ROOT;
  const RegEntry* node_entry = CAST_REGENTRY( const RegEntry*, pRegistry );
  if ( !node_entry ) return Status::INVALID_OBJECT;
  refpParent = node_entry->parent();
  return StatusCode::SUCCESS;
}

/// IDataManagerSvc: Explore an object identified by its pointer.
StatusCode DataSvc::objectLeaves( const DataObject* pObject, std::vector<IRegistry*>& leaves ) {
  if ( !pObject ) return Status::INVALID_OBJECT;
  return objectLeaves( pObject->registry(), leaves );
}

/** IDataManagerSvc: Explore an object identified by the pointer to the
 * registry entry.
 */
StatusCode DataSvc::objectLeaves( const IRegistry* pRegistry, std::vector<IRegistry*>& leaves ) {
  if ( !checkRoot() ) return Status::INVALID_ROOT;
  const RegEntry* node_entry = CAST_REGENTRY( const RegEntry*, pRegistry );
  if ( !node_entry ) return Status::INVALID_OBJECT;
  leaves = node_entry->leaves();
  return StatusCode::SUCCESS;
}

///  IDataManagerSvc: Register object address with the data store.
StatusCode DataSvc::registerAddress( std::string_view fullPath, IOpaqueAddress* pAddress ) {
  if ( fullPath.empty() ) return Status::INVALID_OBJ_PATH;
  return registerAddress( fullPath.front() != SEPARATOR ? m_root.get() : nullptr, fullPath, pAddress );
}

///  IDataManagerSvc: Register object address with the data store.
StatusCode DataSvc::registerAddress( IRegistry* parentObj, std::string_view objPath, IOpaqueAddress* pAddress ) {
  if ( !checkRoot() ) return Status::INVALID_ROOT;
  if ( objPath.empty() ) return Status::INVALID_OBJ_PATH;

  if ( !parentObj ) {
    if ( objPath.front() != SEPARATOR ) { return registerAddress( m_root.get(), objPath, pAddress ); }
    auto sep = find( objPath, SEPARATOR, 1 );
    if ( sep == std::string_view::npos || objPath.substr( 0, sep ) != m_rootName.value() ) {
      return Status::INVALID_PARENT;
    }
    return registerAddress( m_root.get(), objPath.substr( sep ), pAddress );
  }
  if ( objPath.front() != SEPARATOR ) { return registerAddress( parentObj, char( SEPARATOR ) + objPath, pAddress ); }
  RegEntry* par_entry = CAST_REGENTRY( RegEntry*, parentObj );
  if ( !par_entry ) return Status::INVALID_PARENT;

  auto sep = objPath.rfind( SEPARATOR );
  if ( sep > 0 && sep != std::string_view::npos ) {
    auto      p_path  = objPath.substr( 0, sep );
    auto      o_path  = objPath.substr( sep );
    RegEntry* p_entry = par_entry->findLeaf( p_path );
    // Create default object leafs if the
    // intermediate nodes are not present
    if ( !p_entry && m_forceLeaves ) {
      DataObject* pLeaf = createDefaultObject();
      StatusCode  sc    = registerObject( par_entry->identifier(), p_path, pLeaf );
      if ( sc.isFailure() ) delete pLeaf;
      p_entry = par_entry->findLeaf( p_path );
    }
    if ( !p_entry ) return Status::INVALID_PARENT;
    return registerAddress( p_entry, o_path, pAddress );
  }
  StatusCode status = par_entry->add( to_string( objPath ), pAddress );
  return status.isSuccess() ? status : Status::DOUBL_OBJ_PATH;
}

///  IDataManagerSvc: Unregister object address from the data store.
StatusCode DataSvc::unregisterAddress( std::string_view fullPath ) {
  if ( fullPath.empty() ) return Status::INVALID_OBJ_PATH;
  return unregisterAddress( fullPath.front() != SEPARATOR ? m_root.get() : nullptr, fullPath );
}

///  IDataManagerSvc: Unregister object address from the data store.
StatusCode DataSvc::unregisterAddress( IRegistry* pParent, std::string_view objPath ) {
  if ( !checkRoot() ) return Status::INVALID_ROOT;
  if ( objPath.empty() ) return Status::INVALID_OBJ_PATH;

  if ( !pParent ) {
    if ( objPath.front() != SEPARATOR ) return unregisterAddress( m_root.get(), objPath );
    auto sep = find( objPath, SEPARATOR, 1 );
    if ( sep != std::string_view::npos && objPath.substr( 0, sep ) == m_rootName.value() ) {
      return unregisterAddress( m_root.get(), objPath.substr( sep ) );
    }
    return Status::INVALID_PARENT;
  }
  if ( objPath.front() != SEPARATOR ) { return unregisterAddress( pParent, char( SEPARATOR ) + objPath ); }
  RegEntry* node_entry = CAST_REGENTRY( RegEntry*, pParent );
  if ( node_entry ) {
    RegEntry* leaf_entry = node_entry->findLeaf( objPath );
    if ( leaf_entry ) {
      auto sep = objPath.rfind( SEPARATOR );
      if ( sep > 0 && sep != std::string_view::npos ) {
        return unregisterAddress( leaf_entry->parent(), objPath.substr( sep ) );
      }
      StatusCode status = node_entry->remove( objPath );
      if ( status.isSuccess() ) return status;
    }
  }
  return Status::INVALID_PARENT;
}

/// Register object with the data store.
StatusCode DataSvc::registerObject( std::string_view parentPath, std::string_view objPath, DataObject* pObject ) {
  DataObject* pO     = nullptr;
  StatusCode  status = retrieveObject( parentPath, pO );
  if ( !status.isSuccess() && m_forceLeaves ) {
    pO     = createDefaultObject();
    status = registerObject( parentPath, pO );
    if ( !status.isSuccess() ) pO->release();
  }
  return status.isSuccess() ? registerObject( pO, objPath, pObject ) : status;
}

/// Register object with the data store.
StatusCode DataSvc::registerObject( DataObject* parentObj, std::string_view objPath, DataObject* pObject ) {
  if ( !checkRoot() ) return Status::INVALID_ROOT;

  if ( !parentObj ) {
    if ( !objPath.empty() ) {
      if ( objPath.front() != SEPARATOR ) { return registerObject( m_rootName.value(), objPath, pObject ); }
      auto sep = find( objPath, SEPARATOR, 1 );
      if ( sep != std::string_view::npos ) {
        return registerObject( objPath.substr( 0, sep ), objPath.substr( sep ), pObject );
      }
    }
    return Status::INVALID_OBJ_PATH;
  }
  RegEntry* node_entry = CAST_REGENTRY( RegEntry*, parentObj->registry() );
  if ( node_entry ) {
    StatusCode status = Status::INVALID_PARENT;
    auto       sep    = find( objPath, SEPARATOR, 1 );
    if ( sep != std::string_view::npos ) {
      auto      p_path    = objPath.substr( 0, sep );
      auto      o_path    = objPath.substr( sep );
      RegEntry* par_entry = node_entry->findLeaf( p_path );
      // Create default object leafs if the
      // intermediate nodes are not present
      if ( !par_entry && m_forceLeaves ) {
        DataObject* pLeaf = createDefaultObject();
        StatusCode  sc    = registerObject( parentObj, p_path, pLeaf );
        if ( !sc.isSuccess() ) delete pLeaf;
        par_entry = node_entry->findLeaf( p_path );
      } else if ( par_entry && !par_entry->object() ) {
        status = i_retrieveEntry( node_entry, p_path, par_entry );
        if ( !status.isSuccess() && !par_entry->address() && m_forceLeaves ) {
          DataObject* pLeaf = createDefaultObject();
          StatusCode  sc    = registerObject( parentObj, p_path, pLeaf );
          if ( !sc.isSuccess() ) delete pLeaf;
          par_entry = node_entry->findLeaf( p_path );
        }
      }
      node_entry = par_entry;
      if ( node_entry ) {
        DataObject* obj = node_entry->object();
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
          }
          pObject->setRegistry( leaf );
          leaf->setAddress( nullptr );
          leaf->setObject( pObject );
          status = StatusCode::SUCCESS;
        } else {
          status = Status::DOUBL_OBJ_PATH;
        }
      }
    }
    return status;
  }
  return Status::INVALID_PARENT;
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject( std::string_view fullPath ) {
  DataObject* pObject = nullptr;
  StatusCode  status  = findObject( fullPath, pObject );
  if ( status.isFailure() ) return status;
  RegEntry* pEntry = CAST_REGENTRY( RegEntry*, pObject->registry() );
  if ( !pEntry ) return Status::INVALID_ROOT;
  if ( !pEntry->isEmpty() ) return Status::DIR_NOT_EMPTY;
  RegEntry* pParent = pEntry->parentEntry();
  if ( !pParent ) return Status::INVALID_PARENT;
  if ( pObject ) pObject->addRef();
  pParent->remove( pEntry );
  return StatusCode::SUCCESS;
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject( DataObject* pObject ) {
  if ( !checkRoot() ) return Status::INVALID_ROOT;
  RegEntry* entry = m_root->findLeaf( pObject );
  if ( !entry ) return Status::INVALID_OBJECT;
  RegEntry* parent = entry->parentEntry();
  if ( !parent ) return Status::INVALID_PARENT;
  if ( !entry->isEmpty() ) return Status::DIR_NOT_EMPTY;
  if ( entry->object() ) entry->object()->addRef();
  if ( parent ) parent->remove( entry );
  return StatusCode::SUCCESS;
}

/// Unregister object from the data store.
StatusCode DataSvc::unregisterObject( DataObject* pParentObj, std::string_view objectPath ) {
  if ( checkRoot() ) {
    try {
      RegEntry* parent = CAST_REGENTRY( RegEntry*, pParentObj->registry() );
      if ( parent ) {
        RegEntry* entry = parent->findLeaf( objectPath );
        if ( entry ) {
          if ( entry->isEmpty() ) {
            if ( entry->object() ) { entry->object()->addRef(); }
            parent->remove( entry );
            return StatusCode::SUCCESS;
          }
          return Status::DIR_NOT_EMPTY;
        }
        return Status::INVALID_OBJECT;
      }
    } catch ( ... ) {}
    return Status::INVALID_PARENT;
  }
  return Status::INVALID_ROOT;
}

/** Invoke data fault handling if enabled
 */
DataObject* DataSvc::handleDataFault( IRegistry* pReg, std::string_view path ) {
  return i_handleDataFault( pReg, path );
}

DataObject* DataSvc::i_handleDataFault( IRegistry* pReg, std::string_view path ) {
  if ( m_enableFaultHdlr ) {
    IRegistry* pLeaf = nullptr;
    if ( pReg && path.empty() ) {
      DataIncident incident( name(), m_faultName, pReg->identifier() );
      m_incidentSvc->fireIncident( incident );
      return pReg->object();
    }
    if ( pReg ) {
      std::string p = pReg->identifier();
      if ( path.front() != SEPARATOR ) p += SEPARATOR;
      p.append( path.data(), path.size() );
      DataIncident incident( name(), m_faultName, p );
      m_incidentSvc->fireIncident( incident );
      pLeaf = m_root->findLeaf( p );
    } else {
      std::string p = m_root->identifier();
      if ( path.front() != SEPARATOR ) p += SEPARATOR;
      p.append( path.data(), path.size() );
      DataIncident incident( name(), m_faultName, p );
      m_incidentSvc->fireIncident( incident );
      pLeaf = m_root->findLeaf( p );
    }
    if ( pLeaf ) { return pLeaf->object(); }
  }
  return nullptr;
}

/** Invoke Persistency service to create transient object from its
 *  persistent representation
 */
StatusCode DataSvc::loadObject( IRegistry* pRegistry ) {
  IConversionSvc* pLoader = getDataLoader( pRegistry );
  return loadObject( pLoader, pRegistry );
}

/** Invoke Persistency service to create transient object from its
 *  persistent representation
 */
StatusCode DataSvc::loadObject( IConversionSvc* pLoader, IRegistry* pRegistry ) {
  StatusCode  status  = Status::INVALID_OBJ_ADDR;
  DataObject* pObject = nullptr;
  if ( !pLoader ) { // Precondition: Data loader must be present
    return handleDataFault( pRegistry ) ? StatusCode::SUCCESS : StatusCode( Status::NO_DATA_LOADER );
  }
  if ( !pRegistry ) { // Precondition: Directory must be valid
    return handleDataFault( pRegistry ) ? StatusCode::SUCCESS : StatusCode( Status::INVALID_OBJ_ADDR );
  }

  VERMSG << "Requested object " << pRegistry->identifier() << endmsg;

  if ( m_enableAccessHdlr ) {
    // Fire data access incident
    DataIncident incident( name(), m_accessName, pRegistry->identifier() );
    m_incidentSvc->fireIncident( incident );
  }
  if ( !m_inhibitPathes.empty() ) {
    auto inhibit = std::find( m_inhibitPathes.begin(), m_inhibitPathes.end(), pRegistry->identifier() );
    if ( inhibit != m_inhibitPathes.end() ) { return Status::NO_ACCESS; }
  }
  IOpaqueAddress* pAddress = pRegistry->address();
  if ( !pAddress ) {                 // Precondition:
    return Status::INVALID_OBJ_ADDR; // Address must be valid
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
    if ( handleDataFault( pRegistry ) ) { return StatusCode::SUCCESS; }
    throw GaudiException( "GaudiException in loadObject() " + pRegistry->identifier(), name(), StatusCode::FAILURE,
                          exc );
  } catch ( const std::exception& x ) {
    if ( handleDataFault( pRegistry ) ) { return StatusCode::SUCCESS; }
    throw GaudiException( "std::exception in loadObject() " + pRegistry->identifier() + ": " +
                              System::typeinfoName( typeid( x ) ) + ", " + x.what(),
                          name(), StatusCode::FAILURE );
  } catch ( ... ) {
    if ( handleDataFault( pRegistry ) ) { return StatusCode::SUCCESS; }
    throw GaudiException( "UNKN exception in loadObject() " + pRegistry->identifier(), name(), StatusCode::FAILURE );
  }
  if ( !status.isSuccess() ) {
    if ( handleDataFault( pRegistry ) ) { return StatusCode::SUCCESS; }
  }
  ON_VERBOSE if ( status.isSuccess() ) {
    verbose() << "Object " << pRegistry->identifier() << " successfully loaded" << endmsg;
  }
  return status;
}

/// Retrieve registry entry from store
StatusCode DataSvc::retrieveEntry( RegEntry* parentObj, std::string_view path, RegEntry*& pEntry ) {
  return i_retrieveEntry( parentObj, path, pEntry );
}

StatusCode DataSvc::i_retrieveEntry( RegEntry* parentObj, std::string_view path, RegEntry*& pEntry ) {
  // A.Valassi 16.08.2001 avoid core dump if store is empty
  if ( !checkRoot() ) return StatusCode( Status::INVALID_ROOT, true );

  static constexpr auto empty = std::string_view{};
  auto                  sep   = find( path, SEPARATOR, 1 );
  pEntry                      = nullptr;

  if ( !parentObj ) {
    if ( path.empty() || path == m_rootName.value() ) {
      parentObj = m_root.get();
      path      = empty;
    } else if ( path.front() != SEPARATOR ) {
      parentObj = m_root.get();
    } else if ( sep != std::string_view::npos ) {
      if ( !m_root->object() ) {
        RegEntry* r      = nullptr;
        auto      status = i_retrieveEntry( m_root.get(), empty, r );
        if ( !status.isSuccess() ) return status;
      }
      parentObj = m_root.get();
      path      = path.substr( sep );
    } else {
      return Status::INVALID_OBJ_PATH;
    }
    sep = find( path, SEPARATOR, 1 );
  }

  StatusCode status = StatusCode( Status::INVALID_ROOT, true );
  if ( sep != std::string_view::npos ) { // the string contains a separator (after pos 0)
    if ( !parentObj->object() ) {        // if the parent object has not been loaded yet, load it now
      status = loadObject( parentObj );
      if ( !status.isSuccess() ) return status;
    }
    auto      p_path     = path.substr( 0, sep );
    RegEntry* root_entry = parentObj->findLeaf( p_path );
    if ( !root_entry && m_enableFaultHdlr ) {
      // If not even the parent is there, an incident
      // to load the parent must be fired...
      i_handleDataFault( parentObj, p_path );
      root_entry = parentObj->findLeaf( p_path );
    }
    if ( root_entry ) {
      DataObject* pO = root_entry->object();
      if ( !pO ) {
        // Object is not loaded: load the object if at all possible
        status = loadObject( root_entry );
        if ( !status.isSuccess() ) return status;
      }
      if ( root_entry->isSoft() ) { root_entry = CAST_REGENTRY( RegEntry*, pO->registry() ); }
      return i_retrieveEntry( root_entry, path.substr( sep ), pEntry );
    }
    return status;
  } else if ( path.empty() ) {
    pEntry = parentObj;
  } else {
    if ( !parentObj->object() ) { // if the parent object has not been loaded yet, load it now
      status = loadObject( parentObj );
      if ( !status.isSuccess() ) return status;
    }
    // last leave in search: find leaf and load
    pEntry = parentObj->findLeaf( path );
    // If no registry entry was found, trigger incident for action-on-demand
    if ( !pEntry && m_enableFaultHdlr ) {
      i_handleDataFault( parentObj, path );
      pEntry = ( path.empty() ? parentObj : parentObj->findLeaf( path ) );
    }
  }

  // Check results and return
  if ( !pEntry ) return Status::INVALID_OBJ_PATH;
  if ( !pEntry->object() ) return loadObject( pEntry );

  if ( m_enableAccessHdlr ) {
    // Fire data access incident
    // I do not know if this is a good idea....
    // This fires too often!
    //
    // DataIncident incident(name(), m_accessName, pEntry->identifier());
    // m_incidentSvc->fireIncident(incident);
    return StatusCode::SUCCESS;
  }
  return StatusCode::SUCCESS;
}

/// Retrieve object identified by its directory from the data store.
StatusCode DataSvc::retrieveObject( IRegistry* pRegistry, std::string_view path, DataObject*& pObject ) {
  pObject           = nullptr;
  RegEntry * result = nullptr, *parent = CAST_REGENTRY( RegEntry*, pRegistry );
  StatusCode status = i_retrieveEntry( parent, path, result );
  if ( status.isSuccess() ) pObject = result->object();
  return status;
}

/// Retrieve object identified by its directory from the data store.
StatusCode DataSvc::findObject( IRegistry* pRegistry, std::string_view path, DataObject*& pObject ) {
  pObject               = nullptr;
  IRegistry* pReg       = ( pRegistry ? pRegistry : m_root.get() );
  RegEntry*  root_entry = CAST_REGENTRY( RegEntry*, pReg );
  if ( root_entry ) {
    if ( !path.empty() ) pReg = root_entry->find( path );
    if ( !pReg ) return Status::INVALID_OBJ_PATH;
    pObject = pReg->object();
  }
  return ( !pObject ) ? Status::OBJ_NOT_LOADED : Status::IDataProviderSvc_NO_ERROR;
}

/// Find object identified by its full path in the data store.
StatusCode DataSvc::findObject( std::string_view path, DataObject*& pObject ) {
  pObject = nullptr;
  if ( !checkRoot() ) return Status::INVALID_ROOT;
  if ( path.empty() || path == m_rootName.value() ) {
    pObject = m_root->object();
    return !pObject ? Status::OBJ_NOT_LOADED : Status::IDataProviderSvc_NO_ERROR;
  }
  return findObject( path.front() != SEPARATOR ? m_root.get() : nullptr, path, pObject );
}

/// Update object.
StatusCode DataSvc::updateObject( IRegistry* pRegistry ) {
  if ( !pRegistry ) {                // Precondition:
    return Status::INVALID_OBJ_ADDR; // Addres must be valid
  }
  DataObject* toUpdate = pRegistry->object();
  if ( !toUpdate ) { // Try first to load
    return loadObject( pRegistry );
  }
  return updateObject( toUpdate );
}

/// Update object.
StatusCode DataSvc::updateObject( DataObject* toUpdate ) {
  StatusCode status = Status::INVALID_OBJ_ADDR;
  if ( !toUpdate ) {               // Precondition:
    return Status::INVALID_OBJECT; // Address must be valid
  }
  IRegistry* pRegistry = toUpdate->registry(); // Precondition:
  if ( !pRegistry ) {                          // Need valid registry
    return Status::INVALID_OBJECT;
  }
  IOpaqueAddress* pAddress = pRegistry->address(); // Precondition:
  if ( !pAddress ) {                               // Need valid address
    return Status::INVALID_OBJ_ADDR;
  }
  IConversionSvc* pLoader = getDataLoader( pRegistry );
  if ( !pLoader ) {                // Precondition:
    return Status::NO_DATA_LOADER; // Data loader must be present
  }
  if ( !m_inhibitPathes.empty() ) {
    auto inhibit = std::find( m_inhibitPathes.begin(), m_inhibitPathes.end(), pRegistry->identifier() );
    if ( inhibit != m_inhibitPathes.end() ) { return Status::NO_ACCESS; }
  }
  try {
    status = pLoader->updateObj( pAddress, toUpdate ); // Call data loader
    if ( status.isSuccess() ) { status = pLoader->updateObjRefs( pAddress, toUpdate ); }
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

// Link object
StatusCode DataSvc::linkObject( IRegistry* from, std::string_view objPath, DataObject* to ) {
  if ( !checkRoot() ) return Status::INVALID_ROOT;
  try {
    RegEntry* from_entry = CAST_REGENTRY( RegEntry*, from );
    if ( from_entry ) {
      // First check if both objects are already registered to the store
      RegEntry* to_entry = m_root->findLeaf( to );
      if ( !to_entry ) return Status::INVALID_OBJECT;
      auto sep = objPath.rfind( SEPARATOR );
      if ( sep > 0 && sep != std::string_view::npos ) { // in case the objPath is a sub-directory itself
        DataObject* pO = nullptr;
        StatusCode  sc = retrieveObject( from, objPath.substr( 0, sep ), pO );
        return sc.isSuccess() ? linkObject( pO->registry(), objPath.substr( sep ), to ) : sc;
      }
      // Now register the soft link
      StatusCode status = from_entry->add( to_string( objPath ), to, true );
      return status.isSuccess() ? Status::IDataProviderSvc_NO_ERROR : Status::DOUBL_OBJ_PATH;
    }
  } catch ( ... ) {}
  return Status::INVALID_PARENT;
}

/// Add a link to another object.
StatusCode DataSvc::linkObject( std::string_view fullPath, DataObject* to ) {
  if ( fullPath.empty() ) return Status::INVALID_OBJ_PATH;
  if ( fullPath.front() != SEPARATOR ) { return linkObject( m_rootName.value(), fullPath, to ); }
  auto sep = fullPath.rfind( SEPARATOR );
  return linkObject( fullPath.substr( 0, sep ), fullPath.substr( sep ), to );
}

/// Remove a link to another object.
StatusCode DataSvc::unlinkObject( IRegistry* from, std::string_view objPath ) {
  if ( !checkRoot() ) return Status::INVALID_ROOT;
  try {
    RegEntry* from_entry = CAST_REGENTRY( RegEntry*, from );
    if ( from_entry ) {
      auto sep = objPath.rfind( SEPARATOR );
      if ( sep > 0 && sep != std::string_view::npos ) { // in case the objPath is a sub-directory itself
        DataObject* pO = nullptr;
        StatusCode  sc = findObject( from, objPath.substr( 0, sep ), pO );
        return sc.isSuccess() ? unlinkObject( pO->registry(), objPath.substr( sep ) ) : sc;
      }
      StatusCode status = from_entry->remove( objPath );
      if ( status.isSuccess() ) return status;
      return Status::INVALID_OBJ_PATH;
    }
  } catch ( ... ) {}
  return Status::INVALID_PARENT;
}

/// Remove a link to another object.
StatusCode DataSvc::unlinkObject( std::string_view fullPath ) {
  if ( fullPath.empty() ) return Status::INVALID_OBJ_PATH;
  if ( fullPath.front() != SEPARATOR ) return unlinkObject( m_rootName.value(), fullPath );
  auto sep = fullPath.rfind( SEPARATOR );
  return unlinkObject( fullPath.substr( 0, sep ), fullPath.substr( sep ) );
}

/// Remove a link to another object.
StatusCode DataSvc::unlinkObject( DataObject* from, std::string_view objPath ) {
  if ( !checkRoot() ) return Status::INVALID_ROOT;
  IRegistry* from_entry = m_root->findLeaf( from );
  return unlinkObject( from_entry, objPath );
}

/// Add an item to the preload list
StatusCode DataSvc::addPreLoadItem( const DataStoreItem& item ) {
  auto i = std::find( begin( m_preLoads ), end( m_preLoads ), item );
  if ( i == end( m_preLoads ) ) m_preLoads.push_back( item );
  return StatusCode::SUCCESS;
}

/// Remove an item from the preload list
StatusCode DataSvc::removePreLoadItem( const DataStoreItem& item ) {
  m_preLoads.erase( std::remove( begin( m_preLoads ), end( m_preLoads ), item ), end( m_preLoads ) );
  return StatusCode::SUCCESS;
}

/// Clear the preload list
StatusCode DataSvc::resetPreLoad() {
  m_preLoads.erase( begin( m_preLoads ), end( m_preLoads ) );
  return StatusCode::SUCCESS;
}

/// Preload one level deep, then recursively call the next level
StatusCode DataSvc::preLoad( int depth, int load_depth, DataObject* pObject ) {
  // unused:  StatusCode sc = StatusCode::FAILURE;
  if ( pObject && depth++ < load_depth ) {
    RegEntry* dir = CAST_REGENTRY( RegEntry*, pObject->registry() );
    if ( dir ) {
      for ( const auto& i : *dir ) {
        DataObject* pObj   = nullptr;
        StatusCode  status = retrieveObject( pObject, i->name(), pObj );
        if ( status.isSuccess() && depth < load_depth ) { preLoad( depth, load_depth, pObj ).ignore(); }
      }
    }
  }
  return StatusCode::SUCCESS;
}

/// load all preload items of the list
StatusCode DataSvc::preLoad() {
  DataObject* pObj = nullptr;
  for ( const auto& i : m_preLoads ) {
    StatusCode sc         = retrieveObject( i.path(), pObj );
    int        load_depth = i.depth();
    if ( sc.isSuccess() && load_depth > 1 ) { preLoad( 1, load_depth, pObj ).ignore(); }
  }
  return StatusCode::SUCCESS;
}

/// Service initialization
StatusCode DataSvc::initialize() {
  // Nothing to do: just call base class initialisation
  StatusCode sc = Service::initialize();
  if ( UNLIKELY( !sc.isSuccess() ) ) return sc;
  m_incidentSvc = service( "IncidentSvc", true );
  if ( UNLIKELY( !m_incidentSvc ) ) { error() << "Failed to access incident service." << endmsg; }
  return sc;
}

/// Service reinitialization
StatusCode DataSvc::reinitialize() {
  // the finalize part is copied here
  setDataLoader( nullptr ).ignore();
  resetPreLoad().ignore();
  clearStore().ignore();
  m_incidentSvc.reset();
  // re-initialize the base class
  StatusCode sc = Service::reinitialize();
  if ( UNLIKELY( !sc.isSuccess() ) ) {
    error() << "Unable to reinitialize base class" << endmsg;
    return sc;
  }
  // the initialize part is copied here
  m_incidentSvc = service( "IncidentSvc", true );
  if ( UNLIKELY( !m_incidentSvc ) ) {
    error() << "Failed to access incident service." << endmsg;
    return StatusCode::FAILURE;
  }
  // return
  return StatusCode::SUCCESS;
}

/// Service initialisation
StatusCode DataSvc::finalize() {
  // Nothing to do: just call base class initialisation
  setDataLoader( nullptr ).ignore();
  resetPreLoad().ignore();
  clearStore().ignore();
  m_incidentSvc.reset();
  return Service::finalize();
}

/// CLID for root Event
CLID DataSvc::rootCLID() const { return (CLID)m_rootCLID; }

/// Name for root Event
const std::string& DataSvc::rootName() const { return m_rootName; }

/// Create default objects in case forced creation of leaves is requested
DataObject* DataSvc::createDefaultObject() const { return new DataObject(); }

/** Retrieve customizable data loader according to registry entry to
 *  be retrieved
 */
IConversionSvc* DataSvc::getDataLoader( IRegistry* /* pReg */ ) { return m_dataLoader.get(); }
