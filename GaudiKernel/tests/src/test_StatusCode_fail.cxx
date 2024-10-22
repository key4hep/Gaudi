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
#include <GaudiKernel/StatusCode.h>

int main() {
#ifdef CASE01
  {
    StatusCode sc{};
    sc = 42; // FAIL01: no implicit conversion from int
  }
#endif
#ifdef CASE02
  {
    StatusCode sc{};
    int        i = sc; // FAIL02: no implicit conversion to int
    (void)i;           // silence "unused" compiler warnings
  }
#endif
#ifdef CASE03
  {
    StatusCode sc{};
    bool       b = sc; // FAIL03: no implicit conversion to bool
    (void)b;           // silence "unused" compiler warnings
  }
#endif
#ifdef CASE04
  {
    StatusCode sc1, sc2;
    StatusCode sc = sc1 && sc2; // FAIL04: no conversion from bool
  }
#endif
}
