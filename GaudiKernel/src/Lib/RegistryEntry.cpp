// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/RegistryEntry.cpp,v 1.12 2006/07/07 13:15:30 hmd Exp $
//====================================================================
//	RegistryEntry.cpp
//--------------------------------------------------------------------
//
//	Package    : DataSvc ( The LHCb Offline System)
//
//  Description: implementation of the Transient data store
//
//	Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who     
// +---------+----------------------------------------------+---------
// | 29/10/98| Initial version                              | MF
// | 03/02/99| Protect dynamic_cast with try-catch clauses  | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define  DATASVC_REGISTRYENTRY_CPP

// STL include files
#include <algorithm>

// Interfaces
#include "GaudiKernel/IDataStoreAgent.h"
#include "GaudiKernel/IOpaqueAddress.h"

// Framework include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/RegistryEntry.h"


// If you absolutely need optimization: switch off dynamic_cast.
// This improves access to the data store roughly by more than 10 %
// for balanced trees.
//
// M.Frank
//
#define CAST_REGENTRY(x,y) dynamic_cast<x>(y)
//#define CAST_REGENTRY(x,y) (x)(y)
enum Seperator { SEPARATOR='/' };

/// Standard Constructor
DataSvcHelpers::RegistryEntry::RegistryEntry(const std::string& path, RegistryEntry* parent)
: m_refCount(0), 
  m_isSoft(false), 
  m_path(path),
  m_pParent(parent),
  m_pAddress(0),
  m_pObject(0), 
  m_pDataProviderSvc(0)
{
  std::string::size_type sep = m_path.rfind(SEPARATOR);
  if ( path[0] != SEPARATOR )   {
    m_path.insert(m_path.begin(), SEPARATOR);
  }
  if ( sep != std::string::npos )    {
    m_path.erase(0,sep);
  }
  assemblePath(m_fullpath);
  addRef();
}

/// Standard destructor
DataSvcHelpers::RegistryEntry::~RegistryEntry()  {
  deleteElements();
  if ( 0 != m_pObject  )   {
    if ( !m_isSoft ) m_pObject->setRegistry(0);
    m_pObject->release();
  }
  if ( 0 != m_pAddress )   {
    if ( !m_isSoft ) m_pAddress->setRegistry(0);
    m_pAddress->release();
  }
}

/// Release entry
unsigned long DataSvcHelpers::RegistryEntry::release()  {
  unsigned long cnt = --m_refCount;
  if ( 0 == m_refCount )   {
    delete this;
  }
  return cnt;
}

/// Set new parent pointer
void DataSvcHelpers::RegistryEntry::setParent(RegistryEntry* pParent)    {
  m_pParent = pParent;
  m_fullpath = "";
  assemblePath(m_fullpath);
}

/// Create soft link
void DataSvcHelpers::RegistryEntry::makeSoft(DataObject* pObject)   {
  m_isSoft = true;
  setObject(pObject);
//  if ( 0 != m_pObject )   { // Useless: This justs sets my own address again...
//    setAddress(m_pObject->address());
//  }
}

/// Create soft link
void DataSvcHelpers::RegistryEntry::makeSoft(IOpaqueAddress* pAddress)   {
  m_isSoft = true;
  setAddress(pAddress);
}

/// Create hard link
void DataSvcHelpers::RegistryEntry::makeHard(DataObject* pObject)   {
  makeSoft(pObject);
  m_isSoft = false;
  if ( 0 != m_pObject )   {
    m_pObject->setRegistry(this);
  }
  if ( 0 != m_pAddress )   {
    m_pAddress->setRegistry(this);
  }
}

/// Create hard link
void DataSvcHelpers::RegistryEntry::makeHard(IOpaqueAddress* pAddress)   {
  m_isSoft = false;
  setAddress(pAddress);
}

/// Update Opaque address of registry entry
void DataSvcHelpers::RegistryEntry::setAddress( IOpaqueAddress* pAddress )    {
  if ( 0 != pAddress  )   {
    pAddress->addRef();
    pAddress->setRegistry(this);
  }
  if ( 0 != m_pAddress ) m_pAddress->release();
  m_pAddress = pAddress;
}

/// Set object pointer of data store item
void DataSvcHelpers::RegistryEntry::setObject( DataObject* pObject )   {
  if ( 0 != pObject  )  {
    pObject->addRef();
    if ( !isSoft() ) pObject->setRegistry(this);
  }
  if ( 0 != m_pObject ) m_pObject->release();
  m_pObject = pObject;
}

/// Remove an object from the container
long DataSvcHelpers::RegistryEntry::remove  ( IRegistry* obj )    {
  try   {
    RegistryEntry* pEntry = dynamic_cast<RegistryEntry*>(obj);
    Store::iterator i = std::remove(m_store.begin(), m_store.end(), pEntry);
    if (i != m_store.end())   {
      pEntry->release();
      m_store.erase( i, m_store.end() );
    }
  }
  catch ( ... )   {     }
  return m_store.size();
}

/// Remove entry from data store
long DataSvcHelpers::RegistryEntry::remove ( const std::string& nam )  {
  if ( nam[0] != SEPARATOR )   {
    std::string path = nam;
    path.insert(path.begin(), SEPARATOR);
    return remove(path);
  }
  // if this object is already present, this is an error....
  for (Store::iterator i = m_store.begin(); i != m_store.end(); i++ )   {
    if ( nam == (*i)->name() )   {
      remove(*i);
      return StatusCode::SUCCESS;
    }
  }
  return StatusCode::FAILURE;
}

