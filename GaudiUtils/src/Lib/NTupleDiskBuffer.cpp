/***********************************************************************************\
* (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/NTuple/DiskBuffer.h>

#include <cerrno>
#include <cstring>
#include <unistd.h>

#ifdef GAUDI_USE_ZSTD
#  include <zstd.h>
#endif

namespace {
  constexpr char          MAGIC[8]        = { 'G', 'a', 'u', 'd', 'i', 'N', 'T', 'B' };
  constexpr std::uint32_t BYTE_ORDER_MARK = 0x01020304u;
  constexpr std::uint32_t VERSION         = 1;
  constexpr std::uint32_t CODEC_NONE      = 0;
  constexpr std::uint32_t CODEC_ZSTD      = 1;
  constexpr std::size_t   READ_BUFFER     = 64 * 1024;
  constexpr std::size_t   MAX_RECORD      = std::size_t( 1 ) << 30; // keeps a block and its compressed bound in uint32
  constexpr std::size_t   MAX_BLOCK       = MAX_RECORD;             // a block is at most this plus one record
  constexpr std::size_t   MAX_HEADER      = 64 * 1024 * 1024;
  // a block is one block size plus the record that filled it; anything larger is a corrupt header
  constexpr std::uint64_t MAX_STORED   = std::uint64_t( MAX_BLOCK ) + MAX_RECORD + sizeof( std::uint32_t );
  constexpr std::size_t   PREAMBLE     = sizeof( MAGIC ) + 4 * sizeof( std::uint32_t );
  constexpr std::size_t   BLOCK_HEADER = 2 * sizeof( std::uint32_t );

  void          put32( char* dest, std::uint32_t v ) { std::memcpy( dest, &v, sizeof( v ) ); }
  std::uint32_t get32( const char* src ) {
    std::uint32_t v;
    std::memcpy( &v, src, sizeof( v ) );
    return v;
  }
} // namespace

using namespace Gaudi::NTuple::DiskBuffer;

std::size_t Gaudi::NTuple::DiskBuffer::maxBlockSize() { return MAX_BLOCK; }

bool Gaudi::NTuple::DiskBuffer::zstdAvailable() {
#ifdef GAUDI_USE_ZSTD
  return true;
#else
  return false;
#endif
}

Error::Error( const std::filesystem::path& path, std::string_view what, int err )
    : std::runtime_error( std::string( what ) + ": " + path.string() +
                          ( err ? std::string( " (" ) + std::strerror( err ) + ")" : "" ) )
    , m_path( path )
    , m_code( err ) {}

// ---------------------------------------------------------------------------
Writer::Writer( std::filesystem::path path, std::string_view header, int zstdLevel, std::size_t blockSize )
    : m_path( std::move( path ) ), m_level( zstdLevel ), m_blockSize( blockSize ) {
  if ( header.size() > MAX_HEADER ) throw Error( m_path, "header too large" );
  if ( m_level != 0 && !zstdAvailable() ) throw Error( m_path, "zstd compression is not available in this build" );
  if ( m_blockSize == 0 || m_blockSize > MAX_BLOCK ) throw Error( m_path, "invalid block size" );
  m_file.reset( std::fopen( m_path.c_str(), "wb" ) );
  if ( !m_file ) throw Error( m_path, "cannot create buffer file", errno );
  std::setvbuf( m_file.get(), nullptr, _IONBF, 0 ); // m_buf is the buffer
  std::vector<char> preamble( MAGIC, MAGIC + sizeof( MAGIC ) );
  char              words[4 * sizeof( std::uint32_t )];
  put32( words, BYTE_ORDER_MARK );
  put32( words + 4, VERSION );
  put32( words + 8, m_level ? CODEC_ZSTD : CODEC_NONE );
  put32( words + 12, static_cast<std::uint32_t>( header.size() ) );
  preamble.insert( preamble.end(), words, words + sizeof( words ) );
  preamble.insert( preamble.end(), header.begin(), header.end() );
  write( preamble.data(), preamble.size() );
  m_stored = 0; // count from here
  // m_buf grows with the data: reserving m_blockSize up front costs a full block per tuple whether it fills or not
}

Writer::~Writer() = default; // m_file closes itself; close() is the checked path

void Writer::beginRecord() {
  if ( m_inRecord ) throw Error( m_path, "beginRecord called inside a record" );
  m_recordStart = m_buf.size();
  m_buf.resize( m_buf.size() + sizeof( std::uint32_t ) ); // length, patched by endRecord
  m_inRecord = true;
}

void Writer::append( const void* data, std::size_t bytes ) {
  if ( !m_inRecord ) throw Error( m_path, "append called outside a record" );
  if ( bytes == 0 ) return;
  if ( m_buf.size() - m_recordStart - sizeof( std::uint32_t ) + bytes > MAX_RECORD )
    throw Error( m_path, "record too large" );
  const char* src = static_cast<const char*>( data );
  m_buf.insert( m_buf.end(), src, src + bytes );
}

void Writer::endRecord() {
  if ( !m_inRecord ) throw Error( m_path, "endRecord called outside a record" );
  const std::size_t payload = m_buf.size() - m_recordStart - sizeof( std::uint32_t );
  put32( m_buf.data() + m_recordStart, static_cast<std::uint32_t>( payload ) );
  m_inRecord = false;
  ++m_pending;
  m_pendingBytes += payload;
  if ( m_buf.size() >= m_blockSize ) flush();
}

void Writer::write( const void* data, std::size_t bytes ) {
  if ( std::fwrite( data, 1, bytes, m_file.get() ) != bytes ) throw Error( m_path, "write failed", errno );
  m_stored += bytes;
}

// Write m_buf as one block. Only called between records, so a block holds whole records.
void Writer::flush() {
  if ( !m_file ) throw Error( m_path, "buffer file is closed" );
  if ( m_buf.empty() ) return;
  // dropped on failure too: the data is lost either way, do not let the buffer grow
  auto drop = [this] {
    m_buf.clear();
    m_pending = m_pendingBytes = 0;
    // a single huge record should not pin its memory for the rest of the job
    if ( m_buf.capacity() > 4 * m_blockSize ) m_buf.shrink_to_fit();
  };
  const long          start       = std::ftell( m_file.get() );
  const std::uint64_t storedStart = m_stored;
  try {
    const char*       data   = m_buf.data();
    std::size_t       stored = m_buf.size();
    std::vector<char> out;
#ifdef GAUDI_USE_ZSTD
    if ( m_level != 0 ) {
      out.resize( ZSTD_compressBound( m_buf.size() ) );
      stored = ZSTD_compress( out.data(), out.size(), m_buf.data(), m_buf.size(), m_level );
      if ( ZSTD_isError( stored ) )
        throw Error( m_path, std::string( "zstd compression failed: " ) + ZSTD_getErrorName( stored ) );
      data = out.data();
    }
#endif
    char hdr[BLOCK_HEADER];
    put32( hdr, static_cast<std::uint32_t>( stored ) );
    put32( hdr + 4, static_cast<std::uint32_t>( m_buf.size() ) );
    write( hdr, sizeof( hdr ) );
    write( data, stored );
  } catch ( ... ) {
    // cut the half-written block off, so the blocks before it stay readable
    if ( start >= 0 && ::ftruncate( ::fileno( m_file.get() ), start ) == 0 ) {
      std::fseek( m_file.get(), 0, SEEK_END );
      m_stored = storedStart;
    }
    drop();
    throw;
  }
  m_entries += m_pending;
  m_bytes += m_pendingBytes;
  drop();
}

void Writer::close() {
  if ( !m_file ) return;
  if ( m_inRecord ) throw Error( m_path, "close called inside a record" );
  flush();
  bool ok  = std::fflush( m_file.get() ) == 0;
  int  err = ok ? 0 : errno;
  // the block is the buffer, so a deferred write error (quota, NFS) only shows up here
  if ( std::fclose( m_file.release() ) != 0 && ok ) {
    ok  = false;
    err = errno;
  }
  if ( !ok ) throw Error( m_path, "close failed", err );
}

// ---------------------------------------------------------------------------
Reader::Reader( std::filesystem::path path ) : m_path( std::move( path ) ) {
  m_file.reset( std::fopen( m_path.c_str(), "rb" ) );
  if ( !m_file ) throw Error( m_path, "cannot open buffer file", errno );
  std::setvbuf( m_file.get(), nullptr, _IOFBF, READ_BUFFER );
  std::error_code ec;
  m_size = std::filesystem::file_size( m_path, ec );
  if ( ec ) m_size = 0;
  char preamble[PREAMBLE];
  if ( read( preamble, PREAMBLE ) != PREAMBLE || std::memcmp( preamble, MAGIC, sizeof( MAGIC ) ) != 0 )
    throw Error( m_path, "not a Gaudi ntuple buffer file" );
  const char* words = preamble + sizeof( MAGIC );
  if ( get32( words ) != BYTE_ORDER_MARK ) throw Error( m_path, "buffer file has foreign byte order" );
  if ( get32( words + 4 ) != VERSION ) throw Error( m_path, "unsupported buffer file version" );
  switch ( get32( words + 8 ) ) {
  case CODEC_NONE:
    break;
  case CODEC_ZSTD:
    if ( !zstdAvailable() ) throw Error( m_path, "compressed buffer file but zstd is not available in this build" );
    m_compressed = true;
    break;
  default:
    throw Error( m_path, "unknown buffer file compression" );
  }
  const std::uint32_t headerLen = get32( words + 12 );
  if ( headerLen > MAX_HEADER ) throw Error( m_path, "corrupt header length" );
  m_header.resize( headerLen );
  if ( read( m_header.data(), headerLen ) != headerLen ) throw Error( m_path, "truncated header" );
}

Reader::~Reader() = default;

std::size_t Reader::read( void* dest, std::size_t bytes ) {
  const std::size_t got = std::fread( dest, 1, bytes, m_file.get() );
  if ( got != bytes && std::ferror( m_file.get() ) ) throw Error( m_path, "read failed", errno );
  return got;
}

bool Reader::readBlock() {
  char              hdr[BLOCK_HEADER];
  const std::size_t got = read( hdr, sizeof( hdr ) );
  if ( got == 0 ) return false;
  if ( got != sizeof( hdr ) ) throw Error( m_path, "truncated block header" );
  const std::uint32_t stored = get32( hdr ), raw = get32( hdr + 4 );
  // check the lengths before allocating: a corrupt header must not turn into a huge resize
  if ( stored > MAX_STORED || raw > MAX_STORED ) throw Error( m_path, "corrupt block header" );
  const long pos = std::ftell( m_file.get() );
  if ( m_size > 0 && pos >= 0 && stored > m_size - static_cast<std::uintmax_t>( pos ) )
    throw Error( m_path, "truncated block" );
  m_pos = 0;
  if ( !m_compressed ) {
    if ( stored != raw ) throw Error( m_path, "corrupt block header" );
    if ( m_block.size() < raw ) m_block.resize( raw );
    if ( read( m_block.data(), raw ) != raw ) throw Error( m_path, "truncated block" );
    m_block.resize( raw );
    return true;
  }
#ifdef GAUDI_USE_ZSTD
  if ( m_stored.size() < stored ) m_stored.resize( stored );
  if ( read( m_stored.data(), stored ) != stored ) throw Error( m_path, "truncated block" );
  // the frame knows what it unpacks to: trust that over the block header
  const unsigned long long content = ZSTD_getFrameContentSize( m_stored.data(), stored );
  if ( content != ZSTD_CONTENTSIZE_UNKNOWN && content != ZSTD_CONTENTSIZE_ERROR && content != raw )
    throw Error( m_path, "corrupt block header" );
  if ( m_block.size() < raw ) m_block.resize( raw );
  const std::size_t n = ZSTD_decompress( m_block.data(), raw, m_stored.data(), stored );
  if ( ZSTD_isError( n ) ) throw Error( m_path, std::string( "corrupt block: " ) + ZSTD_getErrorName( n ) );
  if ( n != raw ) throw Error( m_path, "corrupt block: size mismatch" );
  m_block.resize( raw );
  return true;
#else
  throw Error( m_path, "compressed buffer file but zstd is not available in this build" );
#endif
}

bool Reader::next( std::span<const char>& record ) {
  while ( m_pos == m_block.size() ) {
    if ( !readBlock() ) return false;
  }
  if ( m_block.size() - m_pos < sizeof( std::uint32_t ) ) throw Error( m_path, "corrupt block: bad record length" );
  const std::uint32_t len = get32( m_block.data() + m_pos );
  m_pos += sizeof( std::uint32_t );
  if ( m_block.size() - m_pos < len ) throw Error( m_path, "corrupt block: record crosses block end" );
  record = std::span<const char>( m_block.data() + m_pos, len );
  m_pos += len;
  ++m_entries;
  return true;
}
