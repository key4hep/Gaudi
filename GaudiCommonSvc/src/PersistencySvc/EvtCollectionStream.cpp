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
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"

#include "GaudiKernel/DataStoreItem.h"
#include "EvtCollectionStream.h"

// Define the algorithm factory for the standard output data writer
DECLARE_ALGORITHM_FACTORY(EvtCollectionStream)

// Standard Constructor
EvtCollectionStream::EvtCollectionStream(const std::string& name, ISvcLocator* pSvcLocator)
: Algorithm(name, pSvcLocator)
{
  m_storeName = "TagCollectionSvc";
  declareProperty("ItemList",   m_itemNames);
  declareProperty("EvtDataSvc", m_storeName);
}

// Standard Destructor
EvtCollectionStream::~EvtCollectionStream()   {
}

// initialize data writer
StatusCode EvtCollectionStream::initialize() {
  MsgStream log(msgSvc(), name());
  // Use the Job options service to set the Algorithm's parameters
  setProperties();
  // Get access to the DataManagerSvc
  m_pTupleSvc = serviceLocator()->service(m_storeName);
  if( !m_pTupleSvc.isValid() ) {
    log << MSG::FATAL << "Unable to locate IDataManagerSvc interface" << endmsg;
    return StatusCode::FAILURE;
  }
  // Clear the item list
  clearItems();
  // Take the new item list from the properties.
  for(ItemNames::iterator i = m_itemNames.begin(); i != m_itemNames.end(); i++)   {
    addItem( *i );
  }
  log << MSG::INFO << "Data source:             " << m_storeName  << endmsg;
  return StatusCode::SUCCESS;
}

// terminate data writer
StatusCode EvtCollectionStream::finalize()    {
  m_pTupleSvc = 0; // release
  clearItems();
  return StatusCode::SUCCESS;
}

// Work entry point
StatusCode EvtCollectionStream::execute() {
  StatusCode status = (m_pTupleSvc) ? StatusCode::SUCCESS : StatusCode::FAILURE;
  if ( status.isSuccess() )   {
    for ( Items::iterator i = m_itemList.begin(); i != m_itemList.end(); i++ )    {
      StatusCode iret = m_pTupleSvc->writeRecord((*i)->path());
      if ( !iret.isSuccess() )    {
        status = iret;
      }
    }
  }
  return status;
}

// Remove all items from the output streamer list;
void EvtCollectionStream::clearItems()     {
  for ( Items::iterator i = m_itemList.begin(); i != m_itemList.end(); i++ )    {
    delete (*i);
  }
  m_itemList.erase(m_itemList.begin(), m_itemList.end());
}

// Add item to output streamer list
void EvtCollectionStream::addItem(const std::string& descriptor)   {
  MsgStream log(msgSvc(), name());
  int sep = descriptor.rfind("#");
  int level = 0;
  std::string obj_path (descriptor,0,sep);
  std::string slevel   (descriptor,sep+1,descriptor.length());
  if ( slevel == "*" )  {
    level = 9999999;
  }
  else   {
    level = ::atoi(slevel.c_str());
  }
  DataStoreItem* item = new DataStoreItem(obj_path, level);
  log << MSG::INFO << "Adding OutputStream item " << item->path()
      << " with " << item->depth()
      << " level(s)." << endmsg;
  m_itemList.push_back( item );
}
