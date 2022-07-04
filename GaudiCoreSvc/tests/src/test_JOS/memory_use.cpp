/***********************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
/// file: Simple command that fills a list of properties from the lines on the stdin.
///
/// For example it can be used as
/// ```
/// heaptrack JOS_memory_use legacy < list_of_property_names.txt
/// ```
///
/// The argument to `JOS_memory_use` is either
///
/// - `legacy`: properties stored in a `std::unordered_map<std::string, ...>`
/// - `default`: properties stored in a `std::unordered_map<PropertyId, ...>`
///

#include "../../../src/JobOptionsSvc/PropertyId.h"
#include <array>
#include <iostream>
#include <string>
#include <unordered_map>

const std::array<std::string, 2> allowed_modes{ "default", "legacy" };

int main( int argc, char* argv[] ) {
  if ( argc != 2 || find( allowed_modes.begin(), allowed_modes.end(), argv[1] ) == allowed_modes.end() ) {
    std::cerr << "usage: " << argv[0] << " <mode>\n\n  where <mode> is any of:";
    for ( const auto& mode : allowed_modes ) std::cerr << ' ' << mode;
    std::cerr << '\n';
    return 1;
  }
  std::string mode = argv[1];

  if ( mode == "legacy" ) {
    std::unordered_map<std::string, int> props;
    for ( std::string line; std::getline( std::cin, line ); ) { props[line] = 0; }
  } else {
    std::unordered_map<Gaudi::Details::PropertyId, int> props;
    for ( std::string line; std::getline( std::cin, line ); ) { props[line] = 0; }
  }
  return 0;
}
