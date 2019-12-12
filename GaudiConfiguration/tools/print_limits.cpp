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
#include <cxxabi.h>
#include <iostream>
#include <limits>

template <typename T>
void print_limits_for() {
  using std::cout;
  int  status;
  auto realname = std::unique_ptr<char, decltype( free )*>(
      abi::__cxa_demangle( typeid( T ).name(), nullptr, nullptr, &status ), free );
  // the unary '+' is explained in https://stackoverflow.com/a/28414758
  cout << "    '" << realname.get() << "': (" << +std::numeric_limits<T>::min() << ", "
       << +std::numeric_limits<T>::max() << "),\n";
}

void print_limits() {
  using std::cout;
  cout << "{\n";
  print_limits_for<signed char>();
  print_limits_for<short>();
  print_limits_for<int>();
  print_limits_for<long>();
  print_limits_for<long long>();
  print_limits_for<unsigned char>();
  print_limits_for<unsigned short>();
  print_limits_for<unsigned int>();
  print_limits_for<unsigned long>();
  print_limits_for<unsigned long long>();
  cout << "}\n";
}
