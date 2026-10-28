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
// Local includes
#include "OutputStream.h"

// Framework includes
#include <GaudiKernel/AttribStringParser.h>
#include <GaudiKernel/GenericAddress.h>
#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataSourceMgr.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/NTuple.h>
#include <GaudiKernel/SmartDataPtr.h>
#include <GaudiKernel/SmartIF.h>

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
class TagCollectionStream : public OutputStream {
protected:
  Gaudi::Property<std::string> m_addrLeaf{ this, "AddressLeaf", "/Event",
                                           "name of the address leaf in the transient event store" };
  Gaudi::Property<std::string> m_addrColName{ this, "AddressColumn", "Address",
                                              "name of the address column of the tag collection" };
  Gaudi::Property<std::string> m_collSvcName{ this, "TagCollectionSvc", "NTupleSvc", "name of the collection service" };
  Gaudi::Property<bool>        m_objectsFirst{ this, "ObjectsFirst", true,
                                        "flag to indicate that the objects should be written first" };
  Gaudi::Property<std::string> m_tagName{ this, "Collection", "", "name of the tag collection in the transient store" };

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

// Define the algorithm factory for the standard output data writer
DECLARE_COMPONENT( TagCollectionStream )

static const char SEPARATOR = IDataProviderSvc::SEPARATOR;

// Standard Constructor
TagCollectionStream::TagCollectionStream( const std::string& nam, ISvcLocator* pSvc )
    : OutputStream( nam, pSvc ), m_addr{ new GenericAddress() } {}

// Connect address column, if not already connected
StatusCode TagCollectionStream::connectAddress() {
  NTuplePtr nt( m_collectionSvc, m_tagName );
  if ( !nt ) {
    error() << "Failed to connect to the tag collection " << m_tagName << endmsg;
    return StatusCode::FAILURE;
  }
  m_addrColumn = nt->find( m_addrColName );
  if ( m_addrColumn ) return StatusCode::SUCCESS;
  StatusCode sc = nt->addItem( m_addrColName, m_item );
  if ( sc.isSuccess() ) {
    m_addrColumn = m_item.operator->();
    return sc;
  }
  error() << "Failed to add the address column:" << m_addrColName << " to the tag collection " << m_tagName << endmsg;
  return sc;
}

// initialize data writer
StatusCode TagCollectionStream::initialize() {
  using Parser = Gaudi::Utils::AttribStringParser;
  std::string log_node, log_file, logical_name;
  m_collectionSvc = service( m_collSvcName, true );
  if ( m_collectionSvc ) {
    std::string tmp;
    for ( auto attrib : Parser( m_output ) ) {
      switch ( ::toupper( attrib.tag[0] ) ) {
      case 'C':
        m_tagName = std::move( attrib.value );
        break;
      case 'A':
        m_addrLeaf = std::move( attrib.value );
        break;
      default:
        tmp += attrib.tag + "='" + attrib.value + "' ";
        break;
      }
    }
    m_output = std::move( tmp );
    {
      auto&                  tag = m_tagName.value();
      std::string::size_type idx = ( tag[0] == SEPARATOR ) ? tag.find( SEPARATOR, 1 ) : 0;
      log_node                   = tag.substr( idx, tag.find( SEPARATOR, idx + 1 ) );
    }
    log_file = log_node + " " + m_output + " SHARED='YES'";
  }
  m_addrColumn = nullptr;                    // reset pointer to item column
  auto sc      = OutputStream::initialize(); // Now initialize the base class
  if ( sc.isSuccess() ) {
    auto src_mgr = m_collectionSvc.as<IDataSourceMgr>();
    if ( src_mgr ) {
      auto data_mgr = m_collectionSvc.as<IDataManagerSvc>();
      if ( data_mgr ) {
        logical_name = data_mgr->rootName();
        logical_name += SEPARATOR;
        logical_name += log_node;
        m_topLeafName =
            m_addrLeaf.value().substr( 0, m_addrLeaf.value().find( SEPARATOR, m_addrLeaf[0] == '/' ? 1 : 0 ) );
        m_isTopLeaf = m_addrLeaf == m_topLeafName;
        if ( src_mgr->isConnected( logical_name ) ) return sc;
        sc = src_mgr->connect( log_file );
        if ( sc.isSuccess() ) return sc;
      }
    }
  }
  error() << "Failed to initialize TagCollection Stream." << endmsg;
  return StatusCode::FAILURE;
}

// terminate data writer
StatusCode TagCollectionStream::finalize() {
  StatusCode status = OutputStream::finalize();
  m_collectionSvc.reset();
  m_addrColumn = nullptr;
  return status;
}

// Write tuple data
StatusCode TagCollectionStream::writeTuple() {
  StatusCode sc = m_collectionSvc->writeRecord( m_tagName );
  if ( !sc.isSuccess() ) {
    error() << "Failed to write tag collection " << m_tagName << ". "
            << "[Tuple write error]" << endmsg;
  }
  return sc;
}

// Write data objects
StatusCode TagCollectionStream::writeData() {
  StatusCode sc = OutputStream::writeObjects();
  if ( !sc.isSuccess() ) {
    error() << "Failed to write tag collection " << m_tagName << ". "
            << "[Object write error]" << endmsg;
  }
  return sc;
}

// Write full event record
StatusCode TagCollectionStream::writeRecord() {
  if ( writeTuple().isSuccess() ) { return m_objectsFirst ? StatusCode::SUCCESS : writeData(); }
  return StatusCode::FAILURE;
}

// Work entry point
StatusCode TagCollectionStream::writeObjects() {
  StatusCode status = !m_addrColumn ? connectAddress() : StatusCode::SUCCESS;
  if ( status.isSuccess() ) {
    status = m_objectsFirst ? writeData() : StatusCode::SUCCESS;
    if ( status.isSuccess() && !m_addrColName.empty() ) {
      SmartDataPtr<DataObject> top( eventSvc(), m_topLeafName );
      if ( !top ) {
        error() << "Failed to write tag collection " << m_tagName << ". " << m_topLeafName << " not found." << endmsg;
        return StatusCode::FAILURE;
      }
      IOpaqueAddress* pA = top->registry()->address();
      if ( !pA ) {
        error() << "Failed to write tag collection " << m_tagName << ". " << m_topLeafName << "'s address not found."
                << endmsg;
        return StatusCode::FAILURE;
      }
      std::string*   par  = const_cast<std::string*>( m_addr->par() );
      unsigned long* ipar = (unsigned long*)m_addr->ipar();
      m_addr->setClID( pA->clID() );
      m_addr->setSvcType( pA->svcType() );
      par[0]                                        = pA->par()[0];
      par[1]                                        = pA->par()[1];
      ipar[0]                                       = pA->ipar()[0];
      ipar[1]                                       = pA->ipar()[1];
      *(IOpaqueAddress**)( m_addrColumn->buffer() ) = m_addr.get();
      if ( m_isTopLeaf ) return writeRecord();
      // Handle redirection mode. Normal tag collection mode
      // is the same like leaving this out....
      SmartDataPtr<DataObject> leaf( eventSvc(), m_addrLeaf );
      if ( !leaf ) {
        error() << "Failed to write tag collection " << m_tagName << ". " << m_addrLeaf << " not found." << endmsg;
        return StatusCode::FAILURE;
      }
      IOpaqueAddress* redir = leaf->registry()->address();
      if ( !redir ) {
        error() << "Failed to write tag collection " << m_tagName << ". " << m_addrLeaf << "'s address not found."
                << endmsg;
        return StatusCode::FAILURE;
      }
      par[0]  = redir->par()[0];
      ipar[0] = ~0x0;
      ipar[1] = redir->ipar()[1];
      return writeRecord();
    }
  }
  return status;
}
