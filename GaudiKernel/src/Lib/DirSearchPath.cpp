/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifdef WIN32
// Disable warning
//    C4996: '...': Function call with parameters that may be unsafe
// Noise probably coming from the use of Boost tokenizer
#  pragma warning( disable : 4996 )
#endif

#include <algorithm> /* find */
#include <iostream>
#include <list>
#ifdef __ICC
// disable icc warning #279: controlling expression is constant
// ... a lot of noise produced by the boost/filesystem/operations.hpp
#  pragma warning( disable : 279 )
#endif
#include <GaudiKernel/DirSearchPath.h>
#include <boost/filesystem/operations.hpp>
#include <boost/tokenizer.hpp>

using namespace std;

using boost::filesystem::exists;
using boost::filesystem::filesystem_error;
using boost::filesystem::is_directory;

using boost::char_separator;
using boost::tokenizer;

// constructors
DirSearchPath::DirSearchPath( const std::string& stringifiedPath, const char* separator ) {
  addCWD(); // FIXME is this a good idea?

  typedef tokenizer<char_separator<char>> Tokenizer;

  Tokenizer tok( stringifiedPath, char_separator<char>( separator ) );

  // add names to dir container, filtering dir names to remove invalid ones
  // notice how we iterate over all tokens even if there is an illegal one
  auto it = tok.begin();
  while ( it != tok.end() ) {
    try {
      path p( *( it++ ) );
      add( p );
    } catch ( boost::filesystem::filesystem_error& /*err*/ ) {}
  }
}

// modifiers
bool DirSearchPath::addCWD() { return add( boost::filesystem::current_path() ); }

bool DirSearchPath::add( const path& dir ) {
  bool dirExist( existsDir( dir ) );
  // add dir to path even if dir does not (yet) exist,
  // but don't add twice same dir
  if ( m_dirs.end() == std::find_if( m_dirs.begin(), m_dirs.end(), eqPath( dir ) ) ) m_dirs.push_back( dir );
  return dirExist;
}

// accessors
bool DirSearchPath::find( const string& fileName, string& fullFileName ) const {
  bool rc( false );
  try {
    path fileFound;
    if ( ( rc = find( path( fileName ), fileFound ) ) ) fullFileName = fileFound.string();
  } catch ( ... ) {}
  return rc;
}

// accessors
bool DirSearchPath::find( const path& file, path& fileFound ) const {
  bool rc( false );
  for ( const auto& iDir : m_dirs ) {
    path full{ iDir / file };
    if ( exists( full ) ) {
      fileFound = full;
      rc        = true;
      break;
    }
  }
  return rc;
}

// accessors
std::list<DirSearchPath::path> DirSearchPath::find_all( const path& file ) const {
  std::list<path> found;
  for ( const auto& iDir : m_dirs ) {
    path full{ iDir / file };
    if ( exists( full ) ) found.push_back( full );
  }
  return found;
}

// helpers
bool DirSearchPath::existsDir( const std::string& dirName ) {
  bool rc( false );
  try {
    rc = is_directory( path( dirName ) );
  } catch ( ... ) {}
  return rc;
}
bool DirSearchPath::existsDir( const path& dir ) { return ( exists( dir ) && is_directory( dir ) ); }
