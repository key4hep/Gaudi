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
#define ROOTHISTCNV_RCWNTUPLECNV_CPP

// Include files
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/INTupleSvc.h>
#include <GaudiKernel/IRegistry.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/NTuple.h>

// Compiler include files
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <list>
#include <numeric>
#include <set>
#include <span>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

#include "RCWNTupleCnv.h"

#include <fmt/format.h>

#include <TLeafD.h>
#include <TLeafF.h>
#include <TLeafI.h>
#include <TTree.h>
#include <TUrl.h>

namespace {
  namespace fs         = std::filesystem;
  namespace DiskBuffer = Gaudi::NTuple::DiskBuffer;
  using Layout         = RootHistCnv::RCWNTupleCnv::Layout;

  template <typename T>
  size_t saveItem( char* target, const NTuple::_Data<T>& src ) {
    static_assert( std::is_trivially_copyable_v<T>, "T must be trivally copyable" );
    std::memcpy( target, src.buffer(), sizeof( T ) * src.length() );
    return sizeof( T ) * src.length();
  }

  template <typename T>
  size_t loadItem( const char* src, NTuple::_Data<T>& target ) {
    static_assert( std::is_trivially_copyable_v<T>, "T must be trivally copyable" );
    std::memcpy( const_cast<void*>( target.buffer() ), src, sizeof( T ) * target.length() );
    return sizeof( T ) * target.length();
  }

  template <typename POD>
  decltype( auto ) downcast_item( const INTupleItem& i ) {
    return dynamic_cast<const NTuple::_Data<POD>&>( i );
  }
  template <typename POD>
  decltype( auto ) downcast_item( INTupleItem& i ) {
    return dynamic_cast<NTuple::_Data<POD>&>( i );
  }
  template <typename POD, typename T>
  void downcast_item( T&& ) = delete;

  template <typename Item, typename F>
  decltype( auto ) visit( Item& i, F&& f ) {
    switch ( i.type() ) {
    case DataTypeInfo::INT:
      return f( downcast_item<int>( i ) );
    case DataTypeInfo::CHAR:
      return f( downcast_item<char>( i ) );
    case DataTypeInfo::SHORT:
      return f( downcast_item<short>( i ) );
    case DataTypeInfo::LONG:
      return f( downcast_item<long>( i ) );
    case DataTypeInfo::LONGLONG:
      return f( downcast_item<long long>( i ) );
    case DataTypeInfo::UCHAR:
      return f( downcast_item<unsigned char>( i ) );
    case DataTypeInfo::USHORT:
      return f( downcast_item<unsigned short>( i ) );
    case DataTypeInfo::UINT:
      return f( downcast_item<unsigned int>( i ) );
    case DataTypeInfo::ULONG:
      return f( downcast_item<unsigned long>( i ) );
    case DataTypeInfo::ULONGLONG:
      return f( downcast_item<unsigned long long>( i ) );
    case DataTypeInfo::DOUBLE:
      return f( downcast_item<double>( i ) );
    case DataTypeInfo::FLOAT:
      return f( downcast_item<float>( i ) );
    case DataTypeInfo::BOOL:
      return f( downcast_item<bool>( i ) );
    }
    throw std::runtime_error( "RCWNTupleCnv::visit: unknown INTupleItem::type()" );
  }

  //-----------------------------------------------------------------------------
  template <class T>
  void analyzeItem( const std::string& typ, const NTuple::_Data<T>* it, std::string& desc, std::string& block_name,
                    std::string& var_name, long& lowerRange, long& upperRange, long& size )
  //-----------------------------------------------------------------------------
  {

    RootHistCnv::parseName( it->name(), block_name, var_name );

    // long item_size = (sizeof(T) < 4) ? 4 : sizeof(T);
    long item_size = sizeof( T );
    long dimension = it->length();
    long ndim      = it->ndim() - 1;
    desc += var_name;
    if ( it->hasIndex() || it->length() > 1 ) { desc += '['; }
    if ( it->hasIndex() ) {
      std::string ind_blk, ind_var;
      RootHistCnv::parseName( it->index(), ind_blk, ind_var );
      if ( ind_blk != block_name ) {
        std::cerr << "ERROR: Index for CWNT variable " << ind_var << " is in a different block: " << ind_blk
                  << std::endl;
      }
      desc += ind_var;
    } else if ( it->dim( ndim ) > 1 ) {
      desc += std::to_string( it->dim( ndim ) );
    }

    for ( int i = ndim - 1; i >= 0; i-- ) {
      desc += "][";
      desc += std::to_string( it->dim( i ) );
    }
    if ( it->hasIndex() || it->length() > 1 ) { desc += ']'; }

    // 0 and -1 are used to mark that the range is not defined
    lowerRange = 0;
    upperRange = -1;
    if constexpr ( std::is_integral_v<T> ) {
      // An explicit range makes sense only for integral types so we check only in that case if it is defined.
      // Note that later a range is only taken into account for int32_t.
      if ( it->range().lower() != it->range().min() && it->range().upper() != it->range().max() ) {
        lowerRange = it->range().lower();
        upperRange = it->range().upper();
      }
    }

    desc += typ;
    size += item_size * dimension;
  }

