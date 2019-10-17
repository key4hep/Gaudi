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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_StatusCode
#include <boost/test/unit_test.hpp>

#include "GaudiKernel/GaudiException.h"
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

namespace {
  struct MyCategory : StatusCode::Category {
    const char* name() const override { return "My"; }

    bool isSuccess( StatusCode::code_t code ) const override {
      const MyErr e = static_cast<MyErr>( code );
      return e == MyErr::SUCCESS || ( e > MyErr::RECOVERABLE && e < MyErr::OTHER_RECOVERABLE );
    }

    bool isRecoverable( StatusCode::code_t code ) const override {
      return static_cast<MyErr>( code ) == MyErr::RECOVERABLE || static_cast<MyErr>( code ) == MyErr::OTHER_RECOVERABLE;
    }

    std::string message( StatusCode::code_t code ) const override {
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
} // namespace

STATUSCODE_ENUM_IMPL( MyErr, MyCategory )

// Test old-style enums
enum OtherErrors { ERROR_1 = 100, ERROR_2 = 200 };

STATUSCODE_ENUM_DECL( OtherErrors )
STATUSCODE_ENUM_IMPL( OtherErrors )

// A test function
StatusCode func() { return StatusCode::SUCCESS; }

BOOST_AUTO_TEST_CASE( values ) {
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

BOOST_AUTO_TEST_CASE( comparison ) {
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

BOOST_AUTO_TEST_CASE( ternary_and ) {
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

BOOST_AUTO_TEST_CASE( ternary_or ) {
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

BOOST_AUTO_TEST_CASE( boolean_logic ) {
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

BOOST_AUTO_TEST_CASE( short_circuiting ) {
  struct F {
    StatusCode run( const StatusCode& sc ) {
      done = true;
      return sc;
    }
    bool done{false};
  };

  // Check AND short-circuiting
  {
    F a, b;
    if ( a.run( StatusCode::FAILURE ) && b.run( StatusCode::SUCCESS ) ) {}
    BOOST_CHECK( a.done );
    BOOST_CHECK( b.done == false );
  }
  {
    F a, b;
    if ( a.run( StatusCode::SUCCESS ) && b.run( StatusCode::SUCCESS ) ) {}
    BOOST_CHECK( a.done );
    BOOST_CHECK( b.done );
  }
  // Check OR short-circuiting
  {
    F a, b;
    if ( a.run( StatusCode::FAILURE ) || b.run( StatusCode::SUCCESS ) ) {}
    BOOST_CHECK( a.done );
    BOOST_CHECK( b.done );
  }
  {
    F a, b;
    if ( a.run( StatusCode::SUCCESS ) || b.run( StatusCode::SUCCESS ) ) {}
    BOOST_CHECK( a.done );
    BOOST_CHECK( b.done == false );
  }
  // Bitwise operators do not short-circuit
  {
    F a, b;
    if ( a.run( StatusCode::FAILURE ) & b.run( StatusCode::SUCCESS ) ) {}
    BOOST_CHECK( a.done );
    BOOST_CHECK( b.done );
  }
  {
    F a, b;
    if ( a.run( StatusCode::SUCCESS ) | b.run( StatusCode::SUCCESS ) ) {}
    BOOST_CHECK( a.done );
    BOOST_CHECK( b.done );
  }
}

BOOST_AUTO_TEST_CASE( user_values ) {
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

BOOST_AUTO_TEST_CASE( user_categories ) {
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

BOOST_AUTO_TEST_CASE( conversions ) {
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

/// Function used for testing that the constants in @c StatusCode can be found
/// by constant references.
bool __attribute__( ( noinline ) )
testReferencePassing( const StatusCode::ErrorCode& value, const StatusCode::ErrorCode& reference ) {
  return ( value == reference );
}

BOOST_AUTO_TEST_CASE( const_definitions ) {
  // Test whether the test can compile/link correctly when passing the
  // constants in @c StatusCode into a function by constant reference.
  BOOST_CHECK( testReferencePassing( StatusCode::SUCCESS, StatusCode::SUCCESS ) );
  BOOST_CHECK( testReferencePassing( StatusCode::FAILURE, StatusCode::FAILURE ) );
  BOOST_CHECK( testReferencePassing( StatusCode::RECOVERABLE, StatusCode::RECOVERABLE ) );
}

BOOST_AUTO_TEST_CASE( chaining ) {
  int  steps = 0; // counter to see when the chaining stops
  auto start = [&steps]( StatusCode init = StatusCode::SUCCESS ) -> StatusCode {
    steps = 1;
    return init;
  };
  auto success_func = [&steps]() -> StatusCode {
    ++steps;
    return StatusCode::SUCCESS;
  };
  auto failure_func = [&steps]() -> StatusCode {
    ++steps;
    return StatusCode::FAILURE;
  };
  auto no_return = [&steps]() { ++steps; };

  StatusCode sc = StatusCode::SUCCESS;

  // --- andThen ---

  // simple chain
  sc = start().andThen( success_func );
  BOOST_CHECK( sc.isSuccess() );
  BOOST_CHECK_EQUAL( steps, 2 );

  // longer chain
  sc = start().andThen( success_func ).andThen( success_func );
  BOOST_CHECK( sc.isSuccess() );
  BOOST_CHECK_EQUAL( steps, 3 );

  // chain functions not returning StatusCode
  sc = start().andThen( no_return );
  BOOST_CHECK( sc.isSuccess() );
  BOOST_CHECK_EQUAL( steps, 2 );

  // chain functions not returning StatusCode (longer)
  sc = start().andThen( no_return ).andThen( success_func );
  BOOST_CHECK( sc.isSuccess() );
  BOOST_CHECK_EQUAL( steps, 3 );

  // category pass through
  sc = StatusCode{MyErr::SUCCESS}.andThen( []() {
    // no-op
  } );
  BOOST_CHECK( sc.isSuccess() );
  BOOST_CHECK_EQUAL( sc.getCategory().name(), "My" );

  // chain with failure
  sc = start().andThen( failure_func );
  BOOST_CHECK( sc.isFailure() );
  BOOST_CHECK_EQUAL( steps, 2 );

  // longer chain with failure
  sc = start().andThen( success_func ).andThen( failure_func );
  BOOST_CHECK( sc.isFailure() );
  BOOST_CHECK_EQUAL( steps, 3 );

  // early exit after failure
  sc = start().andThen( failure_func ).andThen( success_func );
  BOOST_CHECK( sc.isFailure() );
  BOOST_CHECK_EQUAL( steps, 2 );

  // custom category failure
  sc = start().andThen( []() { return MyErr::FAILURE; } );
  BOOST_CHECK( sc.isFailure() );
  BOOST_CHECK_EQUAL( sc.getCategory().name(), "My" );

  // --- orElse ---

  // simple chain
  sc = start( StatusCode::FAILURE ).orElse( no_return );
  BOOST_CHECK( sc.isFailure() );
  BOOST_CHECK_EQUAL( steps, 2 );
  sc = start( StatusCode::SUCCESS ).orElse( no_return );
  BOOST_CHECK( sc.isSuccess() );
  BOOST_CHECK_EQUAL( steps, 1 );

  // simple override
  sc = start( StatusCode::FAILURE ).orElse( success_func );
  BOOST_CHECK( sc.isSuccess() );
  BOOST_CHECK_EQUAL( steps, 2 );

  // longer chain
  sc = start( StatusCode::FAILURE ).orElse( no_return ).andThen( success_func );
  BOOST_CHECK( sc.isFailure() );
  BOOST_CHECK_EQUAL( steps, 2 );

  sc = start( StatusCode::FAILURE ).orElse( success_func ).andThen( success_func );
  BOOST_CHECK( sc.isSuccess() );
  BOOST_CHECK_EQUAL( steps, 3 );

  // category pass through
  sc = start( MyErr::FAILURE ).orElse( no_return );
  BOOST_CHECK( sc.isFailure() );
  BOOST_CHECK_EQUAL( sc.getCategory().name(), "My" );

  // custom category failure
  sc = start( StatusCode::FAILURE ).orElse( [] { return MyErr::FAILURE; } ).andThen( failure_func );
  BOOST_CHECK( sc.isFailure() );
  BOOST_CHECK_EQUAL( sc.getCategory().name(), "My" );

  // --- orThrow ---

  // no throw
  sc = start().orThrow( "no error", "test_StatusCode" );
  BOOST_CHECK( sc.isSuccess() );
  BOOST_CHECK_EQUAL( steps, 1 );

  // this is just to check that there's no warning on ignoring the
  // return value of orThrow
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wunused-result"
  StatusCode{StatusCode::SUCCESS}.orThrow( "no error", "test_StatusCode" );
#pragma GCC diagnostic pop

  // throw
  try {
    sc = start().andThen( failure_func ).orThrow( "failure", "test_StatusCode" );
    BOOST_FAIL( "exception expected" );
  } catch ( GaudiException& exc ) {
    BOOST_CHECK( exc.code().isFailure() );
    BOOST_CHECK_EQUAL( exc.message(), "failure" );
    BOOST_CHECK_EQUAL( exc.tag(), "test_StatusCode" );
    BOOST_CHECK_EQUAL( steps, 2 );
  } catch ( ... ) { BOOST_FAIL( "wrong exception" ); }

  // throw (abort chain)
  try {
    sc = start().andThen( failure_func ).orThrow( "failure", "test_StatusCode" ).andThen( success_func );
    BOOST_FAIL( "exception expected" );
  } catch ( GaudiException& exc ) {
    BOOST_CHECK( exc.code().isFailure() );
    BOOST_CHECK_EQUAL( exc.message(), "failure" );
    BOOST_CHECK_EQUAL( exc.tag(), "test_StatusCode" );
    BOOST_CHECK_EQUAL( steps, 2 );
  } catch ( ... ) { BOOST_FAIL( "wrong exception" ); }

  // throw custom category
  try {
    sc = start().andThen( []() { return MyErr::FAILURE; } ).orThrow( "failure", "test_StatusCode" );
    BOOST_FAIL( "exception expected" );
  } catch ( GaudiException& exc ) {
    BOOST_CHECK( exc.code().isFailure() );
    BOOST_CHECK_EQUAL( exc.code().getCategory().name(), "My" );
    BOOST_CHECK_EQUAL( exc.message(), "failure" );
    BOOST_CHECK_EQUAL( exc.tag(), "test_StatusCode" );
    BOOST_CHECK_EQUAL( steps, 1 );
  } catch ( ... ) { BOOST_FAIL( "wrong exception" ); }
}
