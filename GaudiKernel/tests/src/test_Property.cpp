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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_PropertyHolder
#include <boost/test/unit_test.hpp>

#include <Gaudi/Property.h>

struct MyClass {};

BOOST_AUTO_TEST_CASE( value_props_constructors ) {
  {
    Gaudi::Property<std::string> p;
    BOOST_CHECK_EQUAL( p.value(), "" );
    BOOST_CHECK_EQUAL( p.name(), "" );
    BOOST_CHECK_EQUAL( p.documentation(), "" );
    BOOST_CHECK_EQUAL( p.ownerTypeName(), "unknown owner type" );
  }
  {
    Gaudi::Property<std::string> p( "abc", "xyz" );
    BOOST_CHECK_EQUAL( p.value(), "xyz" );
    BOOST_CHECK_EQUAL( p.name(), "abc" );
    BOOST_CHECK_EQUAL( p.documentation(), "" );
    BOOST_CHECK_EQUAL( p.ownerTypeName(), "unknown owner type" );
  }
  {
    Gaudi::Property<std::string> p( "abc", "xyz", "doc" );
    BOOST_CHECK_EQUAL( p.value(), "xyz" );
    BOOST_CHECK_EQUAL( p.name(), "abc" );
    BOOST_CHECK_EQUAL( p.documentation(), "doc" );
    BOOST_CHECK_EQUAL( p.ownerTypeName(), "unknown owner type" );
  }
  {
    Gaudi::Property<int> p;
    BOOST_CHECK_EQUAL( p.value(), 0 );
    BOOST_CHECK_EQUAL( p.name(), "" );
    BOOST_CHECK_EQUAL( p.documentation(), "" );
    BOOST_CHECK_EQUAL( p.ownerTypeName(), "unknown owner type" );
  }
  {
    Gaudi::Property<int> p( "abc", 456 );
    BOOST_CHECK_EQUAL( p.value(), 456 );
    BOOST_CHECK_EQUAL( p.name(), "abc" );
    BOOST_CHECK_EQUAL( p.documentation(), "" );
    BOOST_CHECK_EQUAL( p.ownerTypeName(), "unknown owner type" );
  }
  {
    Gaudi::Property<int> p( "abc", 456, "doc" );
    BOOST_CHECK_EQUAL( p.value(), 456 );
    BOOST_CHECK_EQUAL( p.name(), "abc" );
    BOOST_CHECK_EQUAL( p.documentation(), "doc" );
    BOOST_CHECK_EQUAL( p.ownerTypeName(), "unknown owner type" );
  }
  {
    Gaudi::Property<int> p( "abc", 456, "doc" );
    p.setOwnerType<MyClass>();
    BOOST_CHECK_EQUAL( p.value(), 456 );
    BOOST_CHECK_EQUAL( p.name(), "abc" );
    BOOST_CHECK_EQUAL( p.documentation(), "doc" );
    BOOST_CHECK_EQUAL( p.ownerTypeName(), "MyClass" );
  }
}

BOOST_AUTO_TEST_CASE( string_conversion ) {
  {
    Gaudi::Property<std::string> p1{ "p1", "" };
    BOOST_CHECK_EQUAL( p1.value(), "" );
    p1 = "abc";
    BOOST_CHECK_EQUAL( p1.value(), "abc" );
    BOOST_CHECK( p1.fromString( "'xyz'" ) );
    BOOST_CHECK_EQUAL( p1.value(), "xyz" );
    BOOST_CHECK_EQUAL( p1.toString(), "xyz" );

    p1 = "with \"quotes\" inside";
    BOOST_CHECK_EQUAL( p1.value(), "with \"quotes\" inside" );
    BOOST_CHECK_EQUAL( p1.toString(), "with \"quotes\" inside" );

    Gaudi::Property<std::string> tgt{ "p1", "" };

    auto sc = tgt.assign( p1 );
    BOOST_CHECK( sc );
    BOOST_CHECK_EQUAL( tgt.value(), "with \"quotes\" inside" );

    sc = p1.load( tgt );
    BOOST_CHECK( sc );
    BOOST_CHECK_EQUAL( tgt.value(), "with \"quotes\" inside" );
  }
  {
    Gaudi::Property<int> p2{ "p2", 10 };
    BOOST_CHECK_EQUAL( p2.value(), 10 );
    p2 = 20;
    BOOST_CHECK_EQUAL( p2.value(), 20 );
    BOOST_CHECK( p2.fromString( "123" ) );
    BOOST_CHECK_EQUAL( p2.value(), 123 );
    BOOST_CHECK_EQUAL( p2.toString(), "123" );
  }
  {
    Gaudi::Property<bool> p3{ "p3", true };
    BOOST_CHECK_EQUAL( p3.value(), true );
    p3 = false;
    BOOST_CHECK_EQUAL( p3.value(), false );
    BOOST_CHECK( p3.fromString( "true" ) );
    BOOST_CHECK_EQUAL( p3.value(), true );
    BOOST_CHECK_EQUAL( p3.toString(), "True" );
  }
}

