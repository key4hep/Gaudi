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
#include <string>
#include <vector>

// Required for inheritance
#include "GaudiKernel/Algorithm.h"

/** A small to stream Data I/O.
    Author:  M.Frank
    Version: 1.0
*/
class EvtCollectionStream : public Algorithm {
protected:
  Gaudi::Property<std::vector<std::string>> m_itemNames{this, "ItemList", {}, "vector of item names"};
  Gaudi::Property<std::string>              m_storeName{this, "EvtDataSvc", "TagCollectionSvc",
                                           "name of the service managing the data store"};

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

#endif // GAUDISVC_PERSISTENCYSVC_EVTCOLLECTIONSTREAM_H
