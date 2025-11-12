/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

// cppcheck-suppress-file useInitializationList; for consistency

#pragma once

#ifdef __GNUC__
#  pragma GCC system_header
#endif

#include <GaudiKernel/HistoDef.h>
#include <GaudiKernel/Point3DTypes.h>
#include <GaudiKernel/Point4DTypes.h>
#include <GaudiKernel/StringKey.h>
#include <GaudiKernel/VectorMap.h>
#include <list>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/fusion/include/std_pair.hpp>
#if ( BOOST_VERSION >= 187000 ) && ( BOOST_VERSION < 188000 )
#  define BOOST_ALLOW_DEPRECATED_HEADERS
#endif
#include <boost/spirit/include/qi.hpp>
#undef BOOST_ALLOW_DEPRECATED_HEADERS

#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>

namespace Gaudi::Parsers {
  namespace sp  = boost::spirit;
  namespace ph  = boost::phoenix;
  namespace qi  = sp::qi;
  namespace enc = sp::ascii;
  namespace rep = sp::repository;

  template <typename Iterator, typename T, typename Skipper, class Enable = void>
  struct Grammar_ {
    /* READ THIS IF YOUR COMPILE BREAKS ON THE FOLLOWING LINE
     *
     * To users: You have to ask developers to implement parser for your type T
     * To developer: You have to implement and register Grammar for type T
     *
     */
    BOOST_MPL_ASSERT_MSG( false, GRAMMAR_FOR_TYPE_DOES_NOT_EXISTS, ( T ) );
  };

#define REGISTER_GRAMMAR( ResultType, GrammarName )                                                                    \
  template <typename Iterator, typename Skipper>                                                                       \
  struct Grammar_<Iterator, ResultType, Skipper> {                                                                     \
    using Grammar = GrammarName<Iterator, Skipper>;                                                                    \
  }

  template <typename Iterator>
  struct SkipperGrammar : qi::grammar<Iterator> {
    qi::rule<Iterator> comments;
    SkipperGrammar() : SkipperGrammar::base_type( comments ) {
      comments = enc::space | rep::confix( "/*", "*/" )[*( qi::char_ - "*/" )] |
                 rep::confix( "//", ( sp::eol | sp::eoi ) )[*( qi::char_ - ( sp::eol | sp::eoi ) )];
    }
  };

  template <typename It, typename Skipper>
  struct StringGrammar : qi::grammar<It, std::string(), Skipper> {
    qi::rule<It, std::string()>          dq_body, sq_body;
    qi::rule<It, std::string(), Skipper> str;
    StringGrammar() : StringGrammar::base_type( str ) {
      dq_body = qi::eps[qi::_val = std::string{}] >> *( ( qi::lit( '\\' ) >> qi::char_( '"' ) )[qi::_val += '"'] |
                                                        ( qi::lit( '\\' ) >> qi::char_( '\\' ) )[qi::_val += '\\'] |
                                                        ( qi::char_ - qi::char_( '"' ) )[qi::_val += qi::_1] );
      sq_body = qi::eps[qi::_val = std::string{}] >> *( ( qi::lit( '\\' ) >> qi::char_( '\'' ) )[qi::_val += '\''] |
                                                        ( qi::lit( '\\' ) >> qi::char_( '\\' ) )[qi::_val += '\\'] |
                                                        ( qi::char_ - qi::char_( '\'' ) )[qi::_val += qi::_1] );
      str     = qi::lexeme[rep::confix( '"', '"' )[dq_body] | rep::confix( '\'', '\'' )[sq_body]];
    }
  };
  REGISTER_GRAMMAR( std::string, StringGrammar );
  REGISTER_GRAMMAR( Gaudi::StringKey, StringGrammar );

  template <typename Iterator, typename Skipper>
  struct CharGrammar : qi::grammar<Iterator, char(), Skipper> {
    using ResultT = char;
    qi::rule<Iterator, char(), Skipper> ch;
    CharGrammar() : CharGrammar::base_type( ch ) { ch = qi::int_parser<char>() | '\'' >> ( qi::char_ - '\'' ) >> '\''; }
  };
  REGISTER_GRAMMAR( char, CharGrammar );