template <class T>
T convert_to( T v ) {
  return v;
}

BOOST_AUTO_TEST_CASE( implicit_conversion ) {
  {
    Gaudi::Property<int> p( "p", 123 );
    BOOST_CHECK_EQUAL( convert_to<int>( p ), 123 );
    BOOST_CHECK_EQUAL( convert_to<long>( p ), 123 );
    BOOST_CHECK_EQUAL( convert_to<double>( p ), 123 );
  }
  {
    Gaudi::Property<bool> p( "p", true );
    BOOST_CHECK_EQUAL( convert_to<int>( p ), 1 );
    BOOST_CHECK_EQUAL( convert_to<bool>( p ), true );
  }
  {
    Gaudi::Property<bool> p( "p", false );
    BOOST_CHECK_EQUAL( convert_to<int>( p ), 0 );
    BOOST_CHECK_EQUAL( convert_to<bool>( p ), false );
  }
  {
    Gaudi::Property<std::string> p( "p", "Hello World" );
    BOOST_CHECK_EQUAL( convert_to<std::string_view>( p ), "Hello World" );
  }
}

BOOST_AUTO_TEST_CASE( copy_constructor ) {
  {
    Gaudi::Property<std::string> orig{ "name", "value", "doc" };
    Gaudi::Property<std::string> dest( orig );
    BOOST_CHECK_EQUAL( dest.name(), "name" );
    BOOST_CHECK_EQUAL( dest.value(), "value" );
    BOOST_CHECK_EQUAL( dest.documentation(), "doc" );
    BOOST_CHECK_EQUAL( dest.ownerTypeName(), "unknown owner type" );
  }
  {
    std::string                   data{ "value" };
    Gaudi::Property<std::string&> orig{ "name", data, "doc" };
    Gaudi::Property<std::string&> dest( orig );
    BOOST_CHECK_EQUAL( dest.name(), "name" );
    BOOST_CHECK_EQUAL( dest.documentation(), "doc" );
    BOOST_CHECK_EQUAL( dest.ownerTypeName(), "unknown owner type" );
    BOOST_CHECK_EQUAL( dest.value(), "value" );
    data = "newvalue";
    BOOST_CHECK_EQUAL( dest.value(), "newvalue" );
  }
}
BOOST_AUTO_TEST_CASE( move_constructor ) {
  Gaudi::Property<std::string> orig{ "name", "value", "doc" };
  Gaudi::Property<std::string> dest( std::move( orig ) );
  BOOST_CHECK_EQUAL( dest.name(), "name" );
  BOOST_CHECK_EQUAL( dest.value(), "value" );
  BOOST_CHECK_EQUAL( dest.documentation(), "doc" );
  BOOST_CHECK_EQUAL( dest.ownerTypeName(), "unknown owner type" );
}
BOOST_AUTO_TEST_CASE( copy_assignment ) {
  Gaudi::Property<std::string> orig{ "name", "value", "doc" };
  Gaudi::Property<std::string> dest = orig;
  BOOST_CHECK_EQUAL( dest.name(), "name" );
  BOOST_CHECK_EQUAL( dest.value(), "value" );
  BOOST_CHECK_EQUAL( dest.documentation(), "doc" );
  BOOST_CHECK_EQUAL( dest.ownerTypeName(), "unknown owner type" );
}
BOOST_AUTO_TEST_CASE( move_assignment ) {
  Gaudi::Property<std::string> orig{ "name", "value", "doc" };
  Gaudi::Property<std::string> dest = std::move( orig );
  BOOST_CHECK_EQUAL( dest.name(), "name" );
  BOOST_CHECK_EQUAL( dest.value(), "value" );
  BOOST_CHECK_EQUAL( dest.documentation(), "doc" );
  BOOST_CHECK_EQUAL( dest.ownerTypeName(), "unknown owner type" );
}

