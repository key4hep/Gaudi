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
#include <GaudiKernel/PathResolver.h>
#include <GaudiKernel/System.h>

#ifdef WIN32
// Disable warning
//   C4996: 'std::copy': Function call with parameters that may be unsafe
// Probably coming from Boost classification.
#  pragma warning( disable : 4996 )
#endif

#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;
using namespace std;

#ifdef _WIN32
static const char* path_separator = ",;";
#else
static const char* path_separator = ",:";
#endif

//
///////////////////////////////////////////////////////////////////////////
//

namespace System {

  typedef enum { PR_regular_file, PR_directory } PR_file_type;

  typedef enum { PR_local, PR_recursive } PR_search_type;

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  static bool PR_find( const bf::path& file, const string& search_list, PR_file_type file_type,
                       PathResolver::SearchType search_type, string& result ) {

    bool found( false );

    // look for file as specified first

    try {
      if ( ( file_type == PR_regular_file && is_regular_file( file ) ) ||
           ( file_type == PR_directory && is_directory( file ) ) ) {
        result = bf::system_complete( file ).string();
        return true;
      }
    } catch ( const bf::filesystem_error& /*err*/ ) {}

    // assume that "." is always part of the search path, so check locally first

    try {
      bf::path local = bf::initial_path() / file;
      if ( ( file_type == PR_regular_file && is_regular_file( local ) ) ||
           ( file_type == PR_directory && is_directory( local ) ) ) {
        result = bf::system_complete( file ).string();
        return true;
      }
    } catch ( const bf::filesystem_error& /*err*/ ) {}

    // iterate through search list
    vector<string> spv;
    split( spv, search_list, boost::is_any_of( path_separator ), boost::token_compress_on );
    for ( const auto& itr : spv ) {

      bf::path fp = itr / file;

      try {
        if ( ( file_type == PR_regular_file && is_regular_file( fp ) ) ||
             ( file_type == PR_directory && is_directory( fp ) ) ) {
          result = bf::system_complete( fp ).string();
          return true;
        }
      } catch ( const bf::filesystem_error& /*err*/ ) {}

      // if recursive searching requested, drill down
      if ( search_type == PathResolver::RecursiveSearch && is_directory( bf::path( itr ) ) ) {

        bf::recursive_directory_iterator end_itr;
        try {
          for ( bf::recursive_directory_iterator ritr( itr ); ritr != end_itr; ++ritr ) {

            // skip if not a directory
            if ( !is_directory( bf::path( *ritr ) ) ) { continue; }

            bf::path fp2 = bf::path( *ritr ) / file;
            if ( ( file_type == PR_regular_file && is_regular_file( fp2 ) ) ||
                 ( file_type == PR_directory && is_directory( fp2 ) ) ) {
              result = bf::system_complete( fp2 ).string();
              return true;
            }
          }
        } catch ( const bf::filesystem_error& /*err*/ ) {}
      }
    }

    return found;
  }

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  string PathResolver::find_file( const std::string& logical_file_name, const std::string& search_path,
                                  SearchType search_type ) {

    std::string path_list;
    System::getEnv( search_path, path_list );

    return ( find_file_from_list( logical_file_name, path_list, search_type ) );
  }

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  std::string PathResolver::find_file_from_list( const std::string& logical_file_name, const std::string& search_list,
                                                 SearchType search_type ) {
    std::string result( "" );

    bf::path lfn( logical_file_name );

    /* bool found = */
    PR_find( lfn, search_list, PR_regular_file, search_type, result );

    // The following functionality was in the original PathResolver, but I believe
    // that it's WRONG. It extracts the filename of the requested item, and searches
    // for that if the preceding search fails. i.e., if you're looking for "B/a.txt",
    // and that fails, it will look for just "a.txt" in the search list.

    // if (! found && lfn.filename() != lfn ) {
    //   result = "";
    //   PR_find (lfn.filename(), search_list, PR_regular_file, search_type, result);
    // }

    return ( result );
  }

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  string PathResolver::find_directory( const std::string& logical_file_name, const std::string& search_path,
                                       SearchType search_type ) {
    std::string path_list;
    System::getEnv( search_path, path_list );

    return ( find_directory_from_list( logical_file_name, path_list, search_type ) );
  }

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  string PathResolver::find_directory_from_list( const std::string& logical_file_name, const std::string& search_list,
                                                 SearchType search_type ) {
    std::string result;

    if ( !PR_find( logical_file_name, search_list, PR_directory, search_type, result ) ) { result = ""; }

    return ( result );
  }

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  PathResolver::SearchPathStatus PathResolver::check_search_path( const std::string& search_path ) {
    std::string path_list;
    if ( !System::getEnv( search_path, path_list ) ) return ( EnvironmentVariableUndefined );

    vector<string> spv;
    boost::split( spv, path_list, boost::is_any_of( path_separator ), boost::token_compress_on );

    try {
      for ( const auto& itr : spv ) {
        bf::path pp( itr );
        if ( !is_directory( pp ) ) { return ( UnknownDirectory ); }
      }
    } catch ( const bf::filesystem_error& /*err*/ ) { return ( UnknownDirectory ); }

    return ( Ok );
  }

  /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

  std::string PathResolverFindXMLFile( const std::string& logical_file_name ) {
    return PathResolver::find_file( logical_file_name, "XMLPATH" );
  }

  std::string PathResolverFindDataFile( const std::string& logical_file_name ) {
    return PathResolver::find_file( logical_file_name, "DATAPATH" );
  }

} // namespace System