  template <typename Iterator, typename Skipper>
  struct BoolGrammar : qi::grammar<Iterator, bool(), Skipper> {
    qi::symbols<char, bool>             kw;
    qi::rule<Iterator, bool(), Skipper> boolean_literal;

    BoolGrammar() : BoolGrammar::base_type( boolean_literal ) {
      kw.add( "true", true )( "True", true )( "TRUE", true )( "1", true )( "false", false )( "False", false )(
          "FALSE", false )( "0", false );
      boolean_literal = kw;
    }
  };
  REGISTER_GRAMMAR( bool, BoolGrammar );

  template <typename Iterator, typename RT, typename Skipper>
  struct IntGrammar : qi::grammar<Iterator, RT(), Skipper> {
    using ResultT = RT;
    IntGrammar() : IntGrammar::base_type( integer ) {
      integer = qi::int_parser<RT>()[qi::_val = qi::_1] >> -qi::no_case[qi::char_( 'L' )];
    }
    qi::rule<Iterator, RT(), Skipper> integer;
  };

  template <typename Iterator, std::integral T, typename Skipper>
  struct Grammar_<Iterator, T, Skipper> {
    using Grammar = IntGrammar<Iterator, T, Skipper>;
  };

  template <typename Iterator, typename RT, typename Skipper>
  struct RealGrammar : qi::grammar<Iterator, RT(), Skipper> {
    using ResultT = RT;
    RealGrammar() : RealGrammar::base_type( real ) { real = qi::real_parser<RT>(); }
    qi::rule<Iterator, RT(), Skipper> real;
  };

  template <typename Iterator, std::floating_point T, typename Skipper>
  struct Grammar_<Iterator, T, Skipper> {
    using Grammar = RealGrammar<Iterator, T, Skipper>;
  };

  template <typename It, typename Skipper, typename... Ts>
  struct TupleGrammar : qi::grammar<It, std::tuple<Ts...>(), Skipper> {
    static_assert( sizeof...( Ts ) > 0, "Tuple must have at least one element" );
    std::tuple<typename Grammar_<It, Ts, Skipper>::Grammar...> elems{};
    qi::rule<It, std::tuple<Ts...>(), Skipper>                 body;
    qi::rule<It, std::tuple<Ts...>(), Skipper>                 tuple;
    TupleGrammar() : TupleGrammar::base_type( tuple ) { init( std::make_index_sequence<sizeof...( Ts ) - 1>{} ); }

  private:
    template <std::size_t... Is>
    void init( std::index_sequence<Is...> ) {
      if constexpr ( sizeof...( Is ) == 0 ) {
        body = get<0>( elems ) >> -qi::lit( ',' );
      } else {
        body = get<0>( elems ) >> ( ( qi::lit( ',' ) >> get<Is + 1>( elems ) ) >> ... ) >> -qi::lit( ',' );
      }
      tuple = rep::confix( '(', ')' )[body] | rep::confix( '[', ']' )[body];
    }
  };

  // Expose via the expected alias for backwards compatibility
  template <typename Iterator, typename Skipper, typename... Ts>
  struct Grammar_<Iterator, std::tuple<Ts...>, Skipper> {
    using Grammar = TupleGrammar<Iterator, Skipper, Ts...>;
  };

  template <typename Iterator, typename VectorT, typename Skipper>
  struct VectorGrammar : qi::grammar<Iterator, VectorT(), Skipper> {
    using ResultT    = VectorT;
    using value_type = typename VectorT::value_type;
    typename Grammar_<Iterator, value_type, Skipper>::Grammar element;
    qi::rule<Iterator, ResultT(), Skipper>                    list;
    qi::rule<Iterator, Skipper>                               trailing_commas;
    qi::rule<Iterator, ResultT(), Skipper>                    body;
    qi::rule<Iterator, ResultT(), Skipper>                    vec;
    VectorGrammar() : VectorGrammar::base_type( vec ) {
      list            = element % ',';
      trailing_commas = qi::omit[*enc::char_( ',' )];
      body            = -list >> trailing_commas;
      vec             = rep::confix( '(', ')' )[body] | rep::confix( '[', ']' )[body] | rep::confix( '{', '}' )[body];
    }
  };

