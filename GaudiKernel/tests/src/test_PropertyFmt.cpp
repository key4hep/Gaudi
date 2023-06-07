/***********************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/PropertyFmt.h>
#include <string>
#include <string_view>
#include <vector>

// this is needed to format std::vector
#include <fmt/ranges.h>

#if __has_include( <catch2/catch.hpp>)
// Catch2 v2
#  include <catch2/catch.hpp>
#else
// Catch2 v3
#  include <catch2/catch_template_test_macros.hpp>
#  include <catch2/catch_test_macros.hpp>
#endif

namespace {
  template <typename T>
  struct test_data {};

  template <>
  struct test_data<int> {
    std::string_view name{ "SomeInt" };
    int              value{ 42 };
    std::string_view fmt{ "42" };
    std::string_view dbg{ " 'SomeInt':42" };
  };

  template <>
  struct test_data<double> {
    std::string_view name{ "SomeDouble" };
    double           value{ 100 };
    std::string_view fmt{ "100" };
    std::string_view dbg{ " 'SomeDouble':100" };
  };

  template <>
  struct test_data<std::vector<int>> {
    std::string_view name{ "SomeVector" };
    std::vector<int> value{ 0, 1, 2, 3 };
    std::string_view fmt{ "[0, 1, 2, 3]" };
    std::string_view dbg{ " 'SomeVector':[0, 1, 2, 3]" };
  };

  template <>
  struct test_data<std::string> {
    std::string_view name{ "SomeString" };
    std::string      value{ "hello world" };
    std::string_view fmt{ "hello world" };
    std::string_view dbg{ " 'SomeString':'hello world'" };
  };
} // namespace

TEMPLATE_TEST_CASE( "property formatting", "", int, double, std::vector<int>, std::string ) {
  test_data<TestType>       data;
  Gaudi::Property<TestType> p{ std::string{ data.name }, data.value };
  CHECK( fmt::format( "{}", p ) == data.fmt );
  CHECK( fmt::format( "{:?}", p ) == data.dbg );
}
