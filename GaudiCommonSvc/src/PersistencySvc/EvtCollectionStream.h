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
protected:
  /// Reference to Tuple service for event collection (may or may not be NTuple service)
  SmartIF<INTupleSvc> m_pTupleSvc;
  /// Name of the service managing the data store
  std::string   m_storeName;
  /// Vector of item names
  std::vector<std::string> m_itemNames;
  /// Vector of items to be saved to this stream
  std::vector<std::unique_ptr<DataStoreItem>> m_itemList;
public:
  /// Standard algorithm Constructor
  EvtCollectionStream(const std::string& name, ISvcLocator* pSvcLocator);
protected:
  /// Standard Destructor
  ~EvtCollectionStream() override = default;
  /// Clear item list
  void clearItems();
  /// Add item to output streamer list
  void addItem(const std::string& descriptor);
public:
  /// Initialize EvtCollectionStream
  StatusCode initialize() override;
  /// Terminate EvtCollectionStream
  StatusCode finalize() override;
  /// Working entry point
  StatusCode execute() override;
};

#endif // GAUDISVC_PERSISTENCYSVC_EVTCOLLECTIONSTREAM_H
