/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <GaudiMP/TESSerializer.h>

// Framework include files
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/DataStoreItem.h>
#include <GaudiKernel/GaudiException.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/LinkManager.h>
#include <GaudiKernel/System.h>

#include <GaudiKernel/ClassID.h>
#include <GaudiKernel/GenericAddress.h>
#include <GaudiKernel/IOpaqueAddress.h>
#include <GaudiKernel/KeyedContainer.h>

#include <GaudiKernel/MsgStream.h>

// ROOT include files
#include <TBufferFile.h>
#include <TClass.h>
#include <TInterpreter.h>
#include <TROOT.h>

#include <map>

// a constant to guard against seg-faults in loadBuffer
#define SERIALIZER_END "EOF"

namespace {
  struct DataObjectPush {
    DataObjectPush( DataObject*& p ) { Gaudi::pushCurrentDataObject( &p ); }
    ~DataObjectPush() { Gaudi::popCurrentDataObject(); }
  };
} // namespace

using namespace std;

bool GaudiMP::TESSerializer::analyse( IRegistry* dir, int level ) {
  if ( level < m_currentItem->depth() ) {
    if ( dir->object() != 0 ) {
      m_objects.push_back( dir->object() );
      return true;
    }
  }
  return false;
}

/// Constructor
GaudiMP::TESSerializer::TESSerializer( IDataProviderSvc* svc, IAddressCreator* ac )
    : m_TES( svc )
    , m_TESMgr( dynamic_cast<IDataManagerSvc*>( svc ) )
    , m_currentItem( 0 )
    , m_verifyItems( false )
    , m_strict( false )
    , m_addressCreator( ac ) {}

