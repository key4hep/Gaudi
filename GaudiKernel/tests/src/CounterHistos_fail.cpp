/***********************************************************************************\
* (c) Copyright 2022 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Accumulators/Histogram.h>
#include <Gaudi/Algorithm.h>

// we do not need a real algorithm, just enough for the compiler to reach the bit of
// code that we want to fail
extern Gaudi::Algorithm* dummy_alg;

int main() {
  Gaudi::Accumulators::Histogram<2> h2d{ dummy_alg, "h2d", "2D Histogram", { { 50, -5, 5, "X" }, { 50, -5, 5, "Y" } } };
  // Try to fill the histogram with a single parameter
  ++h2d[1.2]; // ERROR: cannot fill a 2D histogram with a single index
}
