/**
 * @file EventIDBase.cpp
 *
 * @author RD Schaffer <R.D.Schaffer@cern.ch>
 * @author Paolo Calafiura <pcalafiura@lbl.gov>
 * @author Charles Leggett
 *
 */

#include "GaudiKernel/EventIDBase.h"

EventIDBase::EventIDBase(number_type run_number, 
                         event_number_t event_number)
  :
  m_run_number           (run_number),
  m_event_number         (event_number) 
{
  setup();
}

EventIDBase::EventIDBase(number_type run_number, 
                         event_number_t event_number,
                         number_type time_stamp,
                         number_type time_stamp_ns_offset)
  :
  m_run_number           (run_number),
  m_event_number         (event_number),
  m_time_stamp           (time_stamp),
  m_time_stamp_ns_offset (time_stamp_ns_offset)
{
  setup();
}

EventIDBase::EventIDBase(number_type run_number, 
                         event_number_t event_number,
                         number_type time_stamp,
                         number_type time_stamp_ns_offset,
                         number_type lumi_block,
                         number_type bunch_crossing_id)
  :
  m_run_number           (run_number),
  m_event_number         (event_number),
  m_time_stamp           (time_stamp),
  m_time_stamp_ns_offset (time_stamp_ns_offset),
  m_lumiBlock            (lumi_block),
  m_bunch_crossing_id    (bunch_crossing_id) 
{
  setup();
}

void EventIDBase::setup() {
  if ( (bool)m_run_number  && (bool)m_event_number) {
    setRE();
  }

  if ((bool) m_time_stamp) {
    setTS();
    if ( ! (bool) m_time_stamp_ns_offset ) {
      m_time_stamp_ns_offset = 0;
    }
  }

  if ( (bool)m_lumiBlock && (bool)m_event_number) {
    setLE();
  }
}

EventIDBase::~EventIDBase()
{}

bool
EventIDBase::isRunEvent() const {
  return ( m_type & RunEvent );
}

bool
EventIDBase::isTimeStamp() const {
  return ( m_type & TimeStamp );
}

bool
EventIDBase::isLumiEvent() const {
  return ( m_type & LumiEvent );
}

bool
EventIDBase::isValid() const {
  return ( m_type != Invalid );
}