  template <typename Iterator, typename InnerT, typename AllocatorT, typename Skipper>
  struct Grammar_<Iterator, std::vector<InnerT, AllocatorT>, Skipper> {
    using Grammar = VectorGrammar<Iterator, std::vector<InnerT, AllocatorT>, Skipper>;
  };

  template <typename Iterator, typename InnerT, typename AllocatorT, typename Skipper>
  struct Grammar_<Iterator, std::list<InnerT, AllocatorT>, Skipper> {
    using Grammar = VectorGrammar<Iterator, std::list<InnerT, AllocatorT>, Skipper>;
  };

  template <typename Iterator, typename InnerT, typename CompareT, typename AllocatorT, typename Skipper>
  struct Grammar_<Iterator, std::set<InnerT, CompareT, AllocatorT>, Skipper> {
    using Grammar = VectorGrammar<Iterator, std::set<InnerT, CompareT, AllocatorT>, Skipper>;
  };

  template <typename Iterator, typename SetT, typename Skipper>
  struct SetGrammar : qi::grammar<Iterator, SetT(), qi::locals<char>, Skipper> {
    SetGrammar() : SetGrammar::base_type( set ) {
      // special handling of empty set as "{}" is always a dict
      set = qi::lit( "set()" ) | grVector;
    }
    VectorGrammar<Iterator, SetT, Skipper>                grVector;
    qi::rule<Iterator, SetT(), qi::locals<char>, Skipper> set;
  };

  template <typename Iterator, typename InnerT, typename HashT, typename CompareT, typename AllocatorT,
            typename Skipper>
  struct Grammar_<Iterator, std::unordered_set<InnerT, HashT, CompareT, AllocatorT>, Skipper> {
    using Grammar = SetGrammar<Iterator, std::unordered_set<InnerT, HashT, CompareT, AllocatorT>, Skipper>;
  };

  template <typename Iterator, typename PairT, typename Skipper, char Delim = ','>
  struct PairGrammar : qi::grammar<Iterator, PairT(), Skipper> {
    typename Grammar_<Iterator, typename PairT::first_type, Skipper>::Grammar  key;
    typename Grammar_<Iterator, typename PairT::second_type, Skipper>::Grammar val;
    qi::rule<Iterator, PairT(), Skipper>                                       kv;
    qi::rule<Iterator, PairT(), Skipper>                                       pair;
    PairGrammar() : PairGrammar::base_type( pair ) {
      kv   = key >> qi::lit( Delim ) >> val;
      pair = rep::confix( '(', ')' )[kv] | rep::confix( '[', ']' )[kv];
    }
  };

  template <typename Iterator, typename KeyT, typename ValueT, typename Skipper>
  struct Grammar_<Iterator, std::pair<KeyT, ValueT>, Skipper> {
    using Grammar = PairGrammar<Iterator, std::pair<KeyT, ValueT>, Skipper>;
  };

  template <typename It, typename MapT, typename Skipper>
  struct MapGrammar : qi::grammar<It, MapT(), Skipper> {
    using KeyT    = typename MapT::key_type;
    using MappedT = typename MapT::mapped_type;
    using PairT   = std::pair<KeyT, MappedT>;

    typename Grammar_<It, KeyT, Skipper>::Grammar    key;
    typename Grammar_<It, MappedT, Skipper>::Grammar val;

    qi::rule<It, PairT(), Skipper>              kv;
    qi::rule<It, std::vector<PairT>(), Skipper> body;
    qi::rule<It, std::vector<PairT>(), Skipper> bracketed;
    qi::rule<It, MapT(), Skipper>               map;

    MapGrammar() : MapGrammar::base_type( map ) {
      kv        = key >> ( qi::lit( ':' ) | qi::lit( '=' ) ) >> val;
      body      = -( kv % ',' ) >> qi::omit[*enc::char_( ',' )];
      bracketed = rep::confix( '[', ']' )[body] | rep::confix( '{', '}' )[body];
      map       = bracketed;
    }
  };

