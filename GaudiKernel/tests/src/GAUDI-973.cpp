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
#include <GaudiKernel/StringKey.h>

#include <iostream>
#include <unordered_map>

int main() {
  std::unordered_map<Gaudi::StringKey, int> m = { { "one", 1 }, { "two", 2 } };

  std::cout << "one -> " << m["one"] << std::endl;
  std::cout << "two -> " << m["two"] << std::endl;

  return 0;
}
