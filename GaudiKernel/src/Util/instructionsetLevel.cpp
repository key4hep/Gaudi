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
  std::vector<std::string> sets{
      // "80386",
      "sse",    "sse2",
      "sse3", // FIXME: This is not reported by Linux (?)
      "ssse3",  "sse4_1", "sse4_2", "avx", "avx2",
      "avx512f" // FIXME: This is not reported by Linux (?)
  };

  if ( argc == 1 ) {
    size_t level = System::instructionsetLevel() - 1;
    if ( level < sets.size() )
      for ( int i = System::instructionsetLevel() - 1; i >= 0; --i ) { std::cout << sets[i] << std::endl; }
    else {
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
