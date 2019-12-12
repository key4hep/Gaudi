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
 * CPUFamily.cpp
 *
 *  Created on: Sep 23, 2010
 */

/** @file CPUFamily.cpp
 *
 *  Python module to be able to check the processor family (Core/Nehalem)
 *
 */

#include <boost/python.hpp>

#define cpuid( func, ax, bx, cx, dx )                                                                                  \
  __asm__ __volatile__( "cpuid" : "=a"( ax ), "=b"( bx ), "=c"( cx ), "=d"( dx ) : "a"( func ) );

bool is_nehalem() {
  int a, b, c, d;
  cpuid( 1, a, b, c, d );
  int sse4_2_mask = 1 << 20;
  return ( c & sse4_2_mask );
}

const char* CPUFamily() {
  if ( is_nehalem() ) {
    return "nehalem";
  } else {
    return "core";
  }
}

BOOST_PYTHON_MODULE( PyCPUFamily ) {
  using namespace boost::python;
  def( "CPUFamily", CPUFamily );
}
