#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_StatusCode
#include <boost/test/unit_test.hpp>

#include "GaudiKernel/StatusCode.h"

// Define my own error enum/category
enum class MyErr : StatusCode::code_t {
  FAILURE           = 0,
  SUCCESS           = 1,
  RECOVERABLE       = 2,
  WARNING           = 10,
  OTHER_RECOVERABLE = 20
};

STATUSCODE_ENUM_DECL( MyErr )

namespace
{
  struct MyCategory : StatusCode::Category {
    const char* name() const override { return "My"; }

    bool isSuccess( StatusCode::code_t code ) const override
    {
      const MyErr e = static_cast<MyErr>( code );
      return e == MyErr::SUCCESS || ( e > MyErr::RECOVERABLE && e < MyErr::OTHER_RECOVERABLE );
    }

    bool isRecoverable( StatusCode::code_t code ) const override
    {
      return static_cast<MyErr>( code ) == MyErr::RECOVERABLE || static_cast<MyErr>( code ) == MyErr::OTHER_RECOVERABLE;
    }

    std::string message( StatusCode::code_t code ) const override
    {
      switch ( static_cast<MyErr>( code ) ) {
      case MyErr::WARNING:
        return "WARNING";
      case MyErr::OTHER_RECOVERABLE:
        return "OTHER_RECOVERABLE";
      default:
        return StatusCode::default_category().message( code );
      }
    }
  };
}

STATUSCODE_ENUM_IMPL( MyErr, MyCategory )

// Test old-style enums
enum OtherErrors { ERROR_1 = 100, ERROR_2 = 200 };

STATUSCODE_ENUM_DECL( OtherErrors )
STATUSCODE_ENUM_IMPL( OtherErrors )

// A test function
StatusCode func() { return StatusCode::SUCCESS; }

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
    BOOST_CHECK( sc.isSuccess() == true );
    BOOST_CHECK( sc.isFailure() == false );
    BOOST_CHECK( sc.isRecoverable() == false );
    BOOST_CHECK( static_cast<bool>( sc ) == true );
  }

  // FAILURE
  {
    StatusCode sc( StatusCode::FAILURE );
    BOOST_CHECK( sc == StatusCode::FAILURE );
    BOOST_CHECK( sc.isSuccess() == false );
    BOOST_CHECK( sc.isFailure() == true );
    BOOST_CHECK( sc.isRecoverable() == false );
    BOOST_CHECK( static_cast<bool>( sc ) == false );
  }

  // RECOVERABLE
  {
    StatusCode sc( StatusCode::RECOVERABLE );
    BOOST_CHECK( sc == StatusCode::RECOVERABLE );
    BOOST_CHECK( sc.isSuccess() == false );
    BOOST_CHECK( sc.isFailure() == true ); // !!! note, this particularity
    BOOST_CHECK( sc.isRecoverable() == true );
    BOOST_CHECK( static_cast<bool>( sc ) == false );
  }
}

BOOST_AUTO_TEST_CASE( comparison )
{
  // Copy and comparison
  {
    StatusCode sc;
    StatusCode sc2( sc );
    BOOST_CHECK( sc == sc2 );

    sc2 = StatusCode::FAILURE;
    //    BOOST_CHECK( sc > sc2 );   // not implemented
    BOOST_CHECK( sc2 < sc );
  }
  // Comparison ignores category for SUCCESS/FAILURE
  {
    StatusCode sc1 = StatusCode::FAILURE;
    StatusCode sc2 = MyErr::FAILURE;
    BOOST_CHECK( sc1 == sc2 );
  }
  // ...but not for other values
  {
    StatusCode sc1 = StatusCode::RECOVERABLE;
    StatusCode sc2 = MyErr::RECOVERABLE;
    BOOST_CHECK( sc1 != sc2 );
  }
}

BOOST_AUTO_TEST_CASE( ternary_and )
{
  StatusCode s = StatusCode::SUCCESS;
  StatusCode f = StatusCode::FAILURE;
  StatusCode r = StatusCode::RECOVERABLE;

  BOOST_CHECK( ( s & s ).isSuccess() );
  BOOST_CHECK( ( f & f ).isFailure() );
  BOOST_CHECK( ( r & r ).isRecoverable() );
  BOOST_CHECK( ( s & f ).isFailure() );
  BOOST_CHECK( ( f & s ).isFailure() );
  BOOST_CHECK( ( s & r ).isRecoverable() );
  BOOST_CHECK( ( r & s ).isRecoverable() );
  BOOST_CHECK( ( f & r ).isFailure() );
  BOOST_CHECK( ( r & f ).isFailure() );

  // Test compound assignment (just compilation, logic is checked above)
  StatusCode sc;
  BOOST_CHECK( ( ( sc = s ) &= f ).isFailure() );
}

