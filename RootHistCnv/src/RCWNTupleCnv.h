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
#pragma once

// Include files
#include "RNTupleCnv.h"
#include <Gaudi/NTuple/DiskBuffer.h>
#include <GaudiKernel/Converter.h>

#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>

namespace RootHistCnv {

  /** @class RootHistCnv::RCWNTupleCnv RCWNTupleCnv.h
   *
   * Converter of Column-wise NTuple into ROOT format
   * @author Charles Leggett
   */

  class RCWNTupleCnv : public RNTupleCnv {
  public:
    /// Inquire class type
    static const CLID& classID() { return CLID_ColumnWiseTuple; }
    /// Standard constructor
    RCWNTupleCnv( ISvcLocator* svc ) : RNTupleCnv( svc, classID() ) {}

    StatusCode initialize() override;
    StatusCode finalize() override;
    /// Write the TTree; for a disk-buffered tuple, first build it from the buffer file.
    StatusCode updateRep( IOpaqueAddress* pAddr, DataObject* pObj ) override;

    /// How the items of a tuple map onto the staging buffer and the TTree branches.
    struct Layout {
      struct Item {
        std::string name;     ///< variable name (leaf name)
        std::string block;    ///< block name
        std::string leaflist; ///< ROOT leaf list, e.g. "x[n][3]/F"
        long        bufPos{ 0 };
        long        bufLen{ 0 };
        long        rangeLower{ 0 };
        long        rangeUpper{ -1 };
        long        indexPos{ -1 }; ///< position in items() of the counting item, -1 if fixed size
        long        unitBytes{ 0 }; ///< bytes per unit of the index
        long        maxUnits{ 0 };  ///< bufLen / unitBytes
      };
      std::vector<Item> items;
      long              rowBytes{ 0 };
    };

  protected:
    /// Create the transient representation of an object.
    StatusCode load( TTree* tree, INTuple*& refpObject ) override;
    /// Book the N tuple
    StatusCode book( const std::string& desc, INTuple* pObject, TTree*& tree ) override;
    /// Write N tuple data
    StatusCode writeData( TTree* rtree, INTuple* pObject ) override;
    /// Read N tuple data
    StatusCode readData( TTree* rtree, INTuple* pObject, long ievt ) override;

  private:
    struct Buffered {
      std::string                                        desc;
      Layout                                             layout;
      std::unique_ptr<Gaudi::NTuple::DiskBuffer::Writer> writer;
    };

    Layout     analyse( const INTuple& nt );
    StatusCode createTree( const std::string& desc, INTuple* nt, const Layout& layout, TTree*& rtree );
    StatusCode bookBuffered( const std::string& desc, INTuple* nt, Layout layout );
    StatusCode writeBuffered( Buffered& b, INTuple* nt );
    StatusCode replay( IOpaqueAddress* pAddr, INTuple* nt, Buffered& b );
    StatusCode bufferDirectory( std::filesystem::path& dir );
    void       releaseBufferDir();
    Buffered*  buffered( INTuple* nt );
    bool       replayed( INTuple* nt );

    std::mutex                   m_mutex; ///< guards m_buffered, m_replayed and m_bufferDir
    std::map<INTuple*, Buffered> m_buffered;
    std::set<INTuple*>           m_replayed; ///< tuples whose tree has been built; cannot be written again
    std::filesystem::path        m_bufferDir;
    unsigned                     m_nextBufferId{ 0 };
    bool                         m_bookDirect{ false }; ///< force the columnar path in book()

    /// NTupleSvc.DiskBuffer* of the service owning this stream, read once in initialize()
    bool        m_diskBuffer{ false };
    std::string m_diskBufferDirectory;
    int         m_diskBufferLevel{ 0 };
    std::size_t m_diskBufferBlockSize{ 0 };
  };
} // namespace RootHistCnv