  template <typename Iterator, typename KeyT, typename ValueT, typename KeyCompareT, typename AllocatorT,
            typename Skipper>
  struct Grammar_<Iterator, std::map<KeyT, ValueT, KeyCompareT, AllocatorT>, Skipper> {
    using Grammar = MapGrammar<Iterator, std::map<KeyT, ValueT, KeyCompareT, AllocatorT>, Skipper>;
  };
  template <typename Iterator, typename KeyT, typename ValueT, typename HashT, typename KeyEqT, typename AllocatorT,
            typename Skipper>
  struct Grammar_<Iterator, std::unordered_map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>, Skipper> {
    using Grammar = MapGrammar<Iterator, std::unordered_map<KeyT, ValueT, HashT, KeyEqT, AllocatorT>, Skipper>;
  };
  template <typename Iterator, typename KeyT, typename ValueT, typename KeyCompareT, typename AllocatorT,
            typename Skipper>
  struct Grammar_<Iterator, GaudiUtils::VectorMap<KeyT, ValueT, KeyCompareT, AllocatorT>, Skipper> {
    using Grammar = MapGrammar<Iterator, GaudiUtils::VectorMap<KeyT, ValueT, KeyCompareT, AllocatorT>, Skipper>;
  };

  template <typename Iterator, typename PointT, typename Skipper>
  struct Pnt3DGrammar : qi::grammar<Iterator, PointT(), Skipper> {
    using ResultT = PointT;
    using Scalar  = typename PointT::Scalar;

    struct Operations {
      void operator()( ResultT& res, const Scalar& scalar, const char xyz ) const {
        switch ( xyz ) {
        case 'x':
          res.SetX( scalar );
          break;
        case 'y':
          res.SetY( scalar );
          break;
        case 'z':
          res.SetZ( scalar );
          break;
        default:
          break;
        }
      }
    };

    Pnt3DGrammar() : Pnt3DGrammar::base_type( point ) {
      point = list | ( '(' >> list >> ')' ) | ( '[' >> list >> ']' );
      list = -( enc::no_case[qi::lit( "x" ) | qi::lit( "px" )] >> ':' ) >> scalar[op( qi::_val, qi::_1, 'x' )] >> ',' >>
             -( enc::no_case[qi::lit( "y" ) | qi::lit( "py" )] >> ':' ) >> scalar[op( qi::_val, qi::_1, 'y' )] >> ',' >>
             -( enc::no_case[qi::lit( "z" ) | qi::lit( "pz" )] >> ':' ) >> scalar[op( qi::_val, qi::_1, 'z' )];
    }

    qi::rule<Iterator, ResultT(), Skipper>                point, list;
    typename Grammar_<Iterator, Scalar, Skipper>::Grammar scalar;
    ph::function<Operations>                              op;
  };

  template <typename Iterator, typename T1, typename T2, typename Skipper>
  struct Grammar_<Iterator, ROOT::Math::PositionVector3D<T1, T2>, Skipper> {
    using Grammar = Pnt3DGrammar<Iterator, ROOT::Math::PositionVector3D<T1, T2>, Skipper>;
  };

  template <typename Iterator, typename T1, typename T2, typename Skipper>
  struct Grammar_<Iterator, ROOT::Math::DisplacementVector3D<T1, T2>, Skipper> {
    using Grammar = Pnt3DGrammar<Iterator, ROOT::Math::DisplacementVector3D<T1, T2>, Skipper>;
  };

  template <typename Iterator, typename PointT, typename Skipper>
  struct Pnt4DGrammar : qi::grammar<Iterator, PointT(), Skipper> {
    using ResultT = PointT;
    using ScalarT = typename PointT::Scalar;

    struct Operations {
      void operator()( ResultT& res, const ScalarT& scalar, const char xyz ) const {
        switch ( xyz ) {
        case 'x':
          res.SetPx( scalar );
          break;
        case 'y':
          res.SetPy( scalar );
          break;
        case 'z':
          res.SetPz( scalar );
          break;
        case 'e':
          res.SetE( scalar );
          break;
        default:
          break;
        }
      }
      void operator()( ResultT& res, const ResultT& xyz ) const {
        res.SetPx( xyz.Px() );
        res.SetPy( xyz.Py() );
        res.SetPz( xyz.Pz() );
      }
    };

