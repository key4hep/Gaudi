/***********************************************************************************\
* (c) Copyright 2020 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Interfaces/IOptionsSvc.h>
#include <GaudiKernel/ISvcLocator.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <iterator>
#include <random>
#include <string>
#include <utility>
#include <vector>

#define NO_BOOST
#include "fixture.h"

// function adapted from from
// - https://stackoverflow.com/a/24586587
// - https://stackoverflow.com/a/12468109
// - https://en.cppreference.com/w/cpp/algorithm/generate_n#Example
std::string random_string( std::string::size_type length ) {
  auto randchar = []() {
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz"
                           "._";

    static std::mt19937 rng; // default constructed, seeded with fixed seed

    static std::uniform_int_distribution<std::string::size_type> pick( 0, sizeof( charset ) - 2 );
    return charset[pick( rng )];
  };
  std::string str( length, 0 );
  std::generate_n( str.begin(), length, randchar );
  return str;
}

int main( int argc, char* argv[] ) {
  Fixture f;

  auto& jos = Gaudi::svcLocator()->getOptsSvc();

  std::cout << "preparing the data" << std::endl;
  std::size_t n_entries = 10000;
  if ( argc > 1 ) { n_entries = std::atol( argv[1] ); }

  std::vector<std::pair<std::string, std::string>> m_input_data;
  m_input_data.reserve( n_entries );
  std::generate_n( back_inserter( m_input_data ), n_entries, []() {
    return std::pair{ random_string( 20 + std::rand() % 80 ), random_string( std::rand() % 20 ) };
  } );

  std::cout << "adding " << n_entries << " properties" << std::endl;

  auto start = std::chrono::high_resolution_clock::now();
  for ( const auto& item : m_input_data ) jos.set( item.first, item.second );
  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> diff = end - start;
  std::cout << "elapsed time: " << diff.count() << " s\n";
}