  // --- disk buffer helpers ----------------------------------------------------

  /// Value of the index item `index`, read from `p` (its own buffer or its slot in a row).
  template <typename T>
  long indexValue( const NTuple::_Data<T>&, const void* p ) {
    T v;
    std::memcpy( &v, p, sizeof( T ) );
    return static_cast<long>( v );
  }
  long indexValue( const INTupleItem& index, const void* p ) {
    return visit( index, [p]( const auto& d ) { return indexValue( d, p ); } );
  }

  /// Units of `li` present in a record; the single definition shared by pack and unpack.
  long usedUnits( const Layout::Item& li, const INTupleItem& index, const void* indexData ) {
    return li.indexPos < 0 ? li.maxUnits : std::clamp( indexValue( index, indexData ), 0L, li.maxUnits );
  }

  /// Pack the current item values into one record.
  void packRow( DiskBuffer::Writer& w, const Layout& layout, const INTuple::ItemContainer& items ) {
    w.beginRecord();
    for ( size_t k = 0; k < items.size(); ++k ) {
      const auto& li = layout.items[k];
      const long n = li.indexPos < 0 ? li.maxUnits : usedUnits( li, *items[li.indexPos], items[li.indexPos]->buffer() );
      w.append( items[k]->buffer(), n * li.unitBytes );
    }
    w.endRecord();
  }

  /// Unpack one record into the staging buffer `buf`; slots beyond the stored part come from `defaults`.
  void unpackRow( std::span<const char> rec, const Layout& layout, const INTuple::ItemContainer& items, char* buf,
                  const char* defaults ) {
    const char* src  = rec.data();
    std::size_t left = rec.size();
    for ( size_t k = 0; k < items.size(); ++k ) {
      const auto& li = layout.items[k];
      // an index always precedes the items it counts, so its slot is already unpacked
      const long n =
          li.indexPos < 0 ? li.maxUnits : usedUnits( li, *items[li.indexPos], buf + layout.items[li.indexPos].bufPos );
      const std::size_t bytes = n * li.unitBytes;
      if ( bytes > left ) throw std::runtime_error( "record shorter than its layout" );
      std::memcpy( buf + li.bufPos, src, bytes );
      std::memcpy( buf + li.bufPos + bytes, defaults + li.bufPos + bytes, li.bufLen - bytes );
      src += bytes;
      left -= bytes;
    }
    if ( left != 0 ) throw std::runtime_error( "record longer than its layout" );
  }

  /// Text header of a buffer file: enough to rebuild the tree without the job.
  std::string describe( const std::string& id, const INTuple& nt, const Layout& layout ) {
    std::ostringstream os;
    os << "id " << id << "\ntitle " << nt.title() << "\nrowbytes " << layout.rowBytes << "\nitems "
       << layout.items.size() << "\n# block name leaflist type bufPos bufLen unitBytes indexPos\n";
    for ( size_t k = 0; k < layout.items.size(); ++k ) {
      const auto& li = layout.items[k];
      os << li.block << ' ' << li.name << ' ' << li.leaflist << ' ' << nt.items()[k]->type() << ' ' << li.bufPos << ' '
         << li.bufLen << ' ' << li.unitBytes << ' ' << li.indexPos << '\n';
    }
    return os.str();
  }

  // buffer directories in use by any output stream of this process
  std::mutex         s_dirsMutex;
  std::set<fs::path> s_claimedDirs;
} // namespace