    Pnt4DGrammar() : Pnt4DGrammar::base_type( point4d ) {
      point4d = list4d | ( '(' >> list4d >> ')' ) | ( '[' >> list4d >> ']' );
      list4d  = ( point3d[op( qi::_val, qi::_1 )] >> enc::char_( ";," ) >> e[op( qi::_val, qi::_1, 'e' )] ) |
               ( e[op( qi::_val, qi::_1, 'e' )] >> enc::char_( ";," ) >> point3d[op( qi::_val, qi::_1 )] );
      e       = -( enc::no_case[enc::char_( "te" )] >> ':' ) >> scalar[qi::_val = qi::_1];
      point3d = list3d | ( '(' >> list3d >> ')' ) | ( '[' >> list3d >> ']' );
      list3d  = -( enc::no_case[qi::lit( "x" ) | qi::lit( "px" )] >> ':' ) >> scalar[op( qi::_val, qi::_1, 'x' )] >>
               ',' >> -( enc::no_case[qi::lit( "y" ) | qi::lit( "py" )] >> ':' ) >>
               scalar[op( qi::_val, qi::_1, 'y' )] >> ',' >>
               -( enc::no_case[qi::lit( "z" ) | qi::lit( "pz" )] >> ':' ) >> scalar[op( qi::_val, qi::_1, 'z' )];
    }

    qi::rule<Iterator, ResultT(), Skipper>                 point3d, point4d, list3d, list4d;
    qi::rule<Iterator, ScalarT(), Skipper>                 e;
    typename Grammar_<Iterator, ScalarT, Skipper>::Grammar scalar;
    ph::function<Operations>                               op;
  };

  template <typename Iterator, typename T1, typename Skipper>
  struct Grammar_<Iterator, ROOT::Math::LorentzVector<T1>, Skipper> {
    typedef Pnt4DGrammar<Iterator, ROOT::Math::LorentzVector<T1>, Skipper> Grammar;
  };

  template <typename Iterator, typename Skipper>
  struct Histo1DGrammar : qi::grammar<Iterator, Gaudi::Histo1DDef(), Skipper> {
    using ResultT = Gaudi::Histo1DDef;

    // sub-grammars
    StringGrammar<Iterator, Skipper> title;

    // rules
    qi::rule<Iterator, ResultT(), Skipper> hist;
    qi::rule<Iterator, ResultT(), Skipper> val1, val2, val3;

    Histo1DGrammar() : Histo1DGrammar::base_type( hist ) {
      // convenience setters (Phoenix binds)
      auto setTitle = ph::bind( &ResultT::setTitle, qi::_val, qi::_1 );
      auto setLo    = ph::bind( &ResultT::setLowEdge, qi::_val, qi::_1 );
      auto setHi    = ph::bind( &ResultT::setHighEdge, qi::_val, qi::_1 );
      auto setBins  = ph::bind( &ResultT::setBins, qi::_val, qi::_1 );

      // title, lo, hi [, bins]
      val1 = title[setTitle] >> ',' >> qi::double_[setLo] >> ',' >> qi::double_[setHi] >> -( ',' >> qi::int_[setBins] );

      // lo, hi, title [, bins]
      val2 = qi::double_[setLo] >> ',' >> qi::double_[setHi] >> ',' >> title[setTitle] >> -( ',' >> qi::int_[setBins] );

      // lo, hi [, title] [, bins]
      val3 = qi::double_[setLo] >> ',' >> qi::double_[setHi] >> -( ',' >> title[setTitle] ) >>
             -( ',' >> qi::int_[setBins] );

      // accept either '(...)' or '[...]' around the payload;
      hist = rep::confix( '(', ')' )[val1 | val2 | val3] | rep::confix( '[', ']' )[val1 | val2 | val3];
    }
  };
  REGISTER_GRAMMAR( Gaudi::Histo1DDef, Histo1DGrammar );

  template <typename Iterator, typename Skipper>
  struct KeyValueGrammar : qi::grammar<Iterator, std::pair<std::string, std::string>(), Skipper> {
    typedef std::pair<std::string, std::string> ResultT;

    struct first {};
    struct second {};

    KeyValueGrammar() : KeyValueGrammar::base_type( pair ) { pair = gstring >> ":" >> +enc::char_; }

    StringGrammar<Iterator, Skipper>       gstring;
    qi::rule<Iterator, ResultT(), Skipper> pair;
  };
  // We don't register KeyalueGrammar because it's a special parser
} // namespace Gaudi::Parsers
