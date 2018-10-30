/**
 * @file EventIDBase.cpp
 *
 * @author RD Schaffer <R.D.Schaffer@cern.ch>
 * @author Paolo Calafiura <pcalafiura@lbl.gov>
 * @author Charles Leggett
 *
 */

#include "GaudiKernel/EventIDBase.h"

#include <limits>

const EventIDBase::number_type    EventIDBase::UNDEFNUM = std::numeric_limits<EventIDBase::number_type>::max();
const EventIDBase::event_number_t EventIDBase::UNDEFEVT = std::numeric_limits<EventIDBase::event_number_t>::max();

EventIDBase::EventIDBase( number_type run_number, event_number_t event_number, number_type time_stamp,
                          number_type time_stamp_ns_offset, number_type lumi_block, number_type bunch_crossing_id )
    : m_run_number( run_number )
    , m_event_number( event_number )
    , m_time_stamp( time_stamp )
    , m_time_stamp_ns_offset( time_stamp_ns_offset )
    , m_lumi_block( lumi_block )
    , m_bunch_crossing_id( bunch_crossing_id )
{
  if ( m_run_number != UNDEFNUM && m_event_number != UNDEFEVT ) setRE();

  if ( m_time_stamp != UNDEFNUM ) {
    setTS();
    if ( m_time_stamp_ns_offset == UNDEFNUM ) m_time_stamp_ns_offset = 0;
  }

  if ( m_lumi_block != UNDEFNUM && m_event_number != UNDEFEVT ) setLE();

  if ( m_run_number != UNDEFNUM && m_lumi_block != UNDEFNUM ) setRL();
}


EventIDBase::EventIDBase (std::tuple<number_type,event_number_t,number_type> run_lumi_ev,
			   std::tuple<number_type,number_type> time_stamp,number_type bunch_crossing_id) : 
  EventIDBase(std::get<0>(run_lumi_ev),//run number
	       std::get<1>(run_lumi_ev),//event number
	       std::get<0>(time_stamp),//timestamp in seconds
	       std::get<1>(time_stamp),//nanoseconds offst
	       std::get<2>(run_lumi_ev),//lumi block
	       bunch_crossing_id) {}