BOOST_AUTO_TEST_CASE( backward_compatibility ) {
  {
    Gaudi::Property<int> ip{ "name", 42 };
    BOOST_CHECK( !ip.readCallBack() );
    BOOST_CHECK( !ip.updateCallBack() );

    Gaudi::Details::PropertyBase* p = &ip;
    BOOST_CHECK( !p->readCallBack() );
    BOOST_CHECK( !p->updateCallBack() );
    BOOST_CHECK( 0 == p->updateCallBack() );

    p->declareUpdateHandler( []( Gaudi::Details::PropertyBase& ) {} );
    BOOST_CHECK( ip.updateCallBack() );
    BOOST_CHECK( p->updateCallBack() );
  }

  {
    Gaudi::Property<std::string> p{ "OutputLevel", "6" };
    Gaudi::Property<int>         i;
    i.assign( p );
    BOOST_CHECK_EQUAL( i.value(), 6 );
  }
  {
    Gaudi::Property<int> p{ "OutputLevel", 6 };
    Gaudi::Property<int> i;
    i.assign( p );
    BOOST_CHECK_EQUAL( i.value(), 6 );
  }
  {
    Gaudi::Property<std::string> p{ "OutputLevel", "6" };
    Gaudi::Property<std::string> i;
    i.assign( p );
    BOOST_CHECK_EQUAL( i.value(), "6" );
  }
}

BOOST_AUTO_TEST_CASE( backward_compatibility_2 ) {
  // string conversion compatibility
  {
    Gaudi::Property<int>         dst{ "name", 0 };
    Gaudi::Property<std::string> src{ "name", "321" };
    BOOST_CHECK_EQUAL( dst.value(), 0 );
    BOOST_CHECK( dst.assign( src ) );
    BOOST_CHECK_EQUAL( dst.value(), 321 );
    dst = 100;
    BOOST_CHECK_EQUAL( dst.value(), 100 );
    BOOST_CHECK( dst.load( src ) );
    BOOST_CHECK_EQUAL( src.value(), "100" );
  }
  {
    // string property as from options (old JobOptionsSvc)
    Gaudi::Property<std::string> opt{ "opt", "\"NONE\"" };
    Gaudi::Property<std::string> p{};
    BOOST_CHECK( opt.load( p ) );
    BOOST_CHECK_EQUAL( p.value(), "NONE" );
  }
  {
    // string property as from options (old JobOptionsSvc)
    Gaudi::Property<std::string>  opt{ "opt", "\"NONE\"" };
    std::string                   dst;
    Gaudi::Property<std::string&> p{ "test", dst };
    BOOST_CHECK( opt.load( p ) );
    BOOST_CHECK_EQUAL( p.value(), "NONE" );
    BOOST_CHECK_EQUAL( dst, "NONE" );
  }
  {
    Gaudi::Property<std::string> p1{ "p1", "" };
    BOOST_CHECK_EQUAL( p1.value(), "" );
    p1 = "abc";
    BOOST_CHECK_EQUAL( p1.value(), "abc" );
    BOOST_CHECK( p1.fromString( "xyz" ) );
    BOOST_CHECK_EQUAL( p1.value(), "xyz" );
    BOOST_CHECK_EQUAL( p1.toString(), "xyz" );
  }
  { // this covers the fix to a segfault observed in a few cases
    Gaudi::Property<std::string, Gaudi::Details::Property::NullVerifier, Gaudi::Details::Property::NoHandler> src{
        "src", "42" };
    Gaudi::Property<int> dst{ "dst", 0 };

    BOOST_CHECK( dst.assign( src ) );
    BOOST_CHECK_EQUAL( dst.value(), 42 );
  }
}
