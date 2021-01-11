/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "COPYING".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_PropertyHolder
#include <boost/test/unit_test.hpp>

#include <Gaudi/Property.h>

struct MyClass {};

using WPR = Gaudi::Details::WeakPropertyRef;

BOOST_AUTO_TEST_CASE( default_constructor ) {
  WPR r;
  BOOST_CHECK( !r.isBound() );
  BOOST_CHECK( !r.isSet() );
  BOOST_CHECK_EQUAL( std::string{r}, "" );
  r = "42";
  BOOST_CHECK( r.isSet() );
  BOOST_CHECK_EQUAL( std::string{r}, "42" );
}

BOOST_AUTO_TEST_CASE( construct_from_string ) {
  WPR r{"42"};
  BOOST_CHECK( !r.isBound() );
  BOOST_CHECK( r.isSet() );
  BOOST_CHECK_EQUAL( std::string{r}, "42" );
}

BOOST_AUTO_TEST_CASE( construct_from_property ) {
  Gaudi::Property<int> p;

  WPR r{p};

  BOOST_CHECK( r.isBound() );
  BOOST_CHECK( !r.isSet() );
}

BOOST_AUTO_TEST_CASE( assignment ) {
  {
    WPR r;
    r = "42";
    BOOST_CHECK( r.isSet() );
    BOOST_CHECK_EQUAL( std::string{r}, "42" );
  }
}

BOOST_AUTO_TEST_CASE( binding ) {
  {
    Gaudi::Property<int> p;
    {
      WPR r{p};
      r = "42";
      BOOST_CHECK( r.isSet() );
      BOOST_CHECK_EQUAL( p.value(), 42 );
    }
    // it would be nice to check that p knows about r, until r goes out of scope
  }

  {
    WPR r;
    {
      Gaudi::Property<int> p;
      BOOST_CHECK_EQUAL( p.value(), 0 );
      r = p;
      BOOST_CHECK( r.isBound() );
      BOOST_CHECK( !r.isSet() );
    }
    BOOST_CHECK( !r.isBound() );

    r = "42";
    {
      Gaudi::Property<int> p;
      r = p;
      BOOST_CHECK( r.isBound() );
      BOOST_CHECK( r.isSet() );
      BOOST_CHECK_EQUAL( p.value(), 42 );

      p = 100;
      BOOST_CHECK_EQUAL( std::string{r}, "100" );
    }

    BOOST_CHECK_EQUAL( std::string{r}, "42" );
  }
}

BOOST_AUTO_TEST_CASE( moving ) {
  {
    WPR r;
    BOOST_CHECK( !r.isBound() );
    BOOST_CHECK( !r.isSet() );
    BOOST_CHECK_EQUAL( std::string{r}, "" );

    r = WPR{"42"};
    BOOST_CHECK( !r.isBound() );
    BOOST_CHECK( r.isSet() );
    BOOST_CHECK_EQUAL( std::string{r}, "42" );
  }
  {
    WPR r{WPR{"42"}};
    BOOST_CHECK( !r.isBound() );
    BOOST_CHECK( r.isSet() );
    BOOST_CHECK_EQUAL( std::string{r}, "42" );
  }
  {
    Gaudi::Property<int> p{42};
    WPR                  r{WPR{p}};
    BOOST_CHECK( r.isBound() );
    BOOST_CHECK( !r.isSet() );
    BOOST_CHECK_EQUAL( std::string{r}, "42" );
  }
}

BOOST_AUTO_TEST_CASE( exceptions ) {
  auto orig_policy =
      Gaudi::Details::Property::setParsingErrorPolicy( Gaudi::Details::Property::ParsingErrorPolicy::Exception );
  {
    Gaudi::Property<int> p{42};
    WPR                  r{p};
    BOOST_CHECK_THROW( r = "'abc'", GaudiException );
    BOOST_CHECK_EQUAL( p.value(), 42 );
  }
  {
    WPR                  r{"'abc'"};
    Gaudi::Property<int> p{42};
    BOOST_CHECK_THROW( r = p, GaudiException );
    BOOST_CHECK_EQUAL( p.value(), 42 );
  }
  Gaudi::Details::Property::setParsingErrorPolicy( orig_policy );
}
