#include "TimelineSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"

#include <fstream>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode TimelineSvc::initialize()
{

  StatusCode sc = Service::initialize();
  if ( !sc.isSuccess() ) return sc;

  if ( msgLevel( MSG::DEBUG ) ) debug() << "initialize" << endmsg;

  m_events.clear();

  if ( m_partial ) {
    std::ofstream out( m_timelineFile + ".part", std::ofstream::trunc | std::ofstream::out );
    out << "#start end algorithm thread slot event" << std::endl;
    out.close();
  }

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode TimelineSvc::reinitialize()
{

  MsgStream log( msgSvc(), name() );
  log << MSG::DEBUG << "reinitialize" << endmsg;

  m_events.clear();

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode TimelineSvc::finalize()
{

  if ( m_dumpTimeline && m_events.size() > 0 ) {
    MsgStream log( msgSvc(), name() );

    log << MSG::INFO << "Outputting timeline with " << m_events.size() << " entries to file " << m_timelineFile
        << endmsg;

    outputTimeline();
  }

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void TimelineSvc::registerTimelineEvent( const TimelineEvent& e )
{

  m_events.push_back( e );

  if ( m_partial ) {
    std::ofstream out( m_timelineFile + ".part", std::ofstream::app | std::ofstream::out );
    out << std::chrono::duration_cast<std::chrono::nanoseconds>( e.start.time_since_epoch() ).count() << " "
        << std::chrono::duration_cast<std::chrono::nanoseconds>( e.end.time_since_epoch() ).count() << " "
        << e.algorithm << " " << e.thread << " " << e.slot << " " << e.event << std::endl;

    out.close();
  }
}

void TimelineSvc::getTimelineEvent( TimelineEvent& e ) const
{

  for ( const auto& candidate : m_events ) {
    if ( candidate.algorithm == e.algorithm && candidate.event == e.event && candidate.slot == e.slot ) {
      e = candidate;
      break;
    }
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void TimelineSvc::outputTimeline()
{

  std::ofstream out( m_timelineFile, std::ofstream::out | std::ofstream::trunc );

  out << "#start end algorithm thread slot event" << std::endl;

  for ( const auto& e : m_events ) {
    out << std::chrono::duration_cast<std::chrono::nanoseconds>( e.start.time_since_epoch() ).count() << " "
        << std::chrono::duration_cast<std::chrono::nanoseconds>( e.end.time_since_epoch() ).count() << " "
        << e.algorithm << " " << e.thread << " " << e.slot << " " << e.event << std::endl;
  }

  out.close();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

DECLARE_COMPONENT( TimelineSvc )
