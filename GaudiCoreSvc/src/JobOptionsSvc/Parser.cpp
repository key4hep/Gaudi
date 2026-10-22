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
#include "Parser.h"
#include "IncludedFiles.h"
#include "Iterator.h"
#include "Messages.h"
#include "Node.h"
#include "Utils.h"

// Framework includes
#include <GaudiKernel/PathResolver.h>

// Boost includes
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/fusion.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/phoenix/stl.hpp>
#if ( BOOST_VERSION >= 187000 ) && ( BOOST_VERSION < 188000 )
#  define BOOST_ALLOW_DEPRECATED_HEADERS
#endif
#include <boost/spirit/include/qi.hpp>
#undef BOOST_ALLOW_DEPRECATED_HEADERS
#include <boost/spirit/repository/include/qi_confix.hpp>
#include <boost/spirit/repository/include/qi_iter_pos.hpp>

// STL includes
#include <boost/filesystem.hpp>
#include <format>
#include <fstream>
#include <sstream>
#include <string>

namespace Gaudi {
  namespace Parsers {
    namespace sp  = boost::spirit;
    namespace ph  = boost::phoenix;
    namespace qi  = sp::qi;
    namespace enc = sp::ascii;
    namespace rep = sp::repository;

    template <typename Iterator>
    struct SkipperGrammar : qi::grammar<Iterator> {
      SkipperGrammar() : SkipperGrammar::base_type( comments ) {
        comments = enc::space | rep::confix( "/*", "*/" )[*( qi::char_ - "*/" )] |
                   rep::confix( "//", ( sp::eol | sp::eoi ) )[*( qi::char_ - ( sp::eol | sp::eoi ) )];
      }
      qi::rule<Iterator> comments;
    };