//-----------------------------------------------------------------------------
RootHistCnv::RCWNTupleCnv::Layout RootHistCnv::RCWNTupleCnv::analyse( const INTuple& nt )
//-----------------------------------------------------------------------------
{
  MsgStream   log( msgSvc(), "RCWNTupleCnv" );
  Layout      layout;
  const auto& items = nt.items();
  layout.items.reserve( items.size() );
  long size = 0;

  for ( const auto& i : items ) {
    Layout::Item li;
    const long   oldsize = size;

    visit( *i, [&]( const auto& data ) {
      analyzeItem( this->rootVarType( data.type() ), &data, li.leaflist, li.block, li.name, li.rangeLower,
                   li.rangeUpper, size );
    } );

    li.bufPos = oldsize;
    li.bufLen = size - oldsize;
    if ( i->hasIndex() ) {
      auto idx    = std::find( items.begin(), items.end(), i->indexItem() );
      li.indexPos = idx != items.end() ? idx - items.begin() : -1;
    }
    const long elemBytes = i->length() > 0 ? i->size() / i->length() : 0;
    li.unitBytes         = elemBytes * ( i->ndim() == 2 ? i->dim( 0 ) : 1 ); // a matrix is counted in columns
    li.maxUnits          = li.unitBytes > 0 ? li.bufLen / li.unitBytes : 0;

    log << MSG::VERBOSE << "item: " << li.leaflist << " type " << i->type() << " blk: " << li.block
        << " var: " << li.name << " rng: " << li.rangeLower << " " << li.rangeUpper << " sz: " << size << " "
        << li.bufLen << " buf_pos: " << li.bufPos << endmsg;

    layout.items.push_back( std::move( li ) );
  }
  layout.rowBytes = size;
  return layout;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::createTree( const std::string& desc, INTuple* nt, const Layout& layout,
                                                  TTree*& rtree )
//-----------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), "RCWNTupleCnv" );
  rtree = new TTree( desc.c_str(), nt->title().c_str() );
  log << MSG::VERBOSE << "created tree id: " << rtree->GetName() << "  title: " << nt->title() << " desc: " << desc
      << endmsg;

  // Make a new buffer, and tell the ntuple where it is
  const long size = layout.rowBytes;
  char*      buff = nt->setBuffer( new char[size] );

  log << MSG::VERBOSE << "Created buffer size: " << size << " at " << (void*)buff << endmsg;

  // Zero out the buffer to make ROOT happy
  std::fill_n( buff, size, 0 );

  Gaudi::Property<int> basket_size( "BasketSize", 32000 );
  m_ntupleSvc.as<IProperty>()->getProperty( &basket_size ).ignore();

  // Loop over items, creating a new branch for each one;
  for ( const auto& li : layout.items ) {

    char* buf_pos = buff + li.bufPos;

    auto br = new TBranch( rtree, li.name.c_str(), buf_pos, li.leaflist.c_str(), basket_size );
    if ( li.block != "AUTO_BLK" ) {
      std::string title = li.block + "::" + br->GetTitle();
      br->SetTitle( title.c_str() );
    }

    log << MSG::DEBUG << "adding TBranch  " << br->GetTitle() << "  at " << (void*)buf_pos << endmsg;

    // for index items with a limited range. Must be a TLeafI!
    if ( li.rangeLower < li.rangeUpper ) {
      TLeafI*  index = nullptr;
      TObject* tobj  = br->GetListOfLeaves()->FindObject( li.name.c_str() );
      if ( tobj->IsA()->InheritsFrom( "TLeafI" ) ) {
        index = dynamic_cast<TLeafI*>( tobj );

        if ( index ) {
          index->SetMaximum( li.rangeUpper );
          // FIXME -- add for next version of ROOT
          // index->SetMinimum( li.rangeLower );
        } else {
          log << MSG::ERROR << "Could dynamic cast to TLeafI: " << li.name << endmsg;
        }
      }
    }

    rtree->GetListOfBranches()->Add( br );
  }

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::initialize()
//-----------------------------------------------------------------------------
{
  StatusCode sc = RNTupleCnv::initialize();
  if ( !sc ) return sc;
  MsgStream log( msgSvc(), "RCWNTupleCnv" );

  // the DiskBuffer properties belong to the tuple service owning this stream, which is our data provider
  SmartIF<IProperty> owner = dataProvider().as<IProperty>();
  if ( !owner ) owner = m_ntupleSvc.as<IProperty>(); // not owned by a tuple service: keep the lookup by name
  Gaudi::Property<bool>        diskBuffer( "DiskBuffer", false );
  Gaudi::Property<std::string> directory( "DiskBufferDirectory", "" );
  Gaudi::Property<int>         level( "DiskBufferCompression", 3 );
  Gaudi::Property<int>         blockSize( "DiskBufferBlockSize", 64 * 1024 );
  if ( owner ) {
    for ( auto* p :
          std::initializer_list<Gaudi::Details::PropertyBase*>{ &diskBuffer, &directory, &level, &blockSize } )
      owner->getProperty( p ).ignore();
  }

  m_diskBuffer          = diskBuffer;
  m_diskBufferDirectory = directory;
  m_diskBufferLevel     = level;
  m_diskBufferBlockSize = static_cast<std::size_t>( blockSize.value() );
  if ( !m_diskBuffer ) return sc;

  if ( m_diskBufferLevel != 0 && !DiskBuffer::zstdAvailable() ) {
    log << MSG::WARNING << "DiskBuffer: zstd not available in this build, not compressing" << endmsg;
    m_diskBufferLevel = 0;
  }
  if ( blockSize <= 0 || m_diskBufferBlockSize > DiskBuffer::maxBlockSize() ) {
    log << MSG::ERROR << "DiskBuffer: DiskBufferBlockSize must be between 1 and " << DiskBuffer::maxBlockSize()
        << endmsg;
    return StatusCode::FAILURE;
  }
  return sc;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::book( const std::string& desc, INTuple* nt, TTree*& rtree )
//-----------------------------------------------------------------------------
{
  rtree         = nullptr;
  Layout layout = analyse( *nt );

  if ( m_diskBuffer && !m_bookDirect ) return bookBuffered( desc, nt, std::move( layout ) );

  StatusCode sc = createTree( desc, nt, layout, rtree );
  if ( sc ) {
    MsgStream log( msgSvc(), "RCWNTupleCnv" );
    log << MSG::INFO << "Booked TTree with ID: " << desc << " \"" << nt->title() << "\" in directory " << getDirectory()
        << endmsg;
  }
  return sc;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::bufferDirectory( fs::path& dir )
//-----------------------------------------------------------------------------
{
  if ( !m_bufferDir.empty() ) {
    dir = m_bufferDir;
    return StatusCode::SUCCESS;
  }
  MsgStream log( msgSvc(), "RCWNTupleCnv" );

  TFile* file = gDirectory ? gDirectory->GetFile() : nullptr;
  if ( !file ) {
    log << MSG::ERROR << "DiskBuffer: " << getDirectory() << " is not inside a file" << endmsg;
    return StatusCode::FAILURE;
  }
  TUrl     url( file->GetName(), kTRUE );
  fs::path base;
  if ( m_diskBufferDirectory.empty() && std::string_view( url.GetProtocol() ) == "file" ) {
    base = url.GetFile();
  } else {
    base =
        fs::path( m_diskBufferDirectory.empty() ? "." : m_diskBufferDirectory ) / fs::path( url.GetFile() ).filename();
  }
  base += ".ntbuf";

  {
    std::lock_guard lock( s_dirsMutex );
    if ( !s_claimedDirs.insert( base ).second ) {
      log << MSG::ERROR << "DiskBuffer directory " << base << " is already used by another output stream" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  std::error_code ec;
  if ( fs::exists( base, ec ) ) {
    log << MSG::WARNING << "Removing stale DiskBuffer directory " << base << endmsg;
    fs::remove_all( base, ec );
  }
  if ( !ec ) fs::create_directories( base, ec );
  if ( ec ) {
    log << MSG::ERROR << "Cannot create DiskBuffer directory " << base << ": " << ec.message() << endmsg;
    std::lock_guard lock( s_dirsMutex );
    s_claimedDirs.erase( base );
    return StatusCode::FAILURE;
  }
  m_bufferDir = base;
  dir         = base;
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::bookBuffered( const std::string& desc, INTuple* nt, Layout layout )
//-----------------------------------------------------------------------------
{
  MsgStream   log( msgSvc(), "RCWNTupleCnv" );
  const auto& items = nt->items();
  for ( size_t k = 0; k < items.size(); ++k ) {
    const auto& li = layout.items[k];
    if ( items[k]->hasIndex() && ( li.indexPos < 0 || li.indexPos >= long( k ) ) ) {
      log << MSG::ERROR << "DiskBuffer: index " << items[k]->index() << " must be added before " << items[k]->name()
          << " in " << desc << endmsg;
      return StatusCode::FAILURE;
    }
  }
  std::string id = desc;
  if ( auto obj = dynamic_cast<DataObject*>( nt ); obj && obj->registry() ) id = obj->registry()->identifier();

  std::lock_guard lock( m_mutex );
  fs::path        dir;
  if ( auto sc = bufferDirectory( dir ); !sc ) return sc;
  const fs::path path = dir / fmt::format( "{:04}.ntbuf", m_nextBufferId++ );

  Buffered b{ desc, std::move( layout ), nullptr };
  try {
    b.writer = std::make_unique<DiskBuffer::Writer>( path, describe( id, *nt, b.layout ), m_diskBufferLevel,
                                                     m_diskBufferBlockSize );
  } catch ( const DiskBuffer::Error& e ) {
    log << MSG::ERROR << e.what() << endmsg;
    return StatusCode::FAILURE;
  }
  log << MSG::INFO << "Buffering TTree with ID: " << desc << " \"" << nt->title() << "\" in directory "
      << getDirectory() << " to " << path.string() << endmsg;
  m_buffered.emplace( nt, std::move( b ) );
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
RootHistCnv::RCWNTupleCnv::Buffered* RootHistCnv::RCWNTupleCnv::buffered( INTuple* nt )
//-----------------------------------------------------------------------------
{
  std::lock_guard lock( m_mutex );
  auto            it = m_buffered.find( nt );
  return it != m_buffered.end() ? &it->second : nullptr;
}

//-----------------------------------------------------------------------------
bool RootHistCnv::RCWNTupleCnv::replayed( INTuple* nt )
//-----------------------------------------------------------------------------
{
  std::lock_guard lock( m_mutex );
  return m_replayed.count( nt ) != 0;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::writeData( TTree* rtree, INTuple* nt )
//-----------------------------------------------------------------------------
{
  if ( !rtree ) {
    if ( Buffered* b = buffered( nt ) ) return writeBuffered( *b, nt );
    MsgStream log( msgSvc(), "RCWNTupleCnv" );
    if ( replayed( nt ) ) {
      // save() built the tree and closed the buffer; there is nowhere to put this row
      log << MSG::ERROR << "N-tuple \"" << nt->title()
          << "\" was already written from its disk buffer: with NTupleSvc.DiskBuffer a tuple cannot be filled after "
             "it has been saved. Row lost."
          << endmsg;
    } else {
      log << MSG::ERROR << "No TTree and no disk buffer for N-tuple \"" << nt->title() << "\"" << endmsg;
    }
    return StatusCode::FAILURE;
  }

  // Fill the tree;
  const auto& items = nt->items();
  std::accumulate( begin( items ), end( items ), nt->buffer(), []( char* dest, const INTupleItem* i ) {
    return dest + visit( *i, [dest]( const auto& item ) { return saveItem( dest, item ); } );
  } );

  rtree->Fill();
  nt->reset();
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::writeBuffered( Buffered& b, INTuple* nt )
//-----------------------------------------------------------------------------
{
  const auto& items = nt->items();
  if ( items.size() != b.layout.items.size() ) {
    MsgStream log( msgSvc(), "RCWNTupleCnv" );
    log << MSG::ERROR << "N-tuple " << b.desc << " changed shape after its first write" << endmsg;
    return StatusCode::FAILURE;
  }
  try {
    packRow( *b.writer, b.layout, items );
  } catch ( const DiskBuffer::Error& e ) {
    MsgStream log( msgSvc(), "RCWNTupleCnv" );
    log << MSG::ERROR << "Disk buffer write failed: " << e.what() << endmsg;
    return StatusCode::FAILURE;
  }
  nt->reset();
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::updateRep( IOpaqueAddress* pAddr, DataObject* pObj )
//-----------------------------------------------------------------------------
{
  if ( !pAddr ) {
    // never written: the base class books it and writes one default row; keep that on the direct path
    m_bookDirect  = true;
    StatusCode sc = RNTupleCnv::updateRep( pAddr, pObj );
    m_bookDirect  = false;
    return sc;
  }
  INTuple*  nt = dynamic_cast<INTuple*>( pObj );
  Buffered* b  = nt ? buffered( nt ) : nullptr;
  if ( b ) return replay( pAddr, nt, *b );
  // an earlier save() already built and wrote the tree, there is nothing left to do
  if ( nt && replayed( nt ) ) return StatusCode::SUCCESS;
  return RNTupleCnv::updateRep( pAddr, pObj );
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::replay( IOpaqueAddress* pAddr, INTuple* nt, Buffered& b )
//-----------------------------------------------------------------------------
{
  MsgStream              log( msgSvc(), "RCWNTupleCnv" );
  GlobalDirectoryRestore restore;
  const auto             t0      = std::chrono::steady_clock::now();
  const std::string      desc    = b.desc;
  const fs::path         path    = b.writer->path();
  TDirectory*            pDir    = (TDirectory*)pAddr->ipar()[0];
  TTree*                 tree    = nullptr;
  StatusCode             sc      = StatusCode::SUCCESS;
  std::uint64_t          entries = 0, bytes = 0, stored = 0;

  try {
    b.writer->close();
    entries = b.writer->entries();
    bytes   = b.writer->bytes();
    stored  = b.writer->storedBytes();
    b.writer.reset(); // free its write cache before the tree is built
    if ( nt->items().size() != b.layout.items.size() )
      throw std::runtime_error( "N-tuple changed shape after its first write" );
    if ( !pDir ) throw std::runtime_error( "no output directory" );
    pDir->cd();
    sc = createTree( desc, nt, b.layout, tree );
    if ( sc ) {
      const auto& items = nt->items();
      char*       buf   = nt->buffer();
      // the part of a slot not stored in a record is whatever reset() leaves there
      nt->reset();
      std::vector<char> defaults( b.layout.rowBytes );
      for ( size_t k = 0; k < items.size(); ++k )
        std::memcpy( defaults.data() + b.layout.items[k].bufPos, items[k]->buffer(), b.layout.items[k].bufLen );

      DiskBuffer::Reader    reader( path );
      std::span<const char> rec;
      while ( reader.next( rec ) ) {
        unpackRow( rec, b.layout, items, buf, defaults.data() );
        tree->Fill();
      }
      if ( reader.entries() != entries )
        throw std::runtime_error( fmt::format( "expected {} entries, found {}", entries, reader.entries() ) );
      if ( tree->Write( "", TObject::kOverwrite ) == 0 ) throw std::runtime_error( "TTree::Write failed" );
    }
  } catch ( const std::exception& e ) {
    log << MSG::ERROR << "Failed to build TTree " << desc << " from disk buffer " << path.string() << ": " << e.what()
        << endmsg;
    sc = StatusCode::FAILURE;
  }
  delete tree;
  nt->setBuffer( nullptr );

  std::error_code ec;
  if ( sc ) fs::remove( path, ec );
  {
    std::lock_guard lock( m_mutex );
    m_buffered.erase( nt );
    m_replayed.insert( nt );
    if ( m_buffered.empty() ) {
      fs::remove( m_bufferDir, ec ); // only when empty, i.e. nothing was left behind for inspection
      releaseBufferDir();
    }
  }
  if ( sc ) {
    const double secs = std::chrono::duration<double>( std::chrono::steady_clock::now() - t0 ).count();
    log << MSG::INFO << "Built TTree with ID: " << desc << " \"" << nt->title() << "\" in directory " << pDir->GetPath()
        << " from disk buffer: " << entries << " entries, " << bytes << " bytes (" << stored << " on disk), "
        << fmt::format( "{:.2f}", secs ) << " s" << endmsg;
  }
  return sc;
}

//-----------------------------------------------------------------------------
void RootHistCnv::RCWNTupleCnv::releaseBufferDir()
//-----------------------------------------------------------------------------
{
  if ( m_bufferDir.empty() ) return;
  std::lock_guard lock( s_dirsMutex );
  s_claimedDirs.erase( m_bufferDir );
  m_bufferDir.clear();
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::finalize()
//-----------------------------------------------------------------------------
{
  {
    std::lock_guard lock( m_mutex );
    if ( !m_buffered.empty() ) {
      MsgStream log( msgSvc(), "RCWNTupleCnv" );
      for ( auto& [nt, b] : m_buffered ) {
        // flush first, so the file left behind holds every row that was written
        try {
          b.writer->close();
        } catch ( const DiskBuffer::Error& e ) { log << MSG::ERROR << e.what() << endmsg; }
        log << MSG::ERROR << "TTree " << b.desc << " was never built from its disk buffer " << b.writer->path().string()
            << endmsg;
      }
      m_buffered.clear();
    }
    m_replayed.clear();
    releaseBufferDir();
  }
  return RNTupleCnv::finalize();
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::readData( TTree* rtree, INTuple* ntup, long ievt )
//-----------------------------------------------------------------------------
{
  if ( !rtree ) {
    MsgStream log( msgSvc(), "RCWNTupleCnv::readData" );
    log << MSG::ERROR << "cannot read a disk-buffered N-tuple: its TTree is only built at finalize" << endmsg;
    return StatusCode::FAILURE;
  }
  if ( ievt >= rtree->GetEntries() ) {
    MsgStream log( msgSvc(), "RCWNTupleCnv::readData" );
    log << MSG::ERROR << "no more entries in tree to read. max: " << rtree->GetEntries() << "  current: " << ievt
        << endmsg;
    return StatusCode::FAILURE;
  }

  rtree->GetEvent( ievt );
  ievt++;

  // copy data from ntup->buffer() to ntup->items()->buffer()
  auto& items = ntup->items();
  std::accumulate( begin( items ), end( items ), const_cast<const char*>( ntup->buffer() ),
                   []( const char* src, INTupleItem* i ) {
                     return src + visit( *i, [src]( auto& item ) { return loadItem( src, item ); } );
                   } );

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
StatusCode RootHistCnv::RCWNTupleCnv::load( TTree* tree, INTuple*& refpObject )
//-----------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), "RCWNTupleCnv::load" );

  StatusCode status;

  NTuple::Tuple* pObj = nullptr;

  std::string title = tree->GetTitle();
  log << MSG::VERBOSE << "loading CWNT " << title << " at: " << tree << endmsg;

  status        = m_ntupleSvc->create( CLID_ColumnWiseTuple, title, pObj );
  INTuple* ntup = dynamic_cast<INTuple*>( pObj );
  if ( !ntup ) { log << MSG::ERROR << "cannot dynamic cast to INTuple" << endmsg; }

  INTupleItem* item = nullptr;

  std::string itemName, indexName, item_type, itemTitle, blockName;
  // long numEnt, numVar;
  long                                size, totsize = 0;
  std::vector<std::pair<TLeaf*, int>> itemList;

  // numEnt = (int)tree->GetEntries();
  // numVar = tree->GetNbranches();

  // loop over all branches (==leaves)
  TObjArray* lbr = tree->GetListOfBranches();
  TIter      bitr( lbr );
  while ( TObject* tobjb = bitr() ) {

    TBranch* br = dynamic_cast<TBranch*>( tobjb );
    itemTitle   = br->GetTitle();

    int ipos = itemTitle.find( "::" );
    if ( ipos >= 0 ) {
      blockName = itemTitle.substr( 0, ipos );
    } else {
      blockName = "";
    }

    TObjArray* lf = br->GetListOfLeaves();

    TIter litr( lf );
    while ( TObject* tobj = litr() ) {

      bool hasRange   = false;
      int  indexRange = 0;
      int  itemSize;
      item = nullptr;

      TLeaf* tl = dynamic_cast<TLeaf*>( tobj );
      if ( !tl ) {
        log << MSG::ERROR << "cannot dynamic cast to TLeaf" << endmsg;
        return StatusCode::FAILURE;
      }
      itemName = tl->GetName();

      if ( blockName != "" ) {
        log << MSG::DEBUG << "loading NTuple item " << blockName << "/" << itemName;
      } else {
        log << MSG::DEBUG << "loading NTuple item " << itemName;
      }

      int    arraySize{ 0 };
      TLeaf* indexLeaf = tl->GetLeafCounter( arraySize );

      if ( arraySize == 0 ) { log << MSG::ERROR << "TLeaf counter size = 0. This should not happen!" << endmsg; }

      if ( indexLeaf ) {
        // index Arrays and Matrices

        indexName  = indexLeaf->GetName();
        indexRange = indexLeaf->GetMaximum();
        itemSize   = indexRange * tl->GetLenType() * arraySize;

        log << "[" << indexName;

        // Just for Matrices
        if ( arraySize != 1 ) { log << "][" << arraySize; }
        log << "]";

      } else {
        itemSize = tl->GetLenType() * arraySize;

        indexName = "";

        if ( arraySize == 1 ) {
          // Simple items
        } else {
          // Arrays of constant size
          log << "[" << arraySize << "]";
        }
      }

      log << endmsg;

      size = itemSize;
      totsize += size;

      hasRange = tl->IsRange();

      itemList.emplace_back( tl, itemSize );

      // Integer
      if ( tobj->IsA()->InheritsFrom( "TLeafI" ) ) {

        TLeafI* tli = dynamic_cast<TLeafI*>( tobj );
        if ( tli ) {
          if ( tli->IsUnsigned() ) {
            unsigned long min = 0, max = 0;
            if ( hasRange ) {
              min = tli->GetMinimum();
              max = tli->GetMaximum();
            }

            item = createNTupleItem( itemName, blockName, indexName, indexRange, arraySize, min, max, ntup, hasRange );
          } else {
            long min = 0, max = 0;
            if ( hasRange ) {
              min = tli->GetMinimum();
              max = tli->GetMaximum();
            }

            item = createNTupleItem( itemName, blockName, indexName, indexRange, arraySize, min, max, ntup, hasRange );
          }
        } else {
          log << MSG::ERROR << "cannot dynamic cast to TLeafI" << endmsg;
        }

        // Float
      } else if ( tobj->IsA()->InheritsFrom( "TLeafF" ) ) {
        float min = 0., max = 0.;

        TLeafF* tlf = dynamic_cast<TLeafF*>( tobj );
        if ( tlf ) {
          if ( hasRange ) {
            min = float( tlf->GetMinimum() );
            max = float( tlf->GetMaximum() );
          }
        } else {
          log << MSG::ERROR << "cannot dynamic cast to TLeafF" << endmsg;
        }

        item = createNTupleItem( itemName, blockName, indexName, indexRange, arraySize, min, max, ntup, hasRange );

        // Double
      } else if ( tobj->IsA()->InheritsFrom( "TLeafD" ) ) {
        double min = 0., max = 0.;

        TLeafD* tld = dynamic_cast<TLeafD*>( tobj );
        if ( tld ) {
          if ( hasRange ) {
            min = tld->GetMinimum();
            max = tld->GetMaximum();
          }
        } else {
          log << MSG::ERROR << "cannot dynamic cast to TLeafD" << endmsg;
        }

        item = createNTupleItem( itemName, blockName, indexName, indexRange, arraySize, min, max, ntup, hasRange );

      } else {
        log << MSG::ERROR << "Uknown data type" << endmsg;
      }

      if ( item ) {
        ntup->add( item ).ignore();
      } else {
        log << MSG::ERROR << "Unable to create ntuple item \"" << itemName << "\"" << endmsg;
      }

    } // end litr
  }   // end bitr

  log << MSG::DEBUG << "Total buffer size of NTuple: " << totsize << " Bytes." << endmsg;

  char* buf    = ntup->setBuffer( new char[totsize] );
  char* bufpos = buf;

  int ts = 0;
  for ( const auto& iitr : itemList ) {
    TLeaf* leaf  = iitr.first;
    int    isize = iitr.second;

    log << MSG::VERBOSE << "setting TBranch " << leaf->GetBranch()->GetName() << " buffer at " << (void*)bufpos
        << endmsg;

    leaf->GetBranch()->SetAddress( (void*)bufpos );

    //        //testing
    //        if (leaf->IsA()->InheritsFrom("TLeafI")) {
    //  	for (int ievt=0; ievt<5; ievt++) {
    //  	  leaf->GetBranch()->GetEvent(ievt);
    //  	  int *idat = (int*)bufpos;
    //  	  log << MSG::WARNING << leaf->GetName() << ": " << ievt << "   "
    //  	      << *idat << endmsg;

    //  	}
    //        }

    ts += isize;

    bufpos += isize;
  }

  if ( totsize != ts ) { log << MSG::ERROR << "buffer size mismatch: " << ts << "  " << totsize << endmsg; }

  refpObject = ntup;

  return StatusCode::SUCCESS;
}

// Instantiation of a static factory class used by clients to create
// instances of this service
DECLARE_CONVERTER( RootHistCnv::RCWNTupleCnv )
