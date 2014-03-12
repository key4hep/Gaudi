//	====================================================================
//  EvtCollectionStream.h
//	--------------------------------------------------------------------
//
//	Package   : GaudiSvc/PersistencySvc
//
//	Author    : Markus Frank
//
//	====================================================================
#ifndef GAUDISVC_PERSISTENCYSVC_EVTCOLLECTIONSTREAM_H
#define GAUDISVC_PERSISTENCYSVC_EVTCOLLECTIONSTREAM_H

// STL include files
#include <memory>
#include <vector>
#include <string>

// Required for inheritance
#include "GaudiKernel/Algorithm.h"


/** A small to stream Data I/O.
    Author:  M.Frank
    Version: 1.0
*/
class EvtCollectionStream : public Algorithm     {
  typedef std::vector<std::string>    ItemNames;
  typedef std::vector<DataStoreItem*> Items;
protected:
  /// Reference to Tuple service for event collection (may or may not be NTuple service)
  SmartIF<INTupleSvc> m_pTupleSvc;
  /// Name of the service managing the data store
  std::string   m_storeName;
  /// Vector of item names
  ItemNames     m_itemNames;
  /// Vector of items to be saved to this stream
  Items         m_itemList;
public:
  /// Standard algorithm Constructor
  EvtCollectionStream(const std::string& name, ISvcLocator* pSvcLocator);
protected:
  /// Standard Destructor
  virtual ~EvtCollectionStream();
  /// Clear item list
  void clearItems();
  /// Add item to output streamer list
  void addItem(const std::string& descriptor);
public:
  /// Initialize EvtCollectionStream
  virtual StatusCode initialize();
  /// Terminate EvtCollectionStream
  virtual StatusCode finalize();
  /// Working entry point
  virtual StatusCode execute();
};

#endif // GAUDISVC_PERSISTENCYSVC_EVTCOLLECTIONSTREAM_H
