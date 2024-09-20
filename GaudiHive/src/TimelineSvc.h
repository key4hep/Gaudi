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

#ifndef GAUDIHIVE_TIMELINESVC_H
#define GAUDIHIVE_TIMELINESVC_H

#include <GaudiKernel/ITimelineSvc.h>
#include <GaudiKernel/Service.h>

#include <string>

#include <tbb/concurrent_vector.h>

class TimelineSvc : public extends<Service, ITimelineSvc> {
public:
  using extends::extends;

  StatusCode initialize() override;
  StatusCode reinitialize() override;
  StatusCode finalize() override;

  TimelineRecorder getRecorder( std::string alg, const EventContext& ctx ) override;
  bool             getTimelineEvent( TimelineEvent& ) const override;

  bool isEnabled() const override { return m_isEnabled; }

private:
  void outputTimeline();

  Gaudi::Property<std::string> m_timelineFile{ this, "TimelineFile", "timeline.csv", "" };
  Gaudi::Property<bool>        m_isEnabled{ this, "RecordTimeline", false, "Enable recording of the timeline events" };
  Gaudi::Property<bool>        m_dumpTimeline{ this, "DumpTimeline", false, "Enable dumping of the timeline events" };
  Gaudi::Property<bool>        m_partial{ this, "Partial", false, "" };

  tbb::concurrent_vector<TimelineEvent> m_events;
};

#endif
