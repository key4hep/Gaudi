/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "TimelineSvc.h"
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/StatusCode.h>
#include <GaudiKernel/TimelineEvent.h>

#include <algorithm>
#include <fstream>
#include <ranges>

StatusCode TimelineSvc::initialize() {
  StatusCode sc = Service::initialize();
  if ( !sc.isSuccess() ) return sc;

  if ( msgLevel( MSG::DEBUG ) ) debug() << "initialize" << endmsg;

  m_events.clear();
  // the initial allocation of tbb::concurrent_vector determines the size of
  // the first allocated array, useful to reduce fragmentation
  m_events.reserve( 1024 );

  if ( m_partial ) {
    std::ofstream out( m_timelineFile + ".part", std::ofstream::trunc | std::ofstream::out );
    out << "#start end algorithm thread slot event" << std::endl;
    out.close();
  }

  return StatusCode::SUCCESS;
}

StatusCode TimelineSvc::reinitialize() {

  MsgStream log( msgSvc(), name() );
  log << MSG::DEBUG << "reinitialize" << endmsg;

  m_events.clear();

  return StatusCode::SUCCESS;
}

StatusCode TimelineSvc::finalize() {
  if ( m_dumpTimeline && m_events.size() > 0 ) {
    MsgStream log( msgSvc(), name() );

    log << MSG::INFO << "Outputting timeline with " << m_events.size() << " entries to file " << m_timelineFile.value()
        << endmsg;

    outputTimeline();
  }

  return StatusCode::SUCCESS;
}

ITimelineSvc::TimelineRecorder TimelineSvc::getRecorder( std::string alg, const EventContext& ctx ) {
  auto& newTimelineEvent     = m_events.emplace( std::make_tuple( alg, ctx.evt() ), TimelineEvent{} ).first->second;
  newTimelineEvent.algorithm = std::move( alg );
  newTimelineEvent.slot      = ctx.slot();
  newTimelineEvent.event     = ctx.evt();
  TimelineRecorder recorder{ newTimelineEvent };
  if ( m_partial ) {
    std::ofstream out( m_timelineFile + ".part", std::ofstream::app | std::ofstream::out );
    out << std::chrono::duration_cast<std::chrono::nanoseconds>( newTimelineEvent.start.time_since_epoch() ).count()
        << " "
        << std::chrono::duration_cast<std::chrono::nanoseconds>( newTimelineEvent.end.time_since_epoch() ).count()
        << " " << newTimelineEvent.algorithm << " " << newTimelineEvent.thread << " " << newTimelineEvent.slot << " "
        << newTimelineEvent.event << std::endl;

    out.close();
  }
  return recorder;
}

bool TimelineSvc::getTimelineEvent( TimelineEvent& e ) const { return getFirstMatching( e ); }

bool TimelineSvc::getFirstMatching( TimelineEvent& e ) const {
  auto [begin, end] = m_events.equal_range( std::make_tuple( e.algorithm, e.event ) );
  if ( begin == end ) { return false; }
  auto it =
      std::min_element( begin, end, []( const auto& a, const auto& b ) { return a.second.start < b.second.start; } );
  e = it->second;
  return true;
}

bool TimelineSvc::getLastMatching( TimelineEvent& e ) const {
  auto [begin, end] = m_events.equal_range( std::make_tuple( e.algorithm, e.event ) );
  if ( begin == end ) { return false; }
  auto it =
      std::max_element( begin, end, []( const auto& a, const auto& b ) { return a.second.start < b.second.start; } );
  e = it->second;
  return true;
}

void TimelineSvc::outputTimeline() {
  std::ofstream out( m_timelineFile, std::ofstream::out | std::ofstream::trunc );

  out << "#start end algorithm thread slot event" << std::endl;

  auto flat_events = std::vector<TimelineEvent>();
  flat_events.reserve( m_events.size() );

  std::ranges::copy( m_events | std::views::values, std::back_inserter( flat_events ) );

  std::sort( flat_events.begin(), flat_events.end(), []( const auto& a, const auto& b ) { return a.start < b.start; } );
  for ( const auto& e : flat_events ) {
    out << std::chrono::duration_cast<std::chrono::nanoseconds>( e.start.time_since_epoch() ).count() << " "
        << std::chrono::duration_cast<std::chrono::nanoseconds>( e.end.time_since_epoch() ).count() << " "
        << e.algorithm << " " << e.thread << " " << e.slot << " " << e.event << std::endl;
  }

  out.close();
}

DECLARE_COMPONENT( TimelineSvc )
