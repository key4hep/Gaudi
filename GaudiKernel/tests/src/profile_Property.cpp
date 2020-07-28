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
#include <Gaudi/Property.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

int main() {
  const size_t N          = 1000000;
  const size_t NPropNames = 50;
  const auto   page_sz    = sysconf( _SC_PAGESIZE );

  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

  unsigned long vm_size, vm_rss;
  unsigned long vsize_pages, rss_pages;

  std::array<std::string, NPropNames> names;
  for ( size_t i = 0; i != NPropNames; ++i ) { names[i] = "PropertyName_" + std::to_string( i ); }

  {

    std::ifstream statm;
    statm.open( "/proc/self/statm" );
    statm >> vsize_pages >> rss_pages;
    vm_size = vsize_pages * page_sz;
    vm_rss  = rss_pages * page_sz;
    statm.close();
    // std::cout << vm_size << " " << vm_rss << std::endl;

    using Prop = Gaudi::Property<int>;
    std::vector<Prop> props;
    props.reserve( N );

    start = std::chrono::high_resolution_clock::now();

    for ( size_t i = 0; i != N; ++i ) {
      props.emplace_back( names[i % NPropNames], i,
                          "some long documentation string describing what the property is for" );
    }

    end = std::chrono::high_resolution_clock::now();

    statm.open( "/proc/self/statm" );
    statm >> vsize_pages >> rss_pages;
    vm_size = vsize_pages * page_sz - vm_size;
    vm_rss  = rss_pages * page_sz - vm_rss;
    statm.close();
    // std::cout << vm_size << " " << vm_rss << std::endl;

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "constructed " << N << " properties in " << elapsed_seconds.count() << " seconds ("
              << elapsed_seconds.count() * 1000000 / N << " us/Property)" << std::endl;
    std::cout << "used " << vm_size / 1024 << " kB VSIZE, " << vm_rss / 1024 << " kB RSS" << std::endl;
    std::cout << "used " << vm_size / N << " B/Property VSIZE, " << vm_rss / N << " B/Property RSS" << std::endl;
    std::cout << "sizeof(Prop) -> " << sizeof( Prop ) << std::endl;
    // the constant 21 is to take into account that Property::documentation() adds the owner of the property to the doc
    std::cout << "lengths of prop strings -> " << props[0].name().length() + props[0].documentation().length() - 21
              << std::endl;
  }
}
