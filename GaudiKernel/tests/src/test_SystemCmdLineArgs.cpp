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

// Set up a Boost unit test:
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_SystemCmdLineArgs
#include <boost/test/unit_test.hpp>

// GaudiKernel include(s):
#include "GaudiKernel/System.h"

/// Macro stringifying its argument
#define STRINGIFY( s ) #s
/// Macro stringifying the value of the macro given to it
#define XSTRINGIFY( s ) STRINGIFY( s )

/// Test that we can get the process's name from the command line arguments
BOOST_AUTO_TEST_CASE( exeNameFromCmdLineArgs ) {
  BOOST_CHECK( System::numCmdLineArgs() == 1 );
  BOOST_CHECK( System::cmdLineArgs()[0].find( XSTRINGIFY( BOOST_TEST_MODULE ) ) != std::string::npos );
}
