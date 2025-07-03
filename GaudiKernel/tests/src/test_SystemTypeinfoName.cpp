/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

// Set up a Boost unit test:
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_SystemTypeinfoName
#include <boost/test/unit_test.hpp>

#include <GaudiKernel/System.h>
#include <string>

/// Primitive test class used in the tests
class TestClass {};

/// Helper macro testing the return value of System::typeinfoName
#define CHECK_TYPE( TYPE ) BOOST_CHECK( System::typeinfoName( typeid( TYPE ) ) == #TYPE )

/// Test some simple basic types
BOOST_AUTO_TEST_CASE( simple_types ) {
  CHECK_TYPE( int );
  CHECK_TYPE( std::string );
  CHECK_TYPE( TestClass );

  CHECK_TYPE( int* );
  CHECK_TYPE( float const* );
}

/// Templated test class used in the tests
template <class T>
class TestContainer {};

/// Test some more complex, template types
BOOST_AUTO_TEST_CASE( template_types ) {
  CHECK_TYPE( TestContainer<int> );
  CHECK_TYPE( TestContainer<TestClass> );
  CHECK_TYPE( TestContainer<std::string> );

  CHECK_TYPE( TestContainer<int const*> );
  CHECK_TYPE( TestContainer<std::string*> );

  CHECK_TYPE( TestContainer<float const&> );
  CHECK_TYPE( TestContainer<std::string const&> );
}
