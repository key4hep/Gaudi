#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_StatusCode
#include <boost/test/unit_test.hpp>

#include "GaudiKernel/StatusCode.h"

BOOST_AUTO_TEST_CASE( values )
{
  // default should be SUCCESS
  {
    StatusCode sc;
    BOOST_CHECK( sc == StatusCode::SUCCESS );
  }

  // SUCCESS
  {
    StatusCode sc( StatusCode::SUCCESS );
    BOOST_CHECK( sc == StatusCode::SUCCESS );
    BOOST_CHECK( sc.getCode() == StatusCode::SUCCESS );
    BOOST_CHECK( sc.isSuccess() == true );
    BOOST_CHECK( sc.isFailure() == false );
    BOOST_CHECK( sc.isRecoverable() == false );
    BOOST_CHECK( static_cast<bool>( sc ) == true );
  }

  // FAILURE
  {
    StatusCode sc( StatusCode::FAILURE );
    BOOST_CHECK( sc == StatusCode::FAILURE );
    BOOST_CHECK( sc.getCode() == StatusCode::FAILURE );
    BOOST_CHECK( sc.isSuccess() == false );
    BOOST_CHECK( sc.isFailure() == true );
    BOOST_CHECK( sc.isRecoverable() == false );
    BOOST_CHECK( static_cast<bool>( sc ) == false );
  }

  // RECOVERABLE
  {
    StatusCode sc( StatusCode::RECOVERABLE );
    BOOST_CHECK( sc == StatusCode::RECOVERABLE );
    BOOST_CHECK( sc.getCode() == StatusCode::RECOVERABLE );
    BOOST_CHECK( sc.isSuccess() == false );
    BOOST_CHECK( sc.isFailure() == true ); // !!! note, this particularity
    BOOST_CHECK( sc.isRecoverable() == true );
    BOOST_CHECK( static_cast<bool>( sc ) == true ); // !!!
  }
}

BOOST_AUTO_TEST_CASE( user_values )
{
  enum Status { ERR1 = 10, ERR2 = 12 };

  {
    StatusCode sc( Status::ERR1 );
    BOOST_CHECK( sc.isSuccess() == false );
    BOOST_CHECK( sc.isFailure() == true );
    BOOST_CHECK( sc.isRecoverable() == false );
    BOOST_CHECK( static_cast<bool>( sc ) == true ); // !!!

    sc = Status::ERR2;
    BOOST_CHECK( sc == Status::ERR2 );
  }
}

BOOST_AUTO_TEST_CASE( comparison )
{
  // Copy and comparison
  {
    StatusCode sc;
    StatusCode sc2( sc );
    BOOST_CHECK( sc == sc2 );

    sc2.setCode( StatusCode::FAILURE );
    BOOST_CHECK( sc > sc2 );
    BOOST_CHECK( sc2 < sc );
  }
}

BOOST_AUTO_TEST_CASE( checking )
{
  {
    StatusCode sc( StatusCode::SUCCESS );
    BOOST_CHECK( sc.checked() == false );
    sc.isSuccess();
    BOOST_CHECK( sc.checked() == true );
  }
  {
    StatusCode sc( StatusCode::SUCCESS );
    sc.ignore();
    BOOST_CHECK( sc.checked() == true );
  }
  // Comparison invokes implicit cast --> checked
  {
    StatusCode sc( StatusCode::SUCCESS );
    if ( sc == StatusCode::SUCCESS )
      ;
    BOOST_CHECK( sc.checked() == true );
  }
  // Using the comparison operator does not check
  // This is probably unintentional and we may want to change this
  {
    StatusCode sc1( StatusCode::SUCCESS );
    StatusCode sc2( StatusCode::FAILURE );
    BOOST_CHECK( sc1 > sc2 );
    BOOST_CHECK( sc1.checked() == false ); // !!!
    BOOST_CHECK( sc2.checked() == false ); // !!!
  }
  // Cast to long --> checked
  {
    StatusCode sc;
    BOOST_CHECK( sc.checked() == false );
    if ( sc )
      ;
    BOOST_CHECK( sc.checked() == true );
  }
  // Copy constructor checks source
  {
    StatusCode sc1( StatusCode::SUCCESS );
    StatusCode sc2( sc1 );
    BOOST_CHECK( sc1.checked() == true );
    BOOST_CHECK( sc2.checked() == false );
  }
  // Assignment operator checks rhs
  {
    StatusCode sc1( StatusCode::SUCCESS );
    StatusCode sc2;
    sc2 = sc1;
    BOOST_CHECK( sc1.checked() == true );
    BOOST_CHECK( sc2.checked() == false );
  }
}
