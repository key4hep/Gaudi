#ifndef GAUDISVC_PERSISTENCYSVC_TAGCOLLECTIONSTREAM_H
#define GAUDISVC_PERSISTENCYSVC_TAGCOLLECTIONSTREAM_H

// Required for inheritance
#include "GaudiKernel/NTuple.h"
#include "OutputStream.h"

// forward declarations
class GenericAddress;

/** @class TagCollectionStream TagCollectionStream.h PersistencySvc/TagCollectionStream.h
  *
  * Specialized output stream class for event tag collections, where the basic Event
  * entry point should be placed as well into the collection itself.
  * The TagCollectionStream is a rather specialized object combining features
  * of NTuple I/O and object I/O. The main working points are:
  *
  * 1) Write a "normal" tag collection. The address column
  *    [property AddressColumn, default:"Address"] is added to the Ntuple identified
  *    by its name [mandatory property Collection]. The address column is set to
  *    the opaque address of the specified leaf [property "AddressLeaf", default:"/Event"].
  *    The NTuple must be registered to the TES of the service
  *    [property: TagCollectionSvc, default:"NTupleSvc"].
  *    This is the normal mode event tag collections work.
  *
  * 2) Write a tag collection with "REDIRECTED INPUT".
  *    Example:
  *    The collection is created from an intermediate file (MINI DST), but the
  *    Address column of the NTuple should point to the file e.g. containing the
  *    raw data.
  *    - Set the property "AddressLeaf" to something different from "/Event"
  *      e.g. "/Event/DAQ/RawEvent"
  *
  *    Note: The actual address of "/Event" get changed. No other output should
  *    be created after writing such a tag collection.
  *
  * 3) Write an "EMBEDDED COLLECTION".
  *    In this mode the OutputStream and the NTuple stream are combined.
  *    depending on the property TagCollectionStream.ObjectsFirst
  *    the objects are written first and the the tags.
  *    The tag file and the file containing the objects may be identical.
  *
  * Note:
  * - Also if the OutputStream features are NOT used, the options must still
  *   be properly set.
  *
  * - Tags or objects are only written to the output stream if the event is
  *   accepted according to the output stream criteria (veto algorithms, etc.)
  *
  * - If the address column is not present in the N-tuple (ie. it was not added
  *   by the N-tuple creator), it is automatically added at the first write
  *   attempt.
  *
  * - If the logical N-tuple output file is not yet connected to the tuple
  *   service, it is automatically connected to the the same as the
  *   OutputStream device.
  *
  * Author:  M.Frank
  * Version: 1.0
  */
class TagCollectionStream : public OutputStream
{
protected:
  Gaudi::Property<std::string> m_addrLeaf{this, "AddressLeaf", "/Event",
                                          "name of the address leaf in the transient event store"};
  Gaudi::Property<std::string> m_addrColName{this, "AddressColumn", "Address",
                                             "name of the address column of the tag collection"};
  Gaudi::Property<std::string> m_collSvcName{this, "TagCollectionSvc", "NTupleSvc", "name of the collection service"};
  Gaudi::Property<bool> m_objectsFirst{this, "ObjectsFirst", true,
                                       "flag to indicate that the objects should be written first"};
  Gaudi::Property<std::string> m_tagName{this, "Collection", "", "name of the tag collection in the transient store"};

  /// NTuple column to hold the opaque address of the address leaf
  INTupleItem* m_addrColumn = nullptr;
  /// Name of the top leaf (performance cache)
  std::string m_topLeafName;
  /// Short cut flag to indicate if the address leaf is the top leaf (performace cache)
  bool m_isTopLeaf = false;
  /// Keep reference to the tuple service
  SmartIF<INTupleSvc> m_collectionSvc;
  /// Address buffer
  std::unique_ptr<GenericAddress> m_addr;
  /// Address item buffer
  NTuple::Item<IOpaqueAddress*> m_item;
  /// OutputStream override: Select the different objects and write them to file
  StatusCode writeObjects() override;
  /// Connect address column, if not already connected
  virtual StatusCode connectAddress();
  /// Write data objects
  StatusCode writeData();
  /// Write tuple data
  StatusCode writeTuple();
  /// Write full event record
  StatusCode writeRecord();

public:
  /// Standard algorithm Constructor
  TagCollectionStream( const std::string& name, ISvcLocator* pSvcLocator );
  /// Initialize TagCollectionStream
  StatusCode initialize() override;
  /// Terminate TagCollectionStream
  StatusCode finalize() override;
};

#endif // GAUDISVC_PERSISTENCYSVC_TAGCOLLECTIONSTREAM_H
