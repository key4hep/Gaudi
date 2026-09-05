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

#if __has_include( <catch2/catch.hpp>)
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_test_macros.hpp>
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <unistd.h>
#include <vector>

namespace {
  namespace fs = std::filesystem;
  using namespace Gaudi::NTuple::DiskBuffer;

  struct TempFile {
    fs::path path{ fs::temp_directory_path() / ( "ntdiskbuffer-" + std::to_string( ::getpid() ) + ".bin" ) };
    ~TempFile() { fs::remove( path ); }
  };

  std::vector<std::string> readAll( const fs::path& path ) {
    Reader                   r( path );
    std::vector<std::string> out;
    std::span<const char>    rec;
    while ( r.next( rec ) ) out.emplace_back( rec.data(), rec.size() );
    CHECK( r.entries() == out.size() );
    return out;
  }

  // levels to run the format tests with: 0 always, zstd only when built in
  std::vector<int> levels() { return zstdAvailable() ? std::vector<int>{ 0, 1, 3 } : std::vector<int>{ 0 }; }
} // namespace

TEST_CASE( "NTuple::DiskBuffer round trip" ) {
  for ( int level : levels() ) {
    TempFile tmp;
    {
      Writer w( tmp.path, "header text", level );
      CHECK( w.entries() == 0 );
      CHECK( w.zstdLevel() == level );
      w.beginRecord();
      w.append( "abc", 3 );
      w.append( "de", 2 );
      w.endRecord();
      w.beginRecord(); // empty record
      w.endRecord();
      std::string big( 3 * 64 * 1024 + 17, 'x' ); // larger than one flush chunk
      w.beginRecord();
      w.append( big.data(), big.size() );
      w.endRecord();
      CHECK( w.entries() == 3 );
      CHECK( w.bytes() == 5 + big.size() );
      w.close();
      w.close(); // idempotent
      if ( level ) CHECK( w.storedBytes() < w.bytes() / 10 );
    }
    Reader r( tmp.path );
    CHECK( r.header() == "header text" );
    CHECK( r.compressed() == ( level != 0 ) );
    auto rows = readAll( tmp.path );
    REQUIRE( rows.size() == 3 );
    CHECK( rows[0] == "abcde" );
    CHECK( rows[1].empty() );
    CHECK( rows[2].size() == 3 * 64 * 1024 + 17 );
  }
}

TEST_CASE( "NTuple::DiskBuffer many small records" ) {
  for ( int level : levels() ) {
    TempFile tmp;
    {
      Writer w( tmp.path, "", level );
      for ( int i = 0; i < 100000; ++i ) {
        w.beginRecord();
        w.append( &i, sizeof( i ) );
        w.endRecord();
      }
      w.close();
    }
    Reader                r( tmp.path );
    std::span<const char> rec;
    int                   n = 0;
    while ( r.next( rec ) ) {
      REQUIRE( rec.size() == sizeof( int ) );
      int v;
      std::memcpy( &v, rec.data(), sizeof( v ) );
      CHECK( v == n );
      ++n;
    }
    CHECK( n == 100000 );
  }
}

TEST_CASE( "NTuple::DiskBuffer block size" ) {
  // records are grouped into blocks of at least blockSize bytes; a record never straddles two
  for ( int level : levels() ) {
    TempFile tmp;
    {
      Writer w( tmp.path, "", level, 1000 );
      CHECK( w.blockSize() == 1000 );
      for ( int i = 0; i < 500; ++i ) {
        std::string rec( 300, char( 'a' + i % 26 ) );
        w.beginRecord();
        w.append( rec.data(), rec.size() );
        w.endRecord();
      }
      w.close();
      // 4 records (1216 bytes with the lengths) per block, 125 blocks
      if ( !level ) CHECK( w.storedBytes() == 125 * ( 8 + 4 * 304 ) );
    }
    auto rows = readAll( tmp.path );
    REQUIRE( rows.size() == 500 );
    for ( int i = 0; i < 500; ++i ) CHECK( rows[i] == std::string( 300, char( 'a' + i % 26 ) ) );
  }
}

TEST_CASE( "NTuple::DiskBuffer truncated tail" ) {
  // a partial block is lost as a whole, the blocks before it are fine
  for ( int level : levels() ) {
    TempFile tmp;
    {
      Writer      w( tmp.path, "h", level, 64 * 1024 );
      std::string big( 64 * 1024, 'x' ); // fills a block on its own
      w.beginRecord();
      w.append( big.data(), big.size() );
      w.endRecord();
      for ( int i = 0; i < 3; ++i ) {
        w.beginRecord();
        w.append( "0123456789", 10 );
        w.endRecord();
      }
      w.close();
    }
    fs::resize_file( tmp.path, fs::file_size( tmp.path ) - 4 ); // cut into the last block
    Reader                r( tmp.path );
    std::span<const char> rec;
    CHECK( r.next( rec ) );
    CHECK( rec.size() == 64 * 1024 );
    CHECK_THROWS_AS( r.next( rec ), Error );
    CHECK( r.entries() == 1 );
  }
}

TEST_CASE( "NTuple::DiskBuffer errors" ) {
  TempFile tmp;
  SECTION( "unwritable path" ) {
    fs::path bad = tmp.path / "no-such-dir" / "x.bin";
    try {
      Writer w( bad, "" );
      FAIL( "expected an exception" );
    } catch ( const Error& e ) {
      CHECK( e.path() == bad );
      CHECK( e.code() != 0 );
    }
  }
  SECTION( "misuse" ) {
    CHECK_THROWS_AS( Writer( tmp.path, "", 0, 0 ), Error );
    Writer w( tmp.path, "" );
    CHECK_THROWS_AS( w.append( "x", 1 ), Error );
    CHECK_THROWS_AS( w.endRecord(), Error );
    w.beginRecord();
    CHECK_THROWS_AS( w.beginRecord(), Error );
    CHECK_THROWS_AS( w.close(), Error );
  }
  SECTION( "not a buffer file" ) {
    {
      std::FILE* f = std::fopen( tmp.path.c_str(), "wb" );
      std::fputs( "hello", f );
      std::fclose( f );
    }
    CHECK_THROWS_AS( Reader( tmp.path ), Error );
  }
  SECTION( "block size too large" ) { CHECK_THROWS_AS( Writer( tmp.path, "", 0, maxBlockSize() + 1 ), Error ); }
  SECTION( "corrupt block length" ) {
    {
      Writer w( tmp.path, "" );
      w.beginRecord();
      w.append( "abc", 3 );
      w.endRecord();
      w.close();
    }
    // the first block header follows the 24-byte preamble of a file with an empty header
    std::FILE*          f       = std::fopen( tmp.path.c_str(), "r+b" );
    const std::uint32_t huge[2] = { 0xFFFFFFFFu, 0xFFFFFFFFu };
    std::fseek( f, 24, SEEK_SET );
    std::fwrite( huge, sizeof( huge ), 1, f );
    std::fclose( f );
    CHECK_THROWS_AS( readAll( tmp.path ), Error ); // reported, not resized to 4 GiB
  }
  SECTION( "compression not built in" ) {
    if ( !zstdAvailable() ) CHECK_THROWS_AS( Writer( tmp.path, "", 3 ), Error );
  }
}