/// Serialize contents of TES to a TBufferFile
void GaudiMP::TESSerializer::dumpBuffer( TBufferFile& buffer ) {
  //
  //  Write all valid objects to the TBufferFile provided in the argument
  //  As objects are collected, the member variable m_classMap is filled
  //  with ROOT Class data, and is kept by the Serializer for future
  //  reference
  //
  //  @paramTBufferFile& buffer : TBufferFile passed by reference
  //          Cannot be declared inside the method, as repeated calls
  //          can cause confusion and buffer wiping.
  //
  StatusCode  status;
  DataObject* obj;

  // Clear current selection
  m_objects.clear();

  // Traverse the tree and collect the requested objects
  for ( DataStoreItem* item : m_itemList ) {
    m_currentItem = item;
    // cout << "Retrieving Mandatory Object : " << m_currentItem->path() << endl;
    status = m_TES->retrieveObject( m_currentItem->path(), obj );
    if ( status.isSuccess() ) {
      m_TESMgr->traverseSubTree( obj, this ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
    } else {
      string text( "WARNING: Cannot retrieve TES object(s) for serialisation: " );
      text += m_currentItem->path();
      if ( m_strict ) {
        throw GaudiException( text + m_currentItem->path(), "", status );
      } else {
        cout << text << endl;
        // return StatusCode::FAILURE;
      }
    }
  }
  // Traverse the tree and collect the requested objects (tolerate missing items here)
  for ( DataStoreItem* item : m_optItemList ) {
    m_currentItem = item;
    // cout << "Retrieving Optional Object : " << m_currentItem->path() << endl;
    status = m_TES->retrieveObject( m_currentItem->path(), obj );
    if ( status.isSuccess() ) {
      m_TESMgr->traverseSubTree( obj, this ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
    }
  }

  // Prepare for serialization:
  //  - find all TClass pointers needed and ignore objects without dictionaries
  std::vector<std::tuple<DataObject*, TClass*>> objects;
  objects.reserve( m_objects.size() );
  for_each( begin( m_objects ), end( m_objects ), [this, &objects]( auto obj ) {
    if ( auto cl = getClass( obj ) ) objects.emplace_back( obj, cl );
  } );

  // cout << "TESSerializer : Beginning loop to write to TBufferFile for nObjects : " << m_objects.size() << endl;
  buffer.WriteInt( objects.size() );

  for ( auto& [pObj, cl] : objects ) {
    DataObjectPush p( pObj ); /* add the data object to the list... */

    // write object to buffer in order location-name-object
    std::string loc = pObj->registry()->identifier();
    buffer.WriteString( loc.c_str() );
    buffer.WriteString( cl->GetName() );
    cl->Streamer( pObj, buffer );

    /* take care of links */
    LinkManager* linkMgr  = pObj->linkMgr();
    long int     numLinks = linkMgr->size();
    buffer.WriteInt( numLinks );
    // now write each link
    for ( int it = 0; it != numLinks; it++ ) {
      const string& link = linkMgr->link( it )->path();
      buffer.WriteString( link.c_str() );
    }

    // now do the thing with the opaqueAddress
    // to go from string->object when recovering, will need svc_type, and clid, aswell as the string version
    IOpaqueAddress* iop = pObj->registry()->address();
    if ( iop ) {
      buffer.WriteInt( 1 );
      const string* par     = iop->par();
      long          svcType = iop->svcType();
      long          clid    = iop->clID();
      buffer.WriteLong( svcType );
      buffer.WriteLong( clid );
      buffer.WriteString( par->c_str() );
    } else {
      buffer.WriteInt( 0 );
    }
    // object complete, continue in for-loop
  }

  // Final Actions
  // Write the End Flag, to avoid potential SegFaults on loadBuffer
  buffer.WriteString( SERIALIZER_END );
  // return StatusCode::SUCCESS;
}

/// Reconstruct the TES from a given TBufferFile
void GaudiMP::TESSerializer::loadBuffer( TBufferFile& buffer ) {

  // reverse mechanism of dumps
  // buffer is: length of DataObjects vector
  //            location string
  //            type name string
  //            the object itself
  //            count of links
  //            list of links (conditional on count)
  //            flag indicating Opaque Address presence
  //            Opaque Address svcType (conditional on flag)
  //            Opaque Address clID    (conditional on flag)
  //            Opaque Address par     (conditional on flag)

  int nObjects;
  // 3 StatusCodes... for :
  //   general use : registering objects : creating OpaqueAddresses
  StatusCode sc, registerStat, createAddressStat;

  // Prepare for Reading
  buffer.SetReadMode();
  buffer.SetBufferOffset();

  buffer.ReadInt( nObjects );
  for ( int i = 0; i < nObjects; ++i ) {
    char text[4096];
    buffer.ReadString( text, sizeof( text ) );
    string location( text );
    if ( !location.compare( "EOF" ) ) {
      /* There was an error in serialization, but the EOF
         flag marks the endpoint in any case */
      break;
    }
    buffer.ReadString( text, sizeof( text ) );
    TClass* cl = gROOT->GetClass( text );
    if ( cl == 0 ) {
      if ( m_strict ) {
        throw GaudiException( "gROOT->GetClass cannot find clName", text, StatusCode::FAILURE );
      } else {
        cout << "TESSerializer WARNING : gROOT->GetClass fails for clname : " << location.c_str() << endl;
        continue;
      }
    }

    /// The next is equivalent to ReadObjectAny(cl) except of the 'magic!!'
    DataObject*    obj = (DataObject*)cl->New();
    DataObjectPush push( obj ); // This is magic!
    cl->Streamer( obj, buffer );

    // now restore links
    if ( obj ) {
      int          nlink  = 0;
      LinkManager* lnkMgr = obj->linkMgr();
      buffer.ReadInt( nlink );

      for ( int j = 0; j < nlink; ++j ) {
        buffer.ReadString( text, sizeof( text ) );
        lnkMgr->addLink( text, 0 );
      }
    }

    // Re-register...
    registerStat = m_TES->registerObject( location, obj );
    if ( registerStat.isFailure() ) {
      DataObject* dummy = NULL;
      if ( location == "/Event" ) {
        sc = m_TESMgr->setRoot( location, obj );
        if ( sc.isFailure() ) throw GaudiException( "Cannot set root at location " + location, "", sc );
      } else {
        m_TES->findObject( location, dummy ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
        if ( !dummy )
          m_TES->registerObject( location, obj ).ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );
        else {
          // skipping to the next object
          // (flush the remaining metadata in the buffer)
          int flag( 0 );
          buffer.ReadInt( flag );
          if ( flag ) {
            long svcType;
            buffer.ReadLong( svcType );
            long clid;
            buffer.ReadLong( clid );
            buffer.ReadString( text, sizeof( text ) );
          }
          continue;
        }
      }
    }
    // next is the opaque address information
    // create Generic Address using the info from the TBufferFile,
    // then create an IOpaqueAddress object using the Persistency Svc
    // IOpaque Address pointer (blank... pass the ref to the createAddress Fn)

    int flag( 0 );
    buffer.ReadInt( flag );
    // flag will be 0 or 1 to indicate OpaqueAddress Info
    if ( flag == 1 ) {
      // will need an IOpaqueAddress and its ref
      IOpaqueAddress*  iop;
      IOpaqueAddress*& iopref = iop;
      // Read svcType, clID and par from buffer
      long svcType;
      buffer.ReadLong( svcType );

      long clid;
      buffer.ReadLong( clid );
      const CLID classid( clid );

      char* cp;
      cp = buffer.ReadString( text, sizeof( text ) );
      const string opaque( cp );
      // create Generic address
      // already have svcType, clID, par1.. just make dummy variables for par2, and ipar1 and 2
      const string&  p2 = "";
      unsigned long  ip1( 0 );
      unsigned long  ip2( 0 );
      GenericAddress gadd( svcType, classid, opaque, p2, ip1, ip2 );

      // now create the address
      createAddressStat =
          m_addressCreator->createAddress( gadd.svcType(), gadd.clID(), gadd.par(), gadd.ipar(), iopref );
      if ( createAddressStat.isFailure() ) {
        throw GaudiException( "Failure in creating OpaqueAddress for reconstructed registry", "", createAddressStat );
      }
      // And finally, set this address
      obj->registry()->setAddress( iop );
    }
    // all done
  }
}

// Protected
/// Add item to output streamer list (protected)
void GaudiMP::TESSerializer::addItem( Items& itms, const std::string& descriptor ) {
  // supports # notation
  int level = 0;

  std::string slevel;
  std::string obj_path;

  // Process the incoming string
  size_t sep = descriptor.rfind( "#" );
  if ( sep > descriptor.length() ) {
    // invalid sep case (# not found in string)
    obj_path = descriptor;
    slevel   = "1";
  } else {
    // valid sep case
    obj_path = descriptor.substr( 0, sep );
    slevel   = descriptor.substr( sep + 1 );
  }

  // Convert the level string to an integer
  if ( slevel == "*" ) {
    level = 9999999;
  } else {
    level = std::stoi( slevel );
  }

  // Are we verifying?
  if ( m_verifyItems ) {
    size_t idx = obj_path.find( "/", 1 );
    while ( idx != std::string::npos ) {
      std::string sub_item = obj_path.substr( 0, idx );
      if ( 0 == findItem( sub_item ) ) {
        cout << "... calling addItem with arg : " << sub_item << endl;
        addItem( itms, sub_item );
      }
      idx = obj_path.find( "/", idx + 1 );
    }
  }
  DataStoreItem* item = new DataStoreItem( obj_path, level );
  // cout << "Adding TESSerializer item " << item->path()
  //   << " with " << item->depth()
  //   << " level(s)." << endl;
  itms.push_back( item );
}

/// Add item to serialization list; ie append to std::vector of DataStoreItems
void GaudiMP::TESSerializer::addItem( const std::string& path ) {
  // #notation supported
  addItem( m_itemList, path );
}

/// Add optional item to output streamer list
void GaudiMP::TESSerializer::addOptItem( const std::string& path ) {
  // #notation supported
  addItem( m_optItemList, path );
}

/// Uses cout to print the contents of the mandatory and optional item lists
void GaudiMP::TESSerializer::checkItems() {
  cout << "TESSerializer m_itemList : " << m_itemList.size() << " Items" << endl;
  for ( Items::const_iterator i = m_itemList.begin(); i != m_itemList.end(); ++i ) {
    cout << "\tItem : " << ( *i )->path() << endl;
  }
  cout << "TESSerializer m_optItemList : " << m_optItemList.size() << " Items" << endl;
  for ( Items::const_iterator i = m_optItemList.begin(); i != m_optItemList.end(); ++i ) {
    cout << "\tItem : " << ( *i )->path() << endl;
  }
}

/// Find single item identified by its path (exact match)
DataStoreItem* GaudiMP::TESSerializer::findItem( const std::string& path ) {
  for ( Items::const_iterator i = m_itemList.begin(); i != m_itemList.end(); ++i ) {
    if ( ( *i )->path() == path ) return ( *i );
  }
  for ( Items::const_iterator j = m_optItemList.begin(); j != m_optItemList.end(); ++j ) {
    if ( ( *j )->path() == path ) return ( *j );
  }
  return 0;
}
