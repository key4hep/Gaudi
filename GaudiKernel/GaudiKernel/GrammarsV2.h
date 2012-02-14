// ============================================================================
#ifndef GAUDIKERNEL_GRAMMARSV2_H
#define GAUDIKERNEL_GRAMMARSV2_H 1
#ifdef __GNUC__
#pragma GCC system_header
#endif
// ============================================================================
// Include files
// ============================================================================
// STD:
//==============================================================================
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
//==============================================================================
// Boost:
//==============================================================================
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/unused.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

#include <boost/spirit/repository/include/qi_confix.hpp>
//==============================================================================
// Gaudi:
//==============================================================================
#include "GaudiKernel/VectorMap.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/Point3DTypes.h"
#include "GaudiKernel/Point4DTypes.h"
#include "GaudiKernel/HistoDef.h"
//==============================================================================
namespace Gaudi {  namespace Parsers {
//==============================================================================
// Namespace aliases:
//==============================================================================
namespace sp    = boost::spirit;
namespace ph    = boost::phoenix;
namespace qi    = sp::qi;
namespace enc   = sp::ascii;
namespace rep  = sp::repository;
//==============================================================================
// Grammars
//==============================================================================
typedef std::string::const_iterator DefaultIterator;
typedef enc::space_type DefaultSkipper;
//==============================================================================
template <typename Iterator, typename T,  typename Skipper,
	class Enable = void>
struct Grammar_ {
  /* READ THIS IF YOUR COMPILE BREAKS ON THE FOLLOWING LINE
   *
   * To users: You have to ask developers to implement parser for your type T
   * To developer: You have to implement and register Grammar for type T
   *
   */
  BOOST_MPL_ASSERT_MSG(false, GRAMMAR_FOR_TYPE_DOES_NOT_EXISTS, (T));
};

#define REGISTER_GRAMMAR(ResultType, GrammarName) \
    template <typename Iterator, typename Skipper>\
    struct Grammar_<Iterator, ResultType, Skipper> \
    { \
        typedef GrammarName<Iterator, Skipper> Grammar;\
    }
//==============================================================================
template< typename Iterator>
struct SkipperGrammar  : qi::grammar<Iterator>
{
	SkipperGrammar() : SkipperGrammar::base_type(comments) {
		comments = enc::space | rep::confix("/*", "*/")[*(qi::char_ - "*/")]
		      |
		      rep::confix("//", (sp::eol | sp::eoi))[*(qi::char_ - (sp::eol|sp::eoi))];
	}
	qi::rule<Iterator> comments;
};
//==============================================================================
template< typename Iterator, typename Skipper>
struct StringGrammar : qi::grammar<Iterator, std::string(), qi::locals<char>,
                                                             Skipper>
{
//------------------------------------------------------------------------------
    typedef std::string ResultT;
//------------------------------------------------------------------------------
    StringGrammar() : StringGrammar::base_type( str ) {
        begin_quote   = enc::char_("\"'");
        quote     = enc::char_(qi::_r1);

        str = qi::lexeme[begin_quote[qi::_a = qi::_1]
                > *( (enc::char_('\\') >> quote(qi::_a))[qi::_val += qi::_a]
                    | (enc::char_[qi::_val += qi::_1] - quote(qi::_a))) >
               quote(qi::_a)]
              ;
    }
//------------------------------------------------------------------------------
    qi::rule<Iterator, std::string(), qi::locals<char>, Skipper> str;
    qi::rule<Iterator, char()> begin_quote;
    qi::rule<Iterator, void(char)> quote;
//------------------------------------------------------------------------------
};
REGISTER_GRAMMAR(std::string, StringGrammar);
REGISTER_GRAMMAR(Gaudi::StringKey, StringGrammar);
//==============================================================================
template< typename Iterator, typename Skipper>
struct CharGrammar : qi::grammar<Iterator, char(), Skipper>
{
    typedef char ResultT;
    CharGrammar() : CharGrammar::base_type( ch ) {
        ch = qi::int_parser<char>()
            |
            '\'' >> (qi::char_-'\'') >> '\'';
    }
    qi::rule<Iterator, char(), Skipper> ch;
};
REGISTER_GRAMMAR(char, CharGrammar);
//==============================================================================
template< typename Iterator, typename Skipper>
struct BoolGrammar : qi::grammar<Iterator, bool(), Skipper>
{
    typedef bool ResultT;
    BoolGrammar() : BoolGrammar::base_type( boolean_literal ) {
        boolean_literal =
            (qi::lit("true") | "True" | "TRUE" | "1")[qi::_val=true]
            |
            (qi::lit("false") | "False" | "FALSE" | "0")[qi::_val=false];
    }
    qi::rule<Iterator, bool(), Skipper> boolean_literal;
};
REGISTER_GRAMMAR(bool, BoolGrammar);
//==============================================================================
template< typename Iterator, typename RT , typename Skipper>
struct IntGrammar : qi::grammar<Iterator, RT(), Skipper>
{
    typedef RT ResultT;
    IntGrammar() : IntGrammar::base_type( integer ) {
        integer = qi::int_parser<RT>()[qi::_val = qi::_1] 
            >> -qi::no_case[qi::char_('L')];
    }
    qi::rule<Iterator, RT(), Skipper> integer;
};
// ----------------------------------------------------------------------------
// Register IntGrammar:
// ----------------------------------------------------------------------------
template <typename Iterator, typename T, typename Skipper>
struct Grammar_<Iterator, T,  Skipper,
    typename boost::enable_if<boost::is_integral<T> >::type>
{
    typedef IntGrammar<Iterator, T, Skipper> Grammar;
};
//==============================================================================
template< typename Iterator, typename RT, typename Skipper>
struct RealGrammar : qi::grammar<Iterator, RT(), Skipper>
{
    typedef RT ResultT;
    RealGrammar() : RealGrammar::base_type(real) {
        real = qi::real_parser<RT>();
    }
    qi::rule<Iterator, RT(), Skipper> real;
};
// ----------------------------------------------------------------------------
// Register RealGrammar:
// ----------------------------------------------------------------------------
template <typename Iterator, typename T, typename Skipper >
struct Grammar_<Iterator, T, Skipper,
               typename boost::enable_if<boost::is_floating_point<T> >::type >
{
    typedef RealGrammar<Iterator, T, Skipper> Grammar;
};
//==============================================================================
template< typename Iterator, typename VectorT, typename Skipper>
struct VectorGrammar : qi::grammar<Iterator,
             VectorT(), qi::locals<char>,Skipper>
{
//------------------------------------------------------------------------------
    typedef VectorT ResultT;
//------------------------------------------------------------------------------
    VectorGrammar() : VectorGrammar::base_type(vec) {
      begin = enc::char_('[')[qi::_val=']'] | enc::char_('{')[qi::_val='}']
              | enc::char_('(')[qi::_val=')'];
      end = enc::char_(qi::_r1);
      list = elementGrammar % ',';
      vec = begin[qi::_a = qi::_1] >> -list[qi::_val=qi::_1] >> end(qi::_a);
    }
// ----------------------------------------------------------------------------
    typename
      Grammar_<Iterator, typename VectorT::value_type, Skipper>::Grammar
      	  	  	  	  	  	  	  	  	  	  	  	  	  	  	 elementGrammar;
    qi::rule<Iterator, char()> begin;
    qi::rule<Iterator, void(char)> end;

    qi::rule<Iterator, ResultT(), qi::locals<char>,Skipper> vec;
    qi::rule<Iterator, ResultT(), Skipper> list;
// ----------------------------------------------------------------------------
};
// ----------------------------------------------------------------------------
// Register VectorGrammar for std::vector:
// ----------------------------------------------------------------------------
template <typename Iterator, typename InnerT, typename AllocatorT,
	typename Skipper>
struct Grammar_<Iterator, std::vector<InnerT, AllocatorT>, Skipper >
{
    typedef
     VectorGrammar<Iterator, std::vector<InnerT, AllocatorT>,Skipper>
    																	Grammar;
};
// ----------------------------------------------------------------------------
// Register VectorGrammar for std::list:
// ----------------------------------------------------------------------------
template <typename Iterator, typename InnerT, typename AllocatorT,
  typename Skipper>
struct Grammar_<Iterator, std::list<InnerT, AllocatorT>, Skipper >
{
    typedef
     VectorGrammar<Iterator, std::list<InnerT, AllocatorT>,Skipper>
                                      Grammar;
};
// ----------------------------------------------------------------------------
// Register VectorGrammar for std::set:
// ----------------------------------------------------------------------------
template <typename Iterator, typename InnerT, typename CompareT,
  typename AllocatorT, typename Skipper>
struct Grammar_<Iterator, std::set<InnerT, CompareT, AllocatorT>, Skipper >
{
    typedef
     VectorGrammar<Iterator, std::set<InnerT, CompareT, AllocatorT>,Skipper>
                                      Grammar;
};

//==============================================================================
template< typename Iterator, typename PairT, typename Skipper>
struct PairGrammar :
  qi::grammar<Iterator,PairT(), qi::locals<char>, Skipper> {
//------------------------------------------------------------------------------
    typedef PairT ResultT;
    typedef typename PairT::first_type first_type;
    typedef typename PairT::second_type second_type;
//------------------------------------------------------------------------------
    PairGrammar() : PairGrammar::base_type(pair) {
        init(",");
    }

    PairGrammar(const std::string& delimeter) : PairGrammar::base_type(pair) {
        init(delimeter);
    }
//------------------------------------------------------------------------------
    struct first {};
    struct second {};
    void init(const std::string& delimeter) {
      begin = enc::char_('(')[qi::_val=')']
              |
              enc::char_('[')[qi::_val=']'];
      end =  qi::char_(qi::_r1);
      pair = begin[qi::_a = qi::_1] >> pair_in[qi::_val = qi::_1] >> end(qi::_a);
      pair_in =  key >> qi::lit(delimeter) >> value;
    }
// ----------------------------------------------------------------------------
    typename
       Grammar_<Iterator, typename PairT::first_type, Skipper>::Grammar key;
    typename
       Grammar_<Iterator, typename PairT::second_type, Skipper>::Grammar
         value;
    qi::rule<Iterator, char()> begin;
    qi::rule<Iterator, void(char)> end;
    qi::rule<Iterator, ResultT(), qi::locals<char>, Skipper> pair;
    qi::rule<Iterator, ResultT(), Skipper> pair_in;
    //ph::function<Operations> op;
// ----------------------------------------------------------------------------
}; // END PairGrammar
// ----------------------------------------------------------------------------
// Register PairGrammar:
// ----------------------------------------------------------------------------
template <typename Iterator, typename KeyT, typename ValueT,
	typename Skipper>
struct Grammar_<Iterator, std::pair<KeyT, ValueT>, Skipper >
{
    typedef PairGrammar<Iterator, std::pair<KeyT, ValueT>, Skipper> Grammar;
};
// ============================================================================
template< typename Iterator, typename MapT, typename Skipper>
struct MapGrammar : qi::grammar<Iterator,MapT(), Skipper>
{
//------------------------------------------------------------------------------
    typedef MapT ResultT;
    typedef typename MapT::key_type KeyT;
    typedef typename MapT::mapped_type MappedT;
    typedef std::pair<KeyT, MappedT> PairT;

