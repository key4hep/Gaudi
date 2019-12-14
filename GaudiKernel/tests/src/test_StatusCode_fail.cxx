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
#include "GaudiKernel/StatusCode.h"

int main() {
  {
    StatusCode sc = 42; // FAIL01: no implicit conversion from int
    int        i  = sc; // FAIL02: no implicit conversion to int
    bool       b  = sc; // FAIL03: no implicit conversion to bool
    (void)i;
    (void)b; // silence "unused" compiler warnings
  }
  {
    StatusCode sc1, sc2;
    StatusCode sc = sc1 && sc2; // FAIL04: no conversion from bool
  }
}
