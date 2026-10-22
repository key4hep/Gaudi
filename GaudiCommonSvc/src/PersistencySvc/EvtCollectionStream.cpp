/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
//	====================================================================
//  EvtCollectionStream.cpp
//	--------------------------------------------------------------------
//
//	Package   : GaudiSvc/PersistencySvc
//
//	Author    : Markus Frank
//
//	====================================================================

// Framework includes
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/DataStoreItem.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/INTupleSvc.h>
#include <GaudiKernel/MsgStream.h>

// STL includes
#include <memory>
#include <string>
#include <vector>

/** A small to stream Data I/O.
    Author:  M.Frank
    Version: 1.0
*/
class EvtCollectionStream : public Algorithm {
protected:
  Gaudi::Property<std::vector<std::string>> m_itemNames{ this, "ItemList", {}, "vector of item names" };
  Gaudi::Property<std::string>              m_storeName{ this, "EvtDataSvc", "TagCollectionSvc",
                                            "name of the service managing the data store" };

  /// Reference to Tuple service for event collection (may or may not be NTuple service)
  SmartIF<INTupleSvc> m_pTupleSvc;
  /// Vector of items to be saved to this stream
  std::vector<std::unique_ptr<DataStoreItem>> m_itemList;

protected:
  /// Clear item list
  void clearItems();
  /// Add item to output streamer list
  void addItem( const std::string& descriptor );

public:
  /// Inherited constructor
  using Algorithm::Algorithm;
  /// Initialize EvtCollectionStream
  StatusCode initialize() override;
  /// Terminate EvtCollectionStream
  StatusCode finalize() override;
  /// Working entry point
  StatusCode execute() override;
};

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT( EvtCollectionStream )

// initialize data writer
StatusCode EvtCollectionStream::initialize() {
  // Get access to the DataManagerSvc
  m_pTupleSvc = serviceLocator()->service( m_storeName );
  if ( !m_pTupleSvc ) {
    fatal() << "Unable to locate IDataManagerSvc interface" << endmsg;
    return StatusCode::FAILURE;
  }
  // Clear the item list
  clearItems();
  // Take the new item list from the properties.
  for ( const auto& i : m_itemNames ) addItem( i );
  info() << "Data source:             " << m_storeName.value() << endmsg;
  return StatusCode::SUCCESS;
}

// terminate data writer
StatusCode EvtCollectionStream::finalize() {
  m_pTupleSvc = nullptr; // release
  clearItems();
  return StatusCode::SUCCESS;
}

// Work entry point
StatusCode EvtCollectionStream::execute() {
  StatusCode status = ( m_pTupleSvc ) ? StatusCode::SUCCESS : StatusCode::FAILURE;
  if ( status.isSuccess() ) {
    for ( const auto& i : m_itemList ) {
      StatusCode iret = m_pTupleSvc->writeRecord( i->path() );
      if ( !iret.isSuccess() ) status = iret;
    }
  }
  return status;
}

// Remove all items from the output streamer list;
void EvtCollectionStream::clearItems() { m_itemList.clear(); }

// Add item to output streamer list
void EvtCollectionStream::addItem( const std::string& descriptor ) {
  auto        sep      = descriptor.rfind( "#" );
  int         level    = 0;
  std::string obj_path = descriptor.substr( 0, sep );
  if ( sep != std::string::npos ) {
    std::string slevel = descriptor.substr( sep + 1 );
    if ( slevel == "*" ) {
      level = 9999999;
    } else {
      level = std::stoi( slevel );
    }
  }
  m_itemList.emplace_back( new DataStoreItem( obj_path, level ) );
  const auto& item = m_itemList.back();
  info() << "Adding OutputStream item " << item->path() << " with " << item->depth() << " level(s)." << endmsg;
}
