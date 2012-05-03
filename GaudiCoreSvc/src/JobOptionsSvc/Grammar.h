#ifndef JOBOPTIONSVC_GRAMMAR_H_
#define JOBOPTIONSVC_GRAMMAR_H_
// ============================================================================
// Includes:
// ============================================================================
// STD & STL:
// ============================================================================
#include <string>
#include <vector>
#include <map>
// ============================================================================
// Boost:
// ============================================================================
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>

#if BOOST_VERSION <= 104400
#include "iter_pos.hpp"
#else
#include <boost/spirit/repository/include/qi_iter_pos.hpp>
#endif

// ============================================================================
//  Project:
// ============================================================================
#include "Node.h"
//============================================================================
namespace Gaudi { namespace Parsers {
// ============================================================================
// Namespace aliases:
// ============================================================================
namespace sp = boost::spirit;
namespace ph = boost::phoenix;
namespace qi = sp::qi;
namespace enc = sp::ascii;
namespace rep = sp::repository;
//=============================================================================
// Grammars
//=============================================================================
template<typename Iterator>
struct SkipperGrammar: qi::grammar<Iterator> {
  SkipperGrammar() : SkipperGrammar::base_type(comments) {
      comments = enc::space
              | rep::confix("/*", "*/")[*(qi::char_ - "*/")]
              | rep::confix("//", (sp::eol | sp::eoi))[*(qi::char_ - (sp::eol | sp::eoi))];
  }
  qi::rule<Iterator> comments;
};
// ============================================================================
template<typename Iterator, typename Skipper>
struct StringGrammar: qi::grammar<Iterator, std::string(), qi::locals<char>,
  Skipper> {
  //---------------------------------------------------------------------------
  typedef std::string ResultT;
        //---------------------------------------------------------------------
  StringGrammar() : StringGrammar::base_type(str) {
      begin_quote = enc::char_("\"'");
      quote = enc::char_(qi::_r1);

      str = qi::lexeme[begin_quote[qi::_a = qi::_1] >> *((enc::char_('\\')
          >> quote(qi::_a))[qi::_val += qi::_a]
          | (enc::char_[qi::_val += qi::_1] - quote(qi::_a)))
          >> quote(qi::_a)];
  }
//-----------------------------------------------------------------------------
  qi::rule<Iterator, std::string(), qi::locals<char>, Skipper> str;
  qi::rule<Iterator, char()> begin_quote;
  qi::rule<Iterator, void(char)> quote;
//-----------------------------------------------------------------------------
};
// ============================================================================
template<typename Iterator, typename Skipper>
struct IdentifierGrammar: qi::grammar<Iterator, Node(), Skipper> {
//-----------------------------------------------------------------------------
  typedef std::string ResultT;
//-----------------------------------------------------------------------------
  IdentifierGrammar() : IdentifierGrammar::base_type(ident) {
      ident = rep::qi::iter_pos[op(qi::_val, qi::_1)] >> str[op(qi::_val, qi::_1)][op(qi::_val, Node::kIdentifier)];
      str = -qi::lit("::")[qi::_val += "::"] >> inner[qi::_val += qi::_1]
        >> *(qi::lit("::") >> inner[qi::_val += ("::"+qi::_1)]);
      inner = qi::alpha >> *(qi::alnum | qi::char_('_'));
  }
// ----------------------------------------------------------------------------
  qi::rule<Iterator, Node(), Skipper> ident;
  qi::rule<Iterator, std::string(), Skipper> str;
  qi::rule<Iterator, std::string()> inner;
  ph::function<NodeOperations> op;
};
// ============================================================================
template<typename Iterator, typename Skipper>
struct BoolGrammar: qi::grammar<Iterator, bool(), Skipper> {
// ----------------------------------------------------------------------------
  typedef bool ResultT;
// ----------------------------------------------------------------------------
  BoolGrammar() : BoolGrammar::base_type(boolean) {
      boolean = enc::no_case[
                             qi::lit("true")[qi::_val=true]
                             |
                             qi::lit("false")[qi::_val=false]
               ];
 }
// ----------------------------------------------------------------------------
  qi::rule<Iterator, bool(), Skipper> boolean;
};
// ============================================================================
template<typename Iterator, typename Skipper>
struct RealGrammar: qi::grammar<Iterator, Node(), Skipper> {
// ----------------------------------------------------------------------------
  typedef bool ResultT;
//---------------------------------------------------------------------
  RealGrammar() : RealGrammar::base_type(real) {
      real = qi::raw[qi::double_][op(qi::_val, qi::_1)]
                   [op(qi::_val, Node::kReal)]
                     >> -(-qi::char_('*') >> gunit[op(qi::_val,qi::_1)]);

  }
// ----------------------------------------------------------------------------
  qi::rule<Iterator, Node(), Skipper> real;
  IdentifierGrammar<Iterator,Skipper> gunit;
  ph::function<NodeOperations> op;
};
// ============================================================================
template<typename Iterator, typename Skipper>
struct UnitsGrammar: qi::grammar<Iterator, Node(), Skipper> {
// ----------------------------------------------------------------------------
  UnitsGrammar() : UnitsGrammar::base_type(units) {
      units = *unit[op(qi::_val, qi::_1)];
      unit = rep::qi::iter_pos[op(qi::_val, qi::_1)] >>
             val[op(qi::_val, qi::_1)]
                     >> -qi::lit('*') >> gunit[op(qi::_val, qi::_1)]
                     >> qi::lit('=') >> val[op(qi::_val, qi::_1)]
                                            [op(qi::_val, Node::kUnit)];;
      val = qi::raw[qi::double_]
                            [op(qi::_val, qi::_1)][op(qi::_val, Node::kReal)];

  }
// ----------------------------------------------------------------------------
  qi::rule<Iterator, Node(), Skipper> units, unit, val;
  IdentifierGrammar<Iterator,Skipper> gunit;
  ph::function<NodeOperations> op;
};
// ============================================================================
template<typename Iterator, typename Skipper>
struct FileGrammar: qi::grammar<Iterator, Node(), Skipper> {
  FileGrammar() : FileGrammar::base_type(file) {
      file = -shell[op(qi::_val,qi::_1)] >>
              *(statement[op(qi::_val, qi::_1)])
              [op(qi::_val, Node::kRoot)];
      shell = rep::confix("#!", qi::eol)[*(qi::char_[qi::_a += qi::_1]
                      - qi::eol)][op(qi::_val,Node::kShell)]
                      [op(qi::_val,qi::_a)];
      statement = rep::qi::iter_pos[qi::_a = qi::_1]
                    >> (include | assign | units | print_options | pragma
                        | condition)[qi::_val = qi::_1][op(qi::_val,qi::_a)];
      condition = (qi::lit("#ifdef")[op(qi::_a,Node::kIfdef)]
                        | qi::lit("#ifndef")[op(qi::_a,Node::kIfndef)])
                        >> property[op(qi::_val,qi::_1)]
                        >> (*statement[op(qi::_a,qi::_1)])[op(qi::_val, qi::_a)]
                        >> -(qi::lit("#else")[op(qi::_b,Node::kElse)]
                          >> *statement[op(qi::_b,qi::_1)])[op(qi::_val,qi::_b)]
                        >> qi::lit("#endif")[op(qi::_val, Node::kCondition)];
      include = qi::lit("#include")
                      >> gstring[op(qi::_val, qi::_1)]
                                [op(qi::_val, Node::kInclude)];
      units = qi::lit("#units")
                      >> gstring[op(qi::_val, qi::_1)]
                                [op(qi::_val, Node::kUnits)];
      print_options = qi::lit("#printOptions") >> qi::lit("full")
                    [op(qi::_val, Node::kPrintOptions)];
      pragma = qi::lit("#pragma") >> (pragma_print | pragma_tree |
          pragma_dump_file);
      pragma_print = qi::lit("print") >> enc::no_case[
        qi::lit("on")[op(qi::_val, Node::kPrintOn)]
        | qi::lit("off")[op(qi::_val, Node::kPrintOff)]
      ];
      pragma_tree = enc::no_case[
                     qi::lit("printtree")[op(qi::_val, Node::kPrintTree)]
                    ];
      pragma_dump_file  = qi::lit("dumpfile") >> gstring[op(qi::_val, qi::_1)]
                                                [op(qi::_val, Node::kDumpFile)];
      assign = property[op(qi::_val, qi::_1)]
                             [op(qi::_val, Node::kAssign)]
                      >> oper[op(qi::_val, qi::_1)]
                      >> value[op(qi::_val, qi::_1)]
                      >> ';';
      property = rep::qi::iter_pos[op(qi::_val, qi::_1)]
                         >> (gidentifier[op(qi::_val, qi::_1)] % '.')
                        [op(qi::_val, Node::kProperty)];
      property_ref %= -qi::lit('@') >>
          property[op(qi::_val, Node::kPropertyRef)];
      oper = rep::qi::iter_pos[op(qi::_val, qi::_1)]
                   >> (qi::lit("=")[op(qi::_val, Node::kEqual)]
                        |
                        qi::lit("+=")[op(qi::_val, Node::kPlusEqual)]
                        |
                        qi::lit("-=")[op(qi::_val, Node::kMinusEqual)]);
      value = rep::qi::iter_pos[qi::_a =  qi::_1]
                     >>
                     (map_value | vector_value |  simple_value | property |
                         property_ref)
                     [qi::_val = qi::_1][op(qi::_val, qi::_a)]
                     ;
      begin_vector = enc::char_('(')[qi::_val=')']
                           |
                           enc::char_('[')[qi::_val=']']
                           |
                           enc::char_('{')[qi::_val='}'];
      end_vector = qi::char_(qi::_r1);
      vector_value = (begin_vector[qi::_a = qi::_1]
                             >> -(value[op(qi::_val,qi::_1)] % ',')
                               >> end_vector(qi::_a)
                           )[op(qi::_val,Node::kVector)];
      map_value = (enc::char_('{')
                    >> -(pair[op(qi::_val, qi::_1)] % ',')
                        >> enc::char_('}'))
                    [op(qi::_val, Node::kMap)];
      pair = simple_value[op(qi::_val,qi::_1)] >> ':'
                    >> value[op(qi::_val,qi::_1)]
                     [op(qi::_val,Node::kPair)];
      simple_value =
                    (gstring[op(qi::_val, qi::_1)][op(qi::_val, Node::kString)])
                    |
                    (gbool[op(qi::_val, qi::_1)][op(qi::_val, Node::kBool)])
                    |
                    (greal[qi::_val = qi::_1]);
  }
  qi::rule<Iterator, Node(), Skipper> file, include, assign, property,
          property_ref, oper, map_value, pair_value, simple_value, pair, units,
          print_options, pragma, pragma_print, pragma_tree, pragma_dump_file;
  qi::rule<Iterator, Node(),qi::locals<std::string> > shell;
  qi::rule<Iterator, Node(), qi::locals<Iterator>, Skipper> statement, value;
  qi::rule<Iterator, Node(), qi::locals<char>, Skipper> vector_value;
  qi::rule<Iterator, Node(), qi::locals<Node, Node>, Skipper> condition;
  qi::rule<Iterator, char()> begin_vector;
  qi::rule<Iterator, void(char)> end_vector;
  StringGrammar<Iterator, Skipper> gstring;
  BoolGrammar<Iterator, Skipper> gbool;
  RealGrammar<Iterator, Skipper> greal;
  IdentifierGrammar<Iterator, Skipper> gidentifier;
  ph::function<NodeOperations> op;
};
// ============================================================================
}  /* Gaudi */ } /* Parsers */
// ============================================================================
#endif  // JOBOPTIONSVC_GRAMMAR_H_
