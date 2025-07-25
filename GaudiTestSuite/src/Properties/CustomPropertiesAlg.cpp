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
#include "CustomPropertiesAlg.h"

// ----------------------------------------------------------------------------
//  Allow to use std::unordered_map as a property
// ----------------------------------------------------------------------------
// Includes needed for the custom type
#include <string>
#include <unordered_map>
// A typedef may save a lot of mistakes
typedef std::unordered_map<std::string, std::string> MyCustomType;

// Define the parser
#include <Gaudi/Parsers/Factory.h>

namespace Gaudi {
  namespace Parsers {

    // Parser grammar
    template <typename Iterator, typename Skipper>
    struct Grammar_<Iterator, MyCustomType, Skipper> {
      // In this case, the type is a mapping type, so it requires the MapGrammar.
      // For other grammars see Gaudi/Parsers/Grammars.h
      typedef MapGrammar<Iterator, MyCustomType, Skipper> Grammar;
    };

    // Parse function... nothing special, but it must be done explicitely.
    StatusCode parse( MyCustomType& result, std::string_view input ) { return parse_( result, input ); }
  } // namespace Parsers
} // namespace Gaudi

// We also need to be able to print an object of our type as a string that both
// Python and our parser can understand,
#include <GaudiKernel/ToStream.h>
#include <map>
namespace std {
  // This is an example valid for any mapping type.
  ostream& operator<<( ostream& s, const MyCustomType& m ) {
    bool first = true;
    s << '{';
    // this is not strictly needed, but it makes the output sorted, which is
    // always nice (in particular for tests)
    const map<string, string> m1( m.begin(), m.end() );
    for ( const auto& i : m1 ) {
      if ( first )
        first = false;
      else
        s << ", ";
      Gaudi::Utils::toStream( i.first, s ) << ": ";
      Gaudi::Utils::toStream( i.second, s );
    }
    s << '}';
    return s;
  }
} // namespace std

// ----------------------------------------------------------------------------
// Implementation file for class: CustomPropertiesAlg
//
// 14/11/2014: Marco Clemencic
// ----------------------------------------------------------------------------
namespace Gaudi {
  namespace TestSuite {
    DECLARE_COMPONENT( CustomPropertiesAlg )

    // ============================================================================
    // Initialization
    // ============================================================================
    StatusCode CustomPropertiesAlg::initialize() {
      StatusCode sc = TestAlg::initialize(); // must be executed first
      if ( sc.isFailure() ) return sc;       // error printed already by TestAlg

      if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Initialize" << endmsg;

      // TODO initialization procedure

      return StatusCode::SUCCESS;
    }

    // ============================================================================
    // Main execution
    // ============================================================================
    StatusCode CustomPropertiesAlg::execute() {
      if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Execute" << endmsg;

      // TODO execution logic

      return StatusCode::SUCCESS;
    }

    // ============================================================================
    // Finalize
    // ============================================================================
    StatusCode CustomPropertiesAlg::finalize() {
      if ( msgLevel( MSG::DEBUG ) ) debug() << "==> Finalize" << endmsg;

      // TODO Implement finalize

      return TestAlg::finalize(); // must be called after all other actions
    }
  } // namespace TestSuite
} // namespace Gaudi
// ============================================================================
