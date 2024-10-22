/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_EventIDBase
#include <boost/test/unit_test.hpp>

#include <GaudiKernel/EventIDBase.h>
#include <algorithm>
#include <array>

BOOST_AUTO_TEST_CASE( comparison ) {
  // comparison and sorting
  {
    EventIDBase ei1{ 100, 1 };
    EventIDBase ei2{ 100, 2 };
    EventIDBase ei3{ 101, 1 };

    BOOST_CHECK( ei1.isRunEvent() );
    BOOST_CHECK( ei1.isValid() );

    BOOST_CHECK( ei1.run_number() == 100 );
    BOOST_CHECK( ei1.event_number() == 1 );
    BOOST_CHECK( ei1.time_stamp() == EventIDBase::UNDEFNUM );

    BOOST_CHECK( ei1 == ei1 );
    BOOST_CHECK( ei1 != ei2 );
    BOOST_CHECK( ei2 != ei3 );
    BOOST_CHECK( ei1 != ei3 );

    BOOST_CHECK( ei1 <= ei1 );
    BOOST_CHECK( ei1 <= ei2 );
    BOOST_CHECK( ei2 <= ei3 );
    BOOST_CHECK( ei1 <= ei3 );

    BOOST_CHECK( ei1 < ei2 );
    BOOST_CHECK( ei2 < ei3 );
    BOOST_CHECK( ei1 < ei3 );

    BOOST_CHECK( ei1 >= ei1 );
    BOOST_CHECK( ei2 >= ei1 );
    BOOST_CHECK( ei3 >= ei2 );
    BOOST_CHECK( ei3 >= ei1 );

    BOOST_CHECK( ei2 > ei1 );
    BOOST_CHECK( ei3 > ei2 );
    BOOST_CHECK( ei3 > ei1 );

    EventIDBase ei1a = ei1;
    BOOST_CHECK( ei1a == ei1 );
    ei1a = ei2;
    BOOST_CHECK( ei1a == ei2 );

    std::array<EventIDBase, 3> a = { ei2, ei1, ei3 };
    std::sort( begin( a ), end( a ), EventIDBase::SortByRunEvent() );
    BOOST_CHECK( a[0] == ei3 );
    BOOST_CHECK( a[1] == ei2 );
    BOOST_CHECK( a[2] == ei1 );

    std::array<EventIDBase*, 3> ap = { &ei1, &ei3, &ei2 };
    std::sort( begin( ap ), end( ap ), EventIDBase::SortByRunEvent() );

    BOOST_CHECK( ap[0] == &ei3 );
    BOOST_CHECK( ap[1] == &ei2 );
    BOOST_CHECK( ap[2] == &ei1 );
  }
}
