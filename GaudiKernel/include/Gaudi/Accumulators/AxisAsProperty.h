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
#pragma once

#include <Gaudi/Accumulators/StaticHistogram.h>

#include <Gaudi/Parsers/Factory.h>
#include <GaudiKernel/ToStream.h>

#include <boost/spirit/include/qi.hpp>

/**
 * This file provides a Grammar for the type Gaudi::Accumulators::Axis
 * It allows to use that type from python with a format liks :
 *    ( nbins, min, max, title )
 * where title can be ommited.
 * note that for the parsing, min and max will be parsed as doubles before
 * being converted to the actual type used as Arithmetic for Axis, so an
 * automatic conversion between the 2 must be provided
 */
namespace Gaudi {
  namespace Parsers {

    namespace qi = boost::spirit::qi;

    template <typename Iterator, typename Skipper, typename Arithmetic>
    struct AxisGrammar : qi::grammar<Iterator, Gaudi::Accumulators::Axis<Arithmetic>(), qi::locals<char>, Skipper> {
      using Axis = Gaudi::Accumulators::Axis<Arithmetic>;
      struct StoreNbinsOp {
        void operator()( Axis& res, unsigned int const& nBins ) const { res.setNumBins( nBins ); }
      };
      struct StoreMinValueOp {
        void operator()( Axis& res, Arithmetic const& minValue ) const { res.setMinValue( minValue ); }
      };
      struct StoreMaxValueOp {
        void operator()( Axis& res, Arithmetic const& maxValue ) const { res.setMaxValue( maxValue ); }
      };
      struct StoreTitleOp {
        void operator()( Axis& res, std::string const& title ) const { res.setTitle( title ); }
      };
      AxisGrammar() : AxisGrammar::base_type( axis ) {
        begin = enc::char_( '[' )[qi::_val = ']'] | enc::char_( '(' )[qi::_val = ')'];
        end   = enc::char_( qi::_r1 );
        core  = qi::int_[storeNbins( qi::_val, qi::_1 )] >> "," >> qi::double_[storeMinValue( qi::_val, qi::_1 )] >>
               "," >> qi::double_[storeMaxValue( qi::_val, qi::_1 )] >>
               -( "," >> title[storeTitle( qi::_val, qi::_1 )] );
        axis = begin[qi::_a = qi::_1] >> core[qi::_val = qi::_1] >> end( qi::_a );
      }
      qi::rule<Iterator, Axis(), qi::locals<char>, Skipper> axis;
      qi::rule<Iterator, Axis(), Skipper>                   core;
      qi::rule<Iterator, char()>                            begin;
      qi::rule<Iterator, void( char )>                      end;
      StringGrammar<Iterator, Skipper>                      title;
      ph::function<StoreNbinsOp>                            storeNbins;
      ph::function<StoreMinValueOp>                         storeMinValue;
      ph::function<StoreMaxValueOp>                         storeMaxValue;
      ph::function<StoreTitleOp>                            storeTitle;
    };

    template <typename Iterator, typename Skipper, typename Arithmetic>
    struct Grammar_<Iterator, Gaudi::Accumulators::Axis<Arithmetic>, Skipper> {
      using Grammar = AxisGrammar<Iterator, Skipper, Arithmetic>;
    };

    // Parse function... nothing special, but it must be done explicitely.
    template <typename Arithmetic>
    StatusCode parse( Gaudi::Accumulators::Axis<Arithmetic>& result, const std::string& input ) {
      return parse_( result, input );
    }

  } // namespace Parsers

} // namespace Gaudi