BOOST_AUTO_TEST_CASE( ternary_or )
{
  StatusCode s = StatusCode::SUCCESS;
  StatusCode f = StatusCode::FAILURE;
  StatusCode r = StatusCode::RECOVERABLE;

  BOOST_CHECK( ( s | s ).isSuccess() );
  BOOST_CHECK( ( f | f ).isFailure() );
  BOOST_CHECK( ( r | r ).isRecoverable() );
  BOOST_CHECK( ( s | f ).isSuccess() );
  BOOST_CHECK( ( f | s ).isSuccess() );
  BOOST_CHECK( ( s | r ).isSuccess() );
  BOOST_CHECK( ( r | s ).isSuccess() );
  BOOST_CHECK( ( f | r ).isRecoverable() );
  BOOST_CHECK( ( r | f ).isRecoverable() );

  StatusCode sc;
  BOOST_CHECK( ( ( sc = s ) |= f ).isSuccess() );
}

BOOST_AUTO_TEST_CASE( boolean_logic )
{
  StatusCode s = StatusCode::SUCCESS;
  StatusCode f = StatusCode::FAILURE;
  bool       b = false;
  b &= s;
  BOOST_CHECK( b == false );
  b |= s;
  BOOST_CHECK( b == true );
  b &= f;
  BOOST_CHECK( b == false );
}

BOOST_AUTO_TEST_CASE( short_circuiting )
{
  struct F {
    StatusCode run( const StatusCode& sc )
    {
      done = true;
      return sc;
    }
    bool done{false};
  };

  // Check AND short-circuiting
  {
    F a, b;
    if ( a.run( StatusCode::FAILURE ) && b.run( StatusCode::SUCCESS ) ) {
    }
    BOOST_CHECK( a.done );
    BOOST_CHECK( b.done == false );
  }
  {
    F a, b;
    if ( a.run( StatusCode::SUCCESS ) && b.run( StatusCode::SUCCESS ) ) {
    }
    BOOST_CHECK( a.done );
    BOOST_CHECK( b.done );
  }
  // Check OR short-circuiting
  {
    F a, b;
    if ( a.run( StatusCode::FAILURE ) || b.run( StatusCode::SUCCESS ) ) {
    }
    BOOST_CHECK( a.done );
    BOOST_CHECK( b.done );
  }
  {
    F a, b;
    if ( a.run( StatusCode::SUCCESS ) || b.run( StatusCode::SUCCESS ) ) {
    }
    BOOST_CHECK( a.done );
    BOOST_CHECK( b.done == false );
  }
  // Bitwise operators do not short-circuit
  {
    F a, b;
    if ( a.run( StatusCode::FAILURE ) & b.run( StatusCode::SUCCESS ) ) {
    }
    BOOST_CHECK( a.done );
    BOOST_CHECK( b.done );
  }
  {
    F a, b;
    if ( a.run( StatusCode::SUCCESS ) | b.run( StatusCode::SUCCESS ) ) {
    }
    BOOST_CHECK( a.done );
    BOOST_CHECK( b.done );
  }
}

