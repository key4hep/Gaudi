/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
/*
 * instructionsetLevel.cpp
 *
 *  Created on: Feb 3, 2015
 *      Author: Marco Clemencic
 */

#include <GaudiKernel/System.h>

#include <iostream>
#include <string>
#include <vector>

int main( int argc, char* argv[] ) {

  // These names are those reported in flags field of /proc/cpuinfo on Linux
  // See arch/x86/include/asm/cpufeature.h
  const std::vector<std::string> sets{// "80386",
                                      "sse",   "sse2",
                                      "sse3", // Note: This is reported by Linux as 'pni'
                                      "ssse3", "sse4_1", "sse4_2", "avx", "avx2", "avx512f", "avx512vl"};

  if ( argc == 1 ) {
    const std::size_t level = System::instructionsetLevel() - 1;
    if ( level < sets.size() ) {
      for ( int i = level; i >= 0; --i ) { std::cout << sets[i] << std::endl; }
    } else {
      std::cout << "unknown instruction set level: " << level << std::endl;
      return 2;
    }
  } else if ( ( argc == 2 ) && ( std::string{"all"} == std::string{argv[1]} ) ) {
    for ( auto& s : sets ) std::cout << s << std::endl;
  } else {
    std::cout << "Error: wrong arguments\nUsage:\n\t" << argv[0] << " [all]" << std::endl;
    return 1;
  }

  return 0;
}