    typedef std::vector<PairT> VectorPairT;
//------------------------------------------------------------------------------
    struct tag_key{};
    struct tag_mapped{};
    struct Operations
    {
        template <typename A, typename B = boost::fusion::unused_type,
            typename C = boost::fusion::unused_type,
            typename D = boost::fusion::unused_type>
        struct result { typedef void type; };
        //----------------------------------------------------------------------
        void operator()(ResultT& res, const VectorPairT& vec) const{
            for(typename VectorPairT::const_iterator cur = vec.begin();
                cur != vec.end(); cur++){
                res.insert(*cur);
            }
        }
        void operator()(PairT& res, const KeyT& key, tag_key) const{
          res.first = key;
        }
        void operator()(PairT& res, const MappedT& value, tag_mapped) const{
          res.second = value;
        }
        //----------------------------------------------------------------------
    };
//------------------------------------------------------------------------------
    MapGrammar() : MapGrammar::base_type(map) {
        pair = key[op(qi::_val,qi::_1, tag_key())] > (qi::lit(':') | '=')  >
        value[op(qi::_val,qi::_1, tag_mapped())];
        list = -(pair % enc::char_(','));
        map = (('['  >> list >> ']')
              | ('{'  >> list >> '}'))[op(qi::_val,qi::_1)];
    }
// ----------------------------------------------------------------------------
    typename
      Grammar_<Iterator, typename MapT::key_type, Skipper>::Grammar key;
    typename
      Grammar_<Iterator, typename MapT::mapped_type, Skipper>::Grammar value;
    qi::rule<Iterator, PairT(), Skipper> pair;
    qi::rule<Iterator, VectorPairT(), Skipper> list;
    qi::rule<Iterator, ResultT(), Skipper> map;
    ph::function<Operations> op;
// ----------------------------------------------------------------------------
};
// ----------------------------------------------------------------------------
// Register MapGrammar for std::map:
// ----------------------------------------------------------------------------
template <typename Iterator, typename KeyT, typename ValueT,
    typename KeyCompareT, typename AllocatorT, typename Skipper>
struct Grammar_<Iterator, std::map<KeyT, ValueT, KeyCompareT, AllocatorT>,
  Skipper > {
    typedef MapGrammar<Iterator,
        std::map<KeyT, ValueT, KeyCompareT, AllocatorT>, Skipper> Grammar;
};
// ----------------------------------------------------------------------------
// Register MapGrammar for GaudiUtils::VectorMap:
// ----------------------------------------------------------------------------
template <typename Iterator, typename KeyT, typename ValueT,
    typename KeyCompareT, typename AllocatorT, typename Skipper>
struct Grammar_<Iterator, GaudiUtils::VectorMap<KeyT, ValueT,
    KeyCompareT, AllocatorT>, Skipper>
{
    typedef MapGrammar<Iterator,
        GaudiUtils::VectorMap<KeyT, ValueT, KeyCompareT, AllocatorT>,
        Skipper> Grammar;
};
// ============================================================================
template< typename Iterator, typename PointT, typename Skipper>
struct Pnt3DGrammar : qi::grammar<Iterator, PointT(), Skipper> {
    typedef PointT ResultT;
    typedef typename PointT::Scalar Scalar;
// ----------------------------------------------------------------------------
    struct Operations {
        template <typename A, typename B = boost::fusion::unused_type,
            typename C = boost::fusion::unused_type,
            typename D = boost::fusion::unused_type>
        struct result { typedef void type; };
        void operator()(ResultT& res, const Scalar& scalar,
                            const char xyz) const{
            switch(xyz){
                case 'x': res.SetX(scalar); break;
                case 'y': res.SetY(scalar); break;
                case 'z': res.SetZ(scalar); break;
                default: break;
            }
        }
    }; //  Operations
// ----------------------------------------------------------------------------
    Pnt3DGrammar() : Pnt3DGrammar::base_type(point) {
        point = list | ('(' >> list >> ')') | ('[' >> list >> ']');
        list = -(enc::no_case[qi::lit("x") | qi::lit("px")]  >> ':')
             >> scalar[op(qi::_val,qi::_1,'x')] >>
         ',' >> -(enc::no_case[qi::lit("y") | qi::lit("py")] >> ':')
             >> scalar[op(qi::_val,qi::_1,'y')] >>
         ',' >> -(enc::no_case[qi::lit("z") | qi::lit("pz")] >> ':')
             >> scalar[op(qi::_val,qi::_1,'z')];
    }
// ----------------------------------------------------------------------------
    qi::rule<Iterator, ResultT(), Skipper> point, list;
    typename Grammar_<Iterator, Scalar, Skipper>::Grammar scalar;
    ph::function<Operations> op;
// ----------------------------------------------------------------------------
}; //   Pnt3DGrammar
// ----------------------------------------------------------------------------
// Register Pnt3DGrammar for ROOT::Math::PositionVector3D:
// ----------------------------------------------------------------------------
template <typename Iterator, typename T1, typename T2, typename Skipper>
struct Grammar_<Iterator, ROOT::Math::PositionVector3D<T1,T2>, Skipper>{
    typedef
        Pnt3DGrammar<Iterator, ROOT::Math::PositionVector3D<T1,T2>, Skipper>
        Grammar;
};
// ----------------------------------------------------------------------------
// Register Pnt3DGrammar for ROOT::Math::DisplacementVector3D:
// ----------------------------------------------------------------------------
template <typename Iterator, typename T1, typename T2, typename Skipper>
struct Grammar_<Iterator, ROOT::Math::DisplacementVector3D<T1,T2>, Skipper>{
  typedef
      Pnt3DGrammar<Iterator,
              ROOT::Math::DisplacementVector3D<T1,T2>, Skipper> Grammar;
};
// ============================================================================
template< typename Iterator, typename PointT, typename Skipper>
struct Pnt4DGrammar : qi::grammar<Iterator, PointT(), Skipper>
{
    typedef PointT ResultT;
    typedef typename PointT::Scalar ScalarT;
//-----------------------------------------------------------------------------
    struct Operations {
        template <typename A, typename B = boost::fusion::unused_type,
            typename C = boost::fusion::unused_type,
            typename D = boost::fusion::unused_type>
        struct result { typedef void type; };

