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
#pragma once

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

/** Append-only file of length-prefixed records, used to park ntuple rows on disk
 *  during the event loop and replay them at finalize.
 *
 *  Layout: 8-byte magic, uint32 byte-order mark, uint32 format version, uint32
 *  compression (0 none, 1 zstd), uint32 header length, header bytes (opaque to this
 *  class), then blocks of `uint32 stored length | uint32 raw length | bytes` until end
 *  of file. A block holds whole records, `uint32 length | payload` each, and is one
 *  zstd frame when compression is on. Nothing here knows what a row is; the caller
 *  decides.
 */
namespace Gaudi::NTuple::DiskBuffer {

  /// Whether this build can write and read compressed blocks.
  bool zstdAvailable();

  /// Largest block size a Writer accepts.
  std::size_t maxBlockSize();

  namespace details {
    struct Close {
      void operator()( std::FILE* f ) const { std::fclose( f ); }
    };
  } // namespace details
  /// Owns an open file; close() is the checked path, this is the fallback.
  using FileHandle = std::unique_ptr<std::FILE, details::Close>;

  /// Any I/O or format problem, carrying the file it happened on.
  class Error : public std::runtime_error {
  public:
    Error( const std::filesystem::path& path, std::string_view what, int err = 0 );
    const std::filesystem::path& path() const { return m_path; }
    int                          code() const { return m_code; }

  private:
    std::filesystem::path m_path;
    int                   m_code;
  };

  class Writer {
  public:
    /// Create (truncating) `path` and write the preamble and `header`.
    /// `zstdLevel` 0 stores the blocks as they are; anything else needs zstdAvailable().
    /// A block is written once `blockSize` bytes of records have accumulated, so that is
    /// the memory held per open file and what zstd sees at a time.
    Writer( std::filesystem::path path, std::string_view header, int zstdLevel = 0, std::size_t blockSize = 64 * 1024 );
    ~Writer();
    Writer( const Writer& )            = delete;
    Writer& operator=( const Writer& ) = delete;

    void beginRecord();
    void append( const void* data, std::size_t bytes );
    void endRecord();

    /// Flush and close; throws on failure. Safe to call twice.
    void close();

    /// Records written to the file; records lost to a failed flush do not count.
    std::uint64_t entries() const { return m_entries; }
    /// Payload bytes of those records
    std::uint64_t bytes() const { return m_bytes; }
    /// Bytes written to the file after the header
    std::uint64_t                storedBytes() const { return m_stored; }
    int                          zstdLevel() const { return m_level; }
    std::size_t                  blockSize() const { return m_blockSize; }
    const std::filesystem::path& path() const { return m_path; }

  private:
    void flush();
    void write( const void* data, std::size_t bytes );

    std::filesystem::path m_path;
    FileHandle            m_file;
    int                   m_level;
    std::size_t           m_blockSize;
    std::vector<char>     m_buf;
    std::size_t           m_recordStart{ 0 }; // offset of the current record's length field
    bool                  m_inRecord{ false };
    std::uint64_t         m_entries{ 0 };
    std::uint64_t         m_bytes{ 0 };
    std::uint64_t         m_stored{ 0 };
    std::uint64_t         m_pending{ 0 };      // records in m_buf, counted once their block is written
    std::uint64_t         m_pendingBytes{ 0 }; // and their payload
  };

  class Reader {
  public:
    /// Open `path` and validate the preamble.
    explicit Reader( std::filesystem::path path );
    ~Reader();
    Reader( const Reader& )            = delete;
    Reader& operator=( const Reader& ) = delete;

    const std::string& header() const { return m_header; }
    bool               compressed() const { return m_compressed; }

    /// Read the next record; false at a clean end of file. A partial block throws.
    bool next( std::span<const char>& record );

    std::uint64_t                entries() const { return m_entries; }
    const std::filesystem::path& path() const { return m_path; }

  private:
    std::size_t read( void* dest, std::size_t bytes );
    bool        readBlock();

    std::filesystem::path m_path;
    FileHandle            m_file;
    std::uintmax_t        m_size{ 0 }; // file size, 0 if unknown: bounds a block length
    std::string           m_header;
    bool                  m_compressed{ false };
    std::vector<char>     m_stored; // one block as read from the file
    std::vector<char>     m_block;  // the same block unpacked
    std::size_t           m_pos{ 0 };
    std::uint64_t         m_entries{ 0 };
  };

} // namespace Gaudi::NTuple::DiskBuffer
