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
#include "Parser.h"
#include "Grammar.h"
#include "IncludedFiles.h"
#include "Iterator.h"
#include "Messages.h"
#include "Node.h"
#include "Utils.h"
#include <GaudiKernel/PathResolver.h>
#include <boost/filesystem.hpp>
#include <fmt/format.h>
#include <fstream>
#include <sstream>

// ============================================================================
namespace classic = boost::spirit::classic;
namespace bf      = boost::filesystem;
namespace gp      = Gaudi::Parsers;
namespace gpu     = Gaudi::Parsers::Utils;
namespace qi      = boost::spirit::qi;
// ============================================================================
namespace {

  // Return last line and column number of text in `s` with newline delimiter `delim`
  std::pair<int, int> GetLastLineAndColumn( std::string_view s, const char delim = '\n' ) {
    size_t line = 1;
    for ( size_t p = s.find( delim ); p != s.npos; p = s.find( delim ) ) {
      s.remove_prefix( p + 1 );
      ++line;
    }
    return { line, s.size() + 1 };
  }

  template <typename Grammar>
  bool ParseStream( std::ifstream& stream, const std::string& stream_name, gp::Messages* messages, gp::Node* root ) {
    // Load input stream
    const std::string input = ( std::ostringstream{} << stream.rdbuf() ).str();

    auto [last_line, last_column] = GetLastLineAndColumn( input );

    BaseIterator in_begin( input.begin() );
    // convert input iterator to forward iterator, usable by spirit parser
    ForwardIterator fwd_begin = boost::spirit::make_default_multi_pass( in_begin );
    ForwardIterator fwd_end;

    // wrap forward iterator with position iterator, to record the position
    Iterator position_begin( fwd_begin, fwd_end, stream_name );
    Iterator position_end;

    Grammar                      gr;
    gp::SkipperGrammar<Iterator> skipper;

    root->value = stream_name;
    bool result = qi::phrase_parse( position_begin, position_end, gr, skipper, *root );

    const IteratorPosition& pos = position_begin.get_position();
    if ( result && ( pos.line == last_line ) && ( pos.column == last_column ) ) { return true; }

    messages->AddError( gp::Position( stream_name, pos.line, pos.column ), "parse error" );
    return false;
  }

  // ============================================================================
  template <typename Grammar>
  bool ParseFile( const gp::Position& from, std::string_view filename, std::string_view search_path,
                  gp::IncludedFiles* included, gp::Messages* messages, gp::Node* root ) {
    std::string search_path_with_current_dir = gpu::replaceEnvironments( search_path );
    if ( !from.filename().empty() ) { // Add current file directory to search_path
      bf::path file_path( from.filename() );
      search_path_with_current_dir =
          file_path.parent_path().string() +
          ( search_path_with_current_dir.empty() ? "" : ( "," + search_path_with_current_dir ) );
    }
    std::string absolute_path =
        System::PathResolver::find_file_from_list( gpu::replaceEnvironments( filename ), search_path_with_current_dir );

    if ( absolute_path.empty() ) {
      messages->AddError( from, "Couldn't find a file " + std::string{ filename } + " in search path '" +
                                    search_path_with_current_dir + "'" );
      return false;
    }
    const gp::Position* included_from;
    if ( !included->GetPosition( absolute_path, &included_from ) ) {
      included->AddFile( absolute_path, from );
      std::ifstream file{ absolute_path };
      if ( !file.is_open() ) {
        messages->AddError( from, std::string{ "Couldn't open a file " }.append( filename ) );
        return false;
      }
      return ParseStream<Grammar>( file, absolute_path, messages, root );
    } else {
      assert( included_from != nullptr );
      messages->AddWarning(
          from, fmt::format( "File {} already included from {}", absolute_path, included_from->ToString() ) );
      return true;
    }
  }
  // ============================================================================
} // namespace
// ============================================================================
bool gp::Parse( std::string_view filename, std::string_view search_path, IncludedFiles* included, Messages* messages,
                Node* root ) {
  return Parse( Position(), filename, search_path, included, messages, root );
}
// ============================================================================
bool gp::Parse( const Position& from, std::string_view filename, std::string_view search_path, IncludedFiles* included,
                Messages* messages, Node* root ) {
  using Grammar = FileGrammar<Iterator, SkipperGrammar<Iterator>>;
  return ParseFile<Grammar>( from, filename, search_path, included, messages, root );
}

// ============================================================================
bool gp::ParseUnits( const Position& from, std::string_view filename, std::string_view search_path,
                     IncludedFiles* included, Messages* messages, Node* root ) {
  using Grammar = UnitsGrammar<Iterator, SkipperGrammar<Iterator>>;
  return ParseFile<Grammar>( from, filename, search_path, included, messages, root );
}
// ============================================================================