        void operator()(ResultT& res, const ScalarT& scalar,
                                                        const char xyz) const{
            switch(xyz){
                case 'x': res.SetPx(scalar); break;
                case 'y': res.SetPy(scalar); break;
                case 'z': res.SetPz(scalar); break;
                case 'e': res.SetE(scalar);  break;
                default: break;
            }
        }
        void operator()(ResultT& res, const ResultT& xyz) const{
                res.SetPx(xyz.Px());
                res.SetPy(xyz.Py());
                res.SetPz(xyz.Pz());
        }
    }; //   Operations
// ----------------------------------------------------------------------------
    Pnt4DGrammar() : Pnt4DGrammar::base_type(point4d) {
        point4d = list4d | ('(' >> list4d >> ')') | ('[' >> list4d >> ']');
        list4d = (point3d[op(qi::_val,qi::_1)] >> enc::char_(";,")
            >> e[op(qi::_val, qi::_1, 'e')])
            |
            (e[op(qi::_val,qi::_1, 'e')] >> enc::char_(";,")
                >> point3d[op(qi::_val, qi::_1)]);
        e =  -(enc::no_case[enc::char_("te")]  >> ':')
            >> scalar[qi::_val = qi::_1];

        point3d = list3d | ('(' >> list3d >> ')') | ('[' >> list3d >> ']');
        list3d = -(enc::no_case[qi::lit("x") | qi::lit("px")]  >> ':')
             >> scalar[op(qi::_val, qi::_1,'x')] >>
         ',' >> -(enc::no_case[qi::lit("y") | qi::lit("py")] >> ':')
             >> scalar[op(qi::_val, qi::_1,'y')] >>
         ',' >> -(enc::no_case[qi::lit("z") | qi::lit("pz")] >> ':')
             >> scalar[op(qi::_val, qi::_1,'z')];
    }
// ----------------------------------------------------------------------------
    qi::rule<Iterator, ResultT(), Skipper> point3d, point4d, list3d,
        list4d;
    qi::rule<Iterator, ScalarT(), Skipper> e;
    typename Grammar_<Iterator, ScalarT, Skipper>::Grammar scalar;
    ph::function<Operations> op;
// ----------------------------------------------------------------------------
}; //   Pnt4DGrammar
// ----------------------------------------------------------------------------
// Register Pnt4DGrammar for ROOT::Math::LorentzVector:
// ----------------------------------------------------------------------------
template <typename Iterator, typename T1, typename Skipper>
struct Grammar_<Iterator, ROOT::Math::LorentzVector<T1>, Skipper >
{
    typedef Pnt4DGrammar<Iterator, ROOT::Math::LorentzVector<T1>, Skipper>
      Grammar;
};
// ============================================================================
template< typename Iterator, typename Skipper>
struct Histo1DGrammar : qi::grammar<Iterator, Gaudi::Histo1DDef(),
  qi::locals<char>, Skipper>
{
    typedef Gaudi::Histo1DDef ResultT;
// ----------------------------------------------------------------------------
    struct Operations {
        template <typename A, typename B = boost::fusion::unused_type,
            typename C = boost::fusion::unused_type,
            typename D = boost::fusion::unused_type>
        struct result { typedef void type; };
        void operator()(ResultT& res, const std::string& title) const{
            res.setTitle(title);
        }
        void operator()(ResultT& res, const double& val, const char lh) const{
            switch (lh) {
                case 'l': res.setLowEdge(val); break;
                case 'h': res.setHighEdge(val); break;
                default: break;
            }
        }
        void operator()(ResultT& res, int val) const{
            res.setBins(val);
        }
        void operator()(ResultT& res) const{}
    }; //   Operations
// ----------------------------------------------------------------------------
    Histo1DGrammar() : Histo1DGrammar::base_type(hist) {
        val1 = title[op(qi::_val, qi::_1)] >> ','
            >> qi::double_[op(qi::_val, qi::_1, 'l')] >> ','
            >> qi::double_[op(qi::_val, qi::_1, 'h')]
            >> -(',' >> qi::int_[op(qi::_val, qi::_1)]);
        val2 = qi::double_[op(qi::_val, qi::_1, 'l')] >> ','
            >> qi::double_[op(qi::_val, qi::_1, 'h')] >> ','
            >> title[op(qi::_val, qi::_1)] >> -(','
            >> qi::int_[op(qi::_val, qi::_1)]);
        val3 = qi::double_[op(qi::_val, qi::_1, 'l')] >> ','
            >> qi::double_[op(qi::_val, qi::_1, 'h')]
           >> -(','  >> title[op(qi::_val, qi::_1)])
           >> -(','  >> qi::int_[op(qi::_val, qi::_1)]);
        begin = enc::char_('[')[qi::_val=']'] | enc::char_('(')[qi::_val=')'];
        end = enc::char_(qi::_r1);
        hist = begin[qi::_a = qi::_1] >> (val1 | val2 | val3)[qi::_val=qi::_1]
                                                       >> end(qi::_a);
    }
// ----------------------------------------------------------------------------
    qi::rule<Iterator, ResultT(), qi::locals<char>, Skipper> hist;
    qi::rule<Iterator, ResultT(), Skipper> val1, val2, val3;
    qi::rule<Iterator, char()> begin;
    qi::rule<Iterator, void(char)> end;
    StringGrammar<Iterator, Skipper> title;
    ph::function<Operations> op;
// ----------------------------------------------------------------------------
}; //   Histo1DGrammar
// ----------------------------------------------------------------------------
REGISTER_GRAMMAR(Gaudi::Histo1DDef, Histo1DGrammar);
// ============================================================================
template< typename Iterator, typename Skipper>
struct KeyValueGrammar :
  qi::grammar<Iterator, std::pair<std::string, std::string>(), Skipper> {
//------------------------------------------------------------------------------
    typedef std::pair<std::string, std::string> ResultT;
//------------------------------------------------------------------------------
    struct first {};
    struct second {};

    KeyValueGrammar() : KeyValueGrammar::base_type(pair) {
//------------------------------------------------------------------------------
      pair =  gstring >> ":" >> +enc::char_;
    }
// ----------------------------------------------------------------------------
    StringGrammar<Iterator, Skipper> gstring;
    qi::rule<Iterator, ResultT(), Skipper> pair;
// ----------------------------------------------------------------------------
}; // END KeyValueGrammar
// We don't register KeyalueGrammar because it's a special parser
// ============================================================================
}} //   Gaudi::Parsers
//============================================================================
#endif