/// Internal method to add entries
DataSvcHelpers::RegistryEntry* DataSvcHelpers::RegistryEntry::i_add(const std::string& nam)    {
  if ( nam[0] != SEPARATOR )   {
    std::string path = nam;
    path.insert(path.begin(), SEPARATOR);
    return i_add(path);
  }
  // if this object is already present, this is an error....
  for (Store::iterator i = m_store.begin(); i != m_store.end(); i++ )   {
    if ( nam == (*i)->name() )  {
      return 0;
    }
  }
  return new RegistryEntry( nam, this );
}

///Add object to the container
long DataSvcHelpers::RegistryEntry::add( IRegistry* obj )    {
  try   {
    RegistryEntry* pEntry = CAST_REGENTRY(RegistryEntry*, obj);
    pEntry->setDataSvc(m_pDataProviderSvc);
    m_store.push_back(pEntry);
    pEntry->setParent(this);
    if ( !pEntry->isSoft() && pEntry->address() != 0 )   {
      pEntry->address()->setRegistry(pEntry);
    }
  }
  catch ( ... )   {
  }
  return m_store.size();
}

/// Add entry to the current data store item
long DataSvcHelpers::RegistryEntry::add ( const std::string& name, DataObject* pObject, bool is_soft )  {
  RegistryEntry* entry = i_add(name);
  if ( 0 != entry )   {
    ( is_soft ) ? entry->makeSoft(pObject) : entry->makeHard(pObject);
    add( entry );
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

/// Add entry to the current data store item
long DataSvcHelpers::RegistryEntry::add ( const std::string& name, IOpaqueAddress* pAddress, bool is_soft )  {
  RegistryEntry* entry = i_add(name);
  if ( 0 != entry )   {
    ( is_soft ) ? entry->makeSoft(pAddress) : entry->makeHard(pAddress);
    add( entry );
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

/// Delete recursively all elements pending from the current store item
long DataSvcHelpers::RegistryEntry::deleteElements()   {
  for (Store::iterator i = m_store.begin(); i != m_store.end(); i++ )   {
    RegistryEntry* entry = CAST_REGENTRY(RegistryEntry*, *i);
    if ( 0 != entry )   {
      entry->deleteElements();
      entry->release();
    }
  }
  m_store.erase(m_store.begin(), m_store.end());
  return 0;
}

/// Try to find an object identified by its pointer
IRegistry* DataSvcHelpers::RegistryEntry::i_find( const IRegistry* obj )  const  {
  Store::const_iterator i = std::find(m_store.begin(),m_store.end(),obj);
  return (i==m_store.end()) ? 0 : (*i);
}

/// Find identified leaf in this registry node
DataSvcHelpers::RegistryEntry* DataSvcHelpers::RegistryEntry::i_find(const std::string& path)   const    {
  if ( path[0] != SEPARATOR )    {
    std::string thePath = path;
    thePath.insert(thePath.begin(), SEPARATOR);
    return i_find(thePath);
  }
  else  {
    std::string::size_type len  = path.length();
    std::string::size_type loc1 = path.find(SEPARATOR,1);
    std::string::size_type len2 = loc1 != std::string::npos ? loc1 : len;
    const char* data = path.data();
    for (Store::const_iterator i = m_store.begin(); i != m_store.end(); i++ )   {
      RegistryEntry* regEnt = CAST_REGENTRY(RegistryEntry*, *i);
      const std::string& nam = regEnt->name();
      if ( nam.length() == len2 )   {
        if ( std::equal(nam.begin(), nam.begin()+len2, data)  )    {
          try   {
            if ( loc1 != std::string::npos )   {
              std::string search_path(path, loc1, len);
              IRegistry* pDir = regEnt->find(search_path);
              if ( 0 != pDir )    {
                return CAST_REGENTRY(RegistryEntry*, pDir);
              }
              return 0;
            }
            else  {
              return CAST_REGENTRY(RegistryEntry*, *i);
            }
          }
          catch (...)   {
          }
        }
      }
    }
    if ( m_path.length() == len2 )    {
      if ( std::equal(m_path.begin(), m_path.begin()+len2, data)  )    {
        if (len2 < len)   {
          std::string search_path(path, loc1, len);
          return i_find(search_path);
        }
        //return this;
      }
    }
  }
  return 0;
}

/// Find identified leaf in this registry node
DataSvcHelpers::RegistryEntry* DataSvcHelpers::RegistryEntry::i_find(const DataObject* key)  const  {
  if ( 0 != key )  {
    if ( key == m_pObject )    {
      return const_cast<RegistryEntry*>(this);
    }
    // Look in the immediate level:
    RegistryEntry *result = CAST_REGENTRY(RegistryEntry*, i_find(key->registry()));
    if ( 0 != result ) return result;
    // Go levels down
    for (Store::const_iterator i = m_store.begin(); i != m_store.end(); i++ )   {
      try   {
        const RegistryEntry *entry = CAST_REGENTRY(RegistryEntry*, *i);
        if( 0 != (result = entry->i_find(key)) ) 
          return result;
      }
      catch ( ... )   {    }
    }
  }
  return 0;
}

// Traverse registry tree
long DataSvcHelpers::RegistryEntry::traverseTree(IDataStoreAgent* pAgent, int level)    {
  bool go_down = pAgent->analyse(this, level);
  long status = StatusCode::SUCCESS;
  if ( go_down )    {
    for ( Store::iterator i = m_store.begin(); i != m_store.end(); i++ )   {
      try   {
        RegistryEntry* entry = CAST_REGENTRY(RegistryEntry*, *i);
        entry->traverseTree(pAgent, level+1);
      }
      catch (...)   {
        status = StatusCode::FAILURE;
      }
    }
  }
  return status;
}

// Recursive helper to assemble the full path name of the entry
void DataSvcHelpers::RegistryEntry::assemblePath(std::string& buffer)  const  {
  if ( m_pParent != 0 )    {
    m_pParent->assemblePath(buffer);
  }
  buffer += m_path;
}