    template <typename Iterator, typename Skipper>
    struct StringGrammar : qi::grammar<Iterator, std::string(), qi::locals<char>, Skipper> {
      typedef std::string ResultT;
      StringGrammar() : StringGrammar::base_type( str ) {
        begin_quote = enc::char_( "\"'" );
        quote       = enc::char_( qi::_r1 );

        str = qi::lexeme[begin_quote[qi::_a = qi::_1] >>
                         *( ( enc::char_( '\\' ) >> quote( qi::_a ) )[qi::_val += qi::_a] |
                            ( enc::char_[qi::_val += qi::_1] - quote( qi::_a ) ) ) >>
                         quote( qi::_a )];
      }
      qi::rule<Iterator, std::string(), qi::locals<char>, Skipper> str;
      qi::rule<Iterator, char()>                                   begin_quote;
      qi::rule<Iterator, void( char )>                             quote;
    };
    template <typename Iterator, typename Skipper>
    struct IdentifierGrammar : qi::grammar<Iterator, Node(), Skipper> {
      typedef std::string ResultT;
      IdentifierGrammar() : IdentifierGrammar::base_type( ident ) {
        ident =
            rep::qi::iter_pos[op( qi::_val, qi::_1 )] >> str[op( qi::_val, qi::_1 )][op( qi::_val, Node::kIdentifier )];
        str = -qi::lit( "::" )[qi::_val += "::"] >> inner[qi::_val += qi::_1] >>
              *( qi::lit( "::" ) >> inner[qi::_val += ( "::" + qi::_1 )] );
        inner = qi::alpha >> *( qi::alnum | qi::char_( '_' ) );
      }
      qi::rule<Iterator, Node(), Skipper>        ident;
      qi::rule<Iterator, std::string(), Skipper> str;
      qi::rule<Iterator, std::string()>          inner;
      ph::function<NodeOperations>               op;
    };
    template <typename Iterator, typename Skipper>
    struct BoolGrammar : qi::grammar<Iterator, bool(), Skipper> {
      typedef bool ResultT;
      BoolGrammar() : BoolGrammar::base_type( boolean ) {
        boolean = enc::no_case[qi::lit( "true" )[qi::_val = true] | qi::lit( "false" )[qi::_val = false]];
      }
      qi::rule<Iterator, bool(), Skipper> boolean;
    };
    template <typename Iterator, typename Skipper>
    struct RealGrammar : qi::grammar<Iterator, Node(), Skipper> {
      typedef bool ResultT;
      RealGrammar() : RealGrammar::base_type( real ) {
        real = qi::raw[qi::double_][op( qi::_val, qi::_1 )][op( qi::_val, Node::kReal )] >> -qi::char_( 'L' ) >>
               -( -qi::char_( '*' ) >> gunit[op( qi::_val, qi::_1 )] );
      }
      qi::rule<Iterator, Node(), Skipper>  real;
      IdentifierGrammar<Iterator, Skipper> gunit;
      ph::function<NodeOperations>         op;
    };
    template <typename Iterator, typename Skipper>
    struct UnitsGrammar : qi::grammar<Iterator, Node(), Skipper> {
      UnitsGrammar() : UnitsGrammar::base_type( units ) {
        units = *unit[op( qi::_val, qi::_1 )];
        unit  = rep::qi::iter_pos[op( qi::_val, qi::_1 )] >> val[op( qi::_val, qi::_1 )] >> -qi::lit( '*' ) >>
               gunit[op( qi::_val, qi::_1 )] >> qi::lit( '=' ) >>
               val[op( qi::_val, qi::_1 )][op( qi::_val, Node::kUnit )];
        ;
        val = qi::raw[qi::double_][op( qi::_val, qi::_1 )][op( qi::_val, Node::kReal )];
      }
      qi::rule<Iterator, Node(), Skipper>  units, unit, val;
      IdentifierGrammar<Iterator, Skipper> gunit;
      ph::function<NodeOperations>         op;
    };
    template <typename Iterator, typename Skipper>
    struct FileGrammar : qi::grammar<Iterator, Node(), Skipper> {
      FileGrammar() : FileGrammar::base_type( file ) {
        file  = -shell[op( qi::_val, qi::_1 )] >> *( statement[op( qi::_val, qi::_1 )] )[op( qi::_val, Node::kRoot )];
        shell = rep::confix(
            "#!",
            qi::eol )[*( qi::char_[qi::_a += qi::_1] - qi::eol )][op( qi::_val, Node::kShell )][op( qi::_val, qi::_a )];
        statement = rep::qi::iter_pos[qi::_a = qi::_1] >> ( include | assign | units | print_options | pragma |
                                                            condition )[qi::_val = qi::_1][op( qi::_val, qi::_a )];
        condition =
            ( qi::lit( "#ifdef" )[op( qi::_a, Node::kIfdef )] | qi::lit( "#ifndef" )[op( qi::_a, Node::kIfndef )] ) >>
            property[op( qi::_val, qi::_1 )] >> ( *statement[op( qi::_a, qi::_1 )] )[op( qi::_val, qi::_a )] >>
            -( qi::lit( "#else" )[op( qi::_b, Node::kElse )] >>
               *statement[op( qi::_b, qi::_1 )] )[op( qi::_val, qi::_b )] >>
            qi::lit( "#endif" )[op( qi::_val, Node::kCondition )];
        include          = qi::lit( "#include" ) >> gstring[op( qi::_val, qi::_1 )][op( qi::_val, Node::kInclude )];
        units            = qi::lit( "#units" ) >> gstring[op( qi::_val, qi::_1 )][op( qi::_val, Node::kUnits )];
        print_options    = qi::lit( "#printOptions" ) >> qi::lit( "full" )[op( qi::_val, Node::kPrintOptions )];
        pragma           = qi::lit( "#pragma" ) >> ( pragma_print | pragma_tree | pragma_dump_file );
        pragma_print     = qi::lit( "print" ) >> enc::no_case[qi::lit( "on" )[op( qi::_val, Node::kPrintOn )] |
                                                          qi::lit( "off" )[op( qi::_val, Node::kPrintOff )]];
        pragma_tree      = enc::no_case[qi::lit( "printtree" )[op( qi::_val, Node::kPrintTree )]];
        pragma_dump_file = qi::lit( "dumpfile" ) >> gstring[op( qi::_val, qi::_1 )][op( qi::_val, Node::kDumpFile )];
        assign = property[op( qi::_val, qi::_1 )][op( qi::_val, Node::kAssign )] >> oper[op( qi::_val, qi::_1 )] >>
                 value[op( qi::_val, qi::_1 )] >> ';';
        property = rep::qi::iter_pos[op( qi::_val, qi::_1 )] >>
                   ( gidentifier[op( qi::_val, qi::_1 )] % '.' )[op( qi::_val, Node::kProperty )];
        property_ref %= -qi::lit( '@' ) >> property[op( qi::_val, Node::kPropertyRef )];
        oper = rep::qi::iter_pos[op( qi::_val, qi::_1 )] >>
               ( qi::lit( "=" )[op( qi::_val, Node::kEqual )] | qi::lit( "+=" )[op( qi::_val, Node::kPlusEqual )] |
                 qi::lit( "-=" )[op( qi::_val, Node::kMinusEqual )] );
        value = rep::qi::iter_pos[qi::_a = qi::_1] >> ( map_value | vector_value | simple_value | property |
                                                        property_ref )[qi::_val = qi::_1][op( qi::_val, qi::_a )];
        begin_vector =
            enc::char_( '(' )[qi::_val = ')'] | enc::char_( '[' )[qi::_val = ']'] | enc::char_( '{' )[qi::_val = '}'];
        end_vector   = qi::char_( qi::_r1 );
        vector_value = ( begin_vector[qi::_a = qi::_1] >> -( value[op( qi::_val, qi::_1 )] % ',' ) >>
                         end_vector( qi::_a ) )[op( qi::_val, Node::kVector )];
        map_value    = ( enc::char_( '{' ) >> -( pair[op( qi::_val, qi::_1 )] % ',' ) >>
                      enc::char_( '}' ) )[op( qi::_val, Node::kMap )];
        pair =
            simple_value[op( qi::_val, qi::_1 )] >> ':' >> value[op( qi::_val, qi::_1 )][op( qi::_val, Node::kPair )];
        simple_value = ( gstring[op( qi::_val, qi::_1 )][op( qi::_val, Node::kString )] ) |
                       ( gbool[op( qi::_val, qi::_1 )][op( qi::_val, Node::kBool )] ) | ( greal[qi::_val = qi::_1] );
      }
      qi::rule<Iterator, Node(), Skipper> file, include, assign, property, property_ref, oper, map_value, pair_value,
          simple_value, pair, units, print_options, pragma, pragma_print, pragma_tree, pragma_dump_file;
      qi::rule<Iterator, Node(), qi::locals<std::string>>         shell;
      qi::rule<Iterator, Node(), qi::locals<Iterator>, Skipper>   statement, value;
      qi::rule<Iterator, Node(), qi::locals<char>, Skipper>       vector_value;
      qi::rule<Iterator, Node(), qi::locals<Node, Node>, Skipper> condition;
      qi::rule<Iterator, char()>                                  begin_vector;
      qi::rule<Iterator, void( char )>                            end_vector;
      StringGrammar<Iterator, Skipper>                            gstring;
      BoolGrammar<Iterator, Skipper>                              gbool;
      RealGrammar<Iterator, Skipper>                              greal;
      IdentifierGrammar<Iterator, Skipper>                        gidentifier;
      ph::function<NodeOperations>                                op;
    };
  } // namespace Parsers
} // namespace Gaudi

namespace bf  = boost::filesystem;
namespace gp  = Gaudi::Parsers;
namespace gpu = Gaudi::Parsers::Utils;
namespace qi  = boost::spirit::qi;

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
          from, std::format( "File {} already included from {}", absolute_path, included_from->ToString() ) );
      return true;
    }
  }
} // namespace

bool gp::Parse( std::string_view filename, std::string_view search_path, IncludedFiles* included, Messages* messages,
                Node* root ) {
  return Parse( Position(), filename, search_path, included, messages, root );
}

bool gp::Parse( const Position& from, std::string_view filename, std::string_view search_path, IncludedFiles* included,
                Messages* messages, Node* root ) {
  using Grammar = FileGrammar<Iterator, SkipperGrammar<Iterator>>;
  return ParseFile<Grammar>( from, filename, search_path, included, messages, root );
}

bool gp::ParseUnits( const Position& from, std::string_view filename, std::string_view search_path,
                     IncludedFiles* included, Messages* messages, Node* root ) {
  using Grammar = UnitsGrammar<Iterator, SkipperGrammar<Iterator>>;
  return ParseFile<Grammar>( from, filename, search_path, included, messages, root );
}
