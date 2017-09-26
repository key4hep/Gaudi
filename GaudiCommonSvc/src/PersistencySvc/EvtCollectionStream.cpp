//	====================================================================
//  EvtCollectionStream.cpp
//	--------------------------------------------------------------------
//
//	Package   : GaudiSvc/PersistencySvc
//
//	Author    : Markus Frank
//
//	====================================================================
#define GAUDISVC_PERSISTENCYSVC_EVTCOLLECTIONSTREAM_CPP

// Framework include files
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/MsgStream.h"

#include "EvtCollectionStream.h"
#include "GaudiKernel/DataStoreItem.h"

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT( EvtCollectionStream )

// initialize data writer
StatusCode EvtCollectionStream::initialize()
{
  // Use the Job options service to set the Algorithm's parameters
  setProperties();
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
StatusCode EvtCollectionStream::finalize()
{
  m_pTupleSvc = nullptr; // release
  clearItems();
  return StatusCode::SUCCESS;
}

// Work entry point
StatusCode EvtCollectionStream::execute()
{
  StatusCode status = ( m_pTupleSvc ) ? StatusCode::SUCCESS : StatusCode::FAILURE;
  if ( status.isSuccess() ) {
    for ( const auto& i : m_itemList ) {
      StatusCode iret                 = m_pTupleSvc->writeRecord( i->path() );
      if ( !iret.isSuccess() ) status = iret;
    }
  }
  return status;
}

// Remove all items from the output streamer list;
void EvtCollectionStream::clearItems() { m_itemList.clear(); }

// Add item to output streamer list
void EvtCollectionStream::addItem( const std::string& descriptor )
{
  auto sep             = descriptor.rfind( "#" );
  int level            = 0;
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
