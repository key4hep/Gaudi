#define  GAUDIHIVE_HIVEEVENTREGISTRYENTRY_CPP

// STL include files
#include <algorithm>

// Interfaces
#include "GaudiKernel/IDataStoreAgent.h"
#include "GaudiKernel/IOpaqueAddress.h"

// Framework include files
#include "GaudiKernel/DataObject.h"
#include "HiveEventRegistryEntry.h"
#include "HiveRegistryEntry.h"


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
Hive::HiveEventRegistryEntry::HiveEventRegistryEntry(const std::string& path, DataSvcHelpers::RegistryEntry* parent)
  : DataSvcHelpers::RegistryEntry(path, parent) 
{
  // ignore the path name and take the one of the parent
  m_fullpath = parent->identifier();
  m_path = "";
}

/// Standard destructor; nothing to do here
Hive::HiveEventRegistryEntry::~HiveEventRegistryEntry()  {
}

/// Internal method to add entries
Hive::HiveRegistryEntry* Hive::HiveEventRegistryEntry::i_add(const std::string& nam)    {
  HiveRegistryEntry* tmp(0);
  if ( nam[0] != SEPARATOR )   {
    std::string path = nam;
    path.insert(path.begin(), SEPARATOR);
    return i_add(path);
  }
  // if this object is already present, this is an error....
  for (Store::iterator i = m_store.begin(); i !=  m_store.end(); ++i )   {
    // tbb::concurrent vector is not thread safe when
    // doing insert and iteration at the same time
    // One has to wait until the object being put is
    // fully constructed. Here:
    // Wait for the pointer becoming valid
    // Alternatively one could introduce real locks. 
    //while( (*i) == NULL) { std::cout << "Ciao";} // TODO: maybe make it volatile? 
    if (*i && nam == (*i)->name() )  {
      return 0;
    }
  }
  tmp = new HiveRegistryEntry( nam, this );
  
  return tmp;
}

/// Add entry to the current data store item
long Hive::HiveEventRegistryEntry::add ( const std::string& name, DataObject * pObject, bool is_soft ) {
  HiveRegistryEntry* entry = i_add(name);
  if ( 0 != entry ) {
    ( is_soft ) ? entry->makeSoft(pObject) : entry->makeHard(pObject);
    add( entry );
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

///Add object to the container
long Hive::HiveEventRegistryEntry::add( IRegistry* obj )    {
  unsigned int size = RegistryEntry::add(obj);
  // set proper full path and notify;
  // TODO: not the provider, but the HIveEventwhatever should; let the DataProviderSvc know about the new data object 
  HiveRegistryEntry* pEntry = dynamic_cast<HiveRegistryEntry*>(obj);
  m_publishedData.push(pEntry->m_fullpath);
  return size;
}

/// Add entry to the current data store item
long Hive::HiveEventRegistryEntry::add ( const std::string& name, IOpaqueAddress* pAddress, bool is_soft )  {
  HiveRegistryEntry* entry = i_add(name);
  if ( 0 != entry )   {
    ( is_soft ) ? entry->makeSoft(pAddress) : entry->makeHard(pAddress);
    add( entry );
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

