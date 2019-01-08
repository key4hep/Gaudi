#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_EventIDRange
#include <boost/test/unit_test.hpp>

#include "GaudiKernel/EventIDRange.h"


BOOST_AUTO_TEST_CASE( comparison )
{
  {
    //Constructor:  
    //number_type run_number, event_number_t event_number, number_type time_stamp = UNDEFNUM,
    //           number_type time_stamp_ns_offset = 0, number_type lumi_block = UNDEFNUM,
    //           number_type bunch_crossing_id = 0 );

    EventIDBase ei_RunLB1{100, 0, EventIDBase::UNDEFNUM,0, 20};
    EventIDBase ei_RunLB2{200, 0, EventIDBase::UNDEFNUM,0, 10};
    EventIDRange rangeRLB1(ei_RunLB1,ei_RunLB2);

    EventIDBase ei_RunLB3{100, 0, EventIDBase::UNDEFNUM,0, 30};
    EventIDBase ei_RunLB4{150, 0, EventIDBase::UNDEFNUM,0, 50};
    EventIDRange rangeRLB2(ei_RunLB3,ei_RunLB4);

    BOOST_CHECK(!rangeRLB1.start().isTimeStamp() && rangeRLB1.start().isRunLumi());
    BOOST_CHECK(!rangeRLB2.start().isTimeStamp() && rangeRLB2.start().isRunLumi());


    EventIDBase ei_TS1{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 1000, 530, EventIDBase::UNDEFNUM}; 
    EventIDBase ei_TS2{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 1500, 550, EventIDBase::UNDEFNUM}; 
    EventIDRange rangeTS1(ei_TS1,ei_TS2);

    EventIDBase ei_TS3{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 1200, 500, EventIDBase::UNDEFNUM}; 
    EventIDBase ei_TS4{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 1800, 800, EventIDBase::UNDEFNUM}; 
    EventIDRange rangeTS2(ei_TS3,ei_TS4);
    
    BOOST_CHECK(rangeTS1.start().isTimeStamp() && !rangeTS1.start().isRunLumi());
    BOOST_CHECK(rangeTS2.start().isTimeStamp() && !rangeTS2.start().isRunLumi());


    //Intersect 2 Run-LB ranges:
    EventIDRange intersect_RLB=EventIDRange::intersect(rangeRLB1,rangeRLB2);

    std::cout << "intersect_RLB" << intersect_RLB << std::endl;

    //Check resulting range
    BOOST_CHECK(intersect_RLB.start().isRunLumi());
    BOOST_CHECK(!intersect_RLB.start().isTimeStamp());

    BOOST_CHECK(intersect_RLB.start().run_number() == 100);
    BOOST_CHECK(intersect_RLB.start().lumi_block()== 30);


    BOOST_CHECK(intersect_RLB.stop().run_number() == 150);
    BOOST_CHECK(intersect_RLB.stop().lumi_block() == 50);
    
    //Intersect 2 Time-stamp ranges:
    EventIDRange intersect_TS=EventIDRange::intersect(rangeTS1,rangeTS2);

    std::cout << "intersect TS:"<< intersect_TS << std::endl;

    //Check resulting range
    BOOST_CHECK(!intersect_TS.start().isRunLumi());
    BOOST_CHECK(intersect_TS.start().isTimeStamp());

    BOOST_CHECK(intersect_TS.start().time_stamp()==1200);
    BOOST_CHECK(intersect_TS.start().time_stamp_ns_offset()==500);

    BOOST_CHECK(intersect_TS.stop().time_stamp()==1500);
    BOOST_CHECK(intersect_TS.stop().time_stamp_ns_offset()==550);

    
    //Intersect a Time-Stamp range and a Run-Lumi range

    EventIDRange intersect_RLBTS1=EventIDRange::intersect(rangeRLB1, rangeTS1);

    std::cout << "intersect RLBTS1:"<< intersect_RLBTS1  << std::endl;
    
    BOOST_CHECK(intersect_RLBTS1.start().isRunLumi());
    BOOST_CHECK(intersect_RLBTS1.start().isTimeStamp());


    BOOST_CHECK(intersect_RLBTS1.start().time_stamp()==1000);
    BOOST_CHECK(intersect_RLBTS1.start().time_stamp_ns_offset()==530);
    BOOST_CHECK(intersect_RLBTS1.stop().time_stamp()==1500);
    BOOST_CHECK(intersect_RLBTS1.stop().time_stamp_ns_offset()==550);


    BOOST_CHECK(intersect_RLBTS1.start().run_number() == 100);
    BOOST_CHECK(intersect_RLBTS1.start().lumi_block()== 20);
    BOOST_CHECK(intersect_RLBTS1.stop().run_number() == 200);
    BOOST_CHECK(intersect_RLBTS1.stop().lumi_block() == 10);


    //Intersect another Time-Stamp range and a Run-Lumi range

    EventIDRange intersect_RLBTS2=EventIDRange::intersect(rangeRLB2, rangeTS2);
    std::cout << "intersect RLBTS2:"<< intersect_RLBTS2  << std::endl;
    
    BOOST_CHECK(intersect_RLBTS2.start().isRunLumi());
    BOOST_CHECK(intersect_RLBTS2.start().isTimeStamp());


    BOOST_CHECK(intersect_RLBTS2.start().time_stamp()==1200);
    BOOST_CHECK(intersect_RLBTS2.start().time_stamp_ns_offset()==500);
    BOOST_CHECK(intersect_RLBTS2.stop().time_stamp()==1800);
    BOOST_CHECK(intersect_RLBTS2.stop().time_stamp_ns_offset()==800);


    BOOST_CHECK(intersect_RLBTS2.start().run_number() == 100);
    BOOST_CHECK(intersect_RLBTS2.start().lumi_block()== 30);
    BOOST_CHECK(intersect_RLBTS2.stop().run_number() == 150);
    BOOST_CHECK(intersect_RLBTS2.stop().lumi_block() == 50);


    //Intersect two mixed ranges:
    EventIDRange intersect_RLBTS3=EventIDRange::intersect(intersect_RLBTS1,intersect_RLBTS2);
    std::cout << "intersect RLBTS3:"<< intersect_RLBTS3  << std::endl;

    BOOST_CHECK(intersect_RLBTS3.start().isRunLumi());
    BOOST_CHECK(intersect_RLBTS3.start().isTimeStamp());


    BOOST_CHECK(intersect_RLBTS3.start().time_stamp()==1200);
    BOOST_CHECK(intersect_RLBTS3.start().time_stamp_ns_offset()==500);
    BOOST_CHECK(intersect_RLBTS3.stop().time_stamp()==1500);
    BOOST_CHECK(intersect_RLBTS3.stop().time_stamp_ns_offset()==550);


    BOOST_CHECK(intersect_RLBTS3.start().run_number() == 100);
    BOOST_CHECK(intersect_RLBTS3.start().lumi_block()== 30);
    BOOST_CHECK(intersect_RLBTS3.stop().run_number() == 150);
    BOOST_CHECK(intersect_RLBTS3.stop().lumi_block() == 50);


    //Communtativity:
    EventIDRange intersect_RLBTS3a=EventIDRange::intersect(intersect_RLBTS2,intersect_RLBTS1);
    BOOST_CHECK(intersect_RLBTS3==intersect_RLBTS3a);
  }
}
