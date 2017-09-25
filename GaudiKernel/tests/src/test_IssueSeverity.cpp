#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_IssueSeverity
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>

#include "GaudiKernel/IssueSeverity.h"

// Use function return value to generate rvalue references
IssueSeverity returnIssue()
{
  return IssueSeverity{IssueSeverity::Level::INFO, 666, "ghost.cpp", "issue severity test"};
}

BOOST_AUTO_TEST_CASE( constructor )
{
  {
    std::cout << "constructor full" << std::endl;
    IssueSeverity is{IssueSeverity::Level::INFO, 666, "ghost.cpp", "issue severity test"};
    BOOST_CHECK( is.getLevel() == IssueSeverity::Level::INFO );
    BOOST_CHECK( is.getOrigin() == std::string( "ghost.cpp:666" ) );
    BOOST_CHECK( is.getMsg() == "issue severity test" );
  }
  {
    std::cout << "constructor partial" << std::endl;
    IssueSeverity is{IssueSeverity::Level::INFO, "issue severity test"};
    BOOST_CHECK( is.getLevel() == IssueSeverity::Level::INFO );
    BOOST_CHECK( is.getMsg() == "issue severity test" );
  }
  {
    std::cout << "constructor move assign" << std::endl;
    IssueSeverity is = returnIssue();
    BOOST_CHECK( is.getLevel() == IssueSeverity::Level::INFO );
    BOOST_CHECK( is.getOrigin() == std::string( "ghost.cpp:666" ) );
    BOOST_CHECK( is.getMsg() == "issue severity test" );
  }
  {
    std::cout << "constructor from rval ref" << std::endl;
    IssueSeverity is( returnIssue() );
    BOOST_CHECK( is.getLevel() == IssueSeverity::Level::INFO );
    BOOST_CHECK( is.getOrigin() == std::string( "ghost.cpp:666" ) );
    BOOST_CHECK( is.getMsg() == "issue severity test" );
  }
}

BOOST_AUTO_TEST_CASE( setter )
{
  {
    std::cout << "setters" << std::endl;
    IssueSeverity is{IssueSeverity::Level::INFO, 666, "ghost.cpp", "issue severity test"};
    is.setLevel( IssueSeverity::Level::ERROR );
    BOOST_CHECK( is.getLevel() == IssueSeverity::Level::ERROR );
    is.setMsg( "updated message" );
    BOOST_CHECK( is.getMsg() == "updated message" );
  }
}
