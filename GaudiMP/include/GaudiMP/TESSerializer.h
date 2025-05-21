/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IAddressCreator.h>
#include <GaudiKernel/IDataStoreAgent.h>

// ROOT includes
#include <TClass.h>

// vector and string
#include <map>
#include <string>
#include <typeindex>
#include <vector>

// forward declarations
class IDataProviderSvc;
struct IDataManagerSvc;
class TBufferFile;
class DataStoreItem;
class IAddressCreator;

/** A class to serialize/deserialize
    TES objects to and from a TBufferFile
    Author:  P. Mato
    Author:  E. Smith
    Version: 1.1
*/

namespace GaudiMP {
  class GAUDI_API TESSerializer : virtual public IDataStoreAgent {
    typedef std::vector<DataStoreItem*> Items;
    typedef std::vector<std::string>    ItemNames;
    typedef std::vector<DataObject*>    Objects;

  public:
    /// Constructor
    TESSerializer( IDataProviderSvc* svc, IAddressCreator* ac );

    /// Dump TES contents listed in m_itemList/m_optItemList to a TBufferFile
    void dumpBuffer( TBufferFile& );

    /// Rebuild TES from items in a TBufferFile
    void loadBuffer( TBufferFile& );

    /// add an item to the TESSerializer's list (#notation)
    void addItem( const std::string& path );

    /// add an item to the TESSerializer's optional list (#notation)
    void addOptItem( const std::string& path );

    /// Analysis callback
    bool analyse( IRegistry* dir, int level ) override;

    /// print out the contents of m_itemList and m_optItemList (std::cout)
    void checkItems();

    virtual ~TESSerializer() {}

  protected:
    /// Add item to the list of items to be serialized (#notation)
    void addItem( Items& itms, const std::string& descriptor );

    /// Find single item identified by its path (exact match)
    DataStoreItem* findItem( const std::string& path );

  private:
    /// caching wrapper to TClass::GetClass
    TClass* getClass( DataObject* obj ) {
      auto& id  = typeid( *obj );
      auto  pos = m_classMap.find( id );
      if ( pos == end( m_classMap ) ) { return m_classMap[id] = TClass::GetClass( id ); }
      return pos->second;
    }

    /// TES pointer
    IDataProviderSvc* m_TES;
    /// TES pointer
    IDataManagerSvc* m_TESMgr;
    /// Vector of item names
    ItemNames m_itemNames;
    /// Vector of items to be saved to this stream (DataStoreItem ptrs)
    Items m_itemList;
    /// Vector of item names (std::strings)
    ItemNames m_optItemNames;
    /// Vector of optional items to be saved to this stream (DataStoreItem ptrs)
    Items m_optItemList;
    /// Current item while traversing the TES tree
    DataStoreItem* m_currentItem;
    /// Selected list of Objects to be serialized (DataObject ptrs)
    Objects m_objects;

    /// Map of gROOT class information
    std::map<std::type_index, TClass*> m_classMap;
    /// Boolean Flag as used by GaudiSvc/PersistencySvc/OutputStreamer
    bool m_verifyItems;
    /// Boolean Flag used to determine error tolerance
    bool m_strict;
    /// IAddress Creator for Opaque Addresses
    IAddressCreator* m_addressCreator;
  };
} // namespace GaudiMP
