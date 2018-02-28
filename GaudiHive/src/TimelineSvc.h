
#ifndef GAUDIHIVE_TIMELINESVC_H
#define GAUDIHIVE_TIMELINESVC_H

#include "GaudiKernel/ITimelineSvc.h"
#include "GaudiKernel/Service.h"

#include <string>

#include "tbb/concurrent_vector.h"

class TimelineSvc : public extends<Service, ITimelineSvc>
{
public:
  using extends::extends;

  StatusCode initialize() override;
  StatusCode reinitialize() override;
  StatusCode finalize() override;

  void registerTimelineEvent( const TimelineEvent& ) override;
  TimelineRecorder getRecorder( std::string alg, const EventContext& ctx ) override;
  bool getTimelineEvent( TimelineEvent& ) const override;

  bool isEnabled() const override { return m_isEnabled; }

private:
  void outputTimeline();

  Gaudi::Property<std::string> m_timelineFile{this, "TimelineFile", "timeline.csv", ""};
  Gaudi::Property<bool> m_isEnabled{this, "RecordTimeline", false, "Enable recording of the timeline events"};
  Gaudi::Property<bool> m_dumpTimeline{this, "DumpTimeline", false, "Enable dumping of the timeline events"};
  Gaudi::Property<bool> m_partial{this, "Partial", false, ""};

  tbb::concurrent_vector<TimelineEvent> m_events;
};

#endif