BOOST_AUTO_TEST_CASE( checking )
{
  {
    StatusCode sc;
    BOOST_CHECK( sc.checked() == false );
    sc.isSuccess();
    BOOST_CHECK( sc.checked() == true );
  }
  {
    StatusCode sc( StatusCode::SUCCESS );
    sc.ignore();
    BOOST_CHECK( sc.checked() == true );
  }
  // Comparison checks
  {
    StatusCode sc( StatusCode::SUCCESS );
    if ( sc == StatusCode::SUCCESS ) {
    }
    BOOST_CHECK( sc.checked() == true );
  }
  {
    StatusCode sc1( StatusCode::SUCCESS );
    StatusCode sc2( StatusCode::FAILURE );
    BOOST_CHECK( sc2 < sc1 );
    BOOST_CHECK( sc1.checked() == true );
    BOOST_CHECK( sc2.checked() == true );
  }
  // Cast to bool --> checked
  {
    StatusCode sc;
    if ( sc ) {
    }
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
  // Function return check
  {
    StatusCode sc = func();
    BOOST_CHECK( sc.checked() == false );
    BOOST_CHECK( func().checked() == false );
  }
  // Logical AND checks both if first is SUCCESS
  {
    StatusCode sc1( StatusCode::SUCCESS );
    StatusCode sc2( StatusCode::FAILURE );
    bool       b = sc1 && sc2;
    BOOST_CHECK( b == false );
    BOOST_CHECK( sc1.checked() == true );
    BOOST_CHECK( sc2.checked() == true );
  }
  // Logical OR checks only first if SUCCESS
  {
    StatusCode sc1( StatusCode::SUCCESS );
    StatusCode sc2( StatusCode::FAILURE );
    bool       b = sc1 || sc2;
    BOOST_CHECK( b == true );
    BOOST_CHECK( sc1.checked() == true );
    BOOST_CHECK( sc2.checked() == false );
  }
  // Ternary AND checks both
  {
    StatusCode sc1( StatusCode::SUCCESS );
    StatusCode sc2( StatusCode::FAILURE );
    StatusCode sc3 = sc1 & sc2;
    BOOST_CHECK( sc1.checked() == true );
    BOOST_CHECK( sc2.checked() == true );
    BOOST_CHECK( sc3.checked() == false );
  }
  // Ternary OR checks both
  {
    StatusCode sc1( StatusCode::SUCCESS );
    StatusCode sc2( StatusCode::FAILURE );
    StatusCode sc3 = sc1 | sc2;
    BOOST_CHECK( sc1.checked() == true );
    BOOST_CHECK( sc2.checked() == true );
    BOOST_CHECK( sc3.checked() == false );
  }
  // AND assignment checks rhs
  {
    StatusCode sc;
    bool       b = false;
    b &= sc;
    BOOST_CHECK( sc.checked() == true );
  }
  // OR assignment checks rhs
  {
    StatusCode sc;
    bool       b = true;
    b |= sc;
    BOOST_CHECK( sc.checked() == true );
  }
  // Ternary AND assignment checks rhs
  {
    StatusCode r, sc;
    r &= sc;
    BOOST_CHECK( sc.checked() == true );
    BOOST_CHECK( r.checked() == false );
  }
  // Ternary OR assignment checks rhs
  {
    StatusCode r, sc;
    r |= sc;
    BOOST_CHECK( sc.checked() == true );
    BOOST_CHECK( r.checked() == false );
  }
}

BOOST_AUTO_TEST_CASE( user_values )
{
  {
    enum Status { ERR1 = 10, ERR2 = 12 };

    StatusCode sc( Status::ERR1 ); // need explicit conversion as enum is not registered
    BOOST_CHECK( sc.isSuccess() == false );
    BOOST_CHECK( sc.isFailure() == true );
    BOOST_CHECK( sc.isRecoverable() == false );
    BOOST_CHECK( static_cast<bool>( sc ) == false );

    sc = StatusCode( Status::ERR2 );
    BOOST_CHECK( sc.getCode() == Status::ERR2 );
  }

  {
    StatusCode sc;
    sc = OtherErrors::ERROR_1; // implicit conversion is OK here
    BOOST_CHECK( sc.isSuccess() == false );
    BOOST_CHECK( sc.isFailure() == true );
    BOOST_CHECK( sc.isRecoverable() == false );
    BOOST_CHECK( static_cast<bool>( sc ) == false );

    sc = OtherErrors::ERROR_2;
    BOOST_CHECK( sc.getCode() == OtherErrors::ERROR_2 );
  }
}

BOOST_AUTO_TEST_CASE( user_categories )
{
  {
    StatusCode sc( MyErr::FAILURE );
    BOOST_CHECK( sc.isSuccess() == false );
    BOOST_CHECK( sc.isFailure() == true );
    BOOST_CHECK( sc.isRecoverable() == false );
    BOOST_CHECK( sc == StatusCode::FAILURE );
    BOOST_CHECK( sc.getCategory().name() == std::string( "My" ) );
    BOOST_CHECK( sc.message() == std::string( "FAILURE" ) );
  }
  {
    StatusCode sc( MyErr::SUCCESS );
    BOOST_CHECK( sc.isSuccess() == true );
    BOOST_CHECK( sc.isFailure() == false );
    BOOST_CHECK( sc.isRecoverable() == false );
    BOOST_CHECK( sc == StatusCode::SUCCESS );
    BOOST_CHECK( sc.message() == std::string( "SUCCESS" ) );
  }
  {
    StatusCode sc( MyErr::WARNING );
    BOOST_CHECK( sc.isSuccess() == true );
    BOOST_CHECK( sc.isFailure() == false );
    BOOST_CHECK( sc.isRecoverable() == false );
    BOOST_CHECK( sc.message() == std::string( "WARNING" ) );
  }
  {
    StatusCode sc( MyErr::OTHER_RECOVERABLE );
    BOOST_CHECK( sc.isSuccess() == false );
    BOOST_CHECK( sc.isFailure() == true );
    BOOST_CHECK( sc.isRecoverable() == true );
  }
}

BOOST_AUTO_TEST_CASE( conversions )
{
  StatusCode sc;

  // Default codes
  sc = StatusCode::SUCCESS;
  // User defined enum class
  sc = MyErr::SUCCESS;
  // User defined enum
  sc = OtherErrors::ERROR_1;

  // sc = 42;     // no implicit conversion from int
  // int i = sc;  // no implicit conversion to int
}
