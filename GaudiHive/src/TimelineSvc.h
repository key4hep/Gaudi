
#ifndef GAUDIHIVE_TIMELINESVC_H
#define GAUDIHIVE_TIMELINESVC_H

#include "GaudiKernel/Service.h"
#include "GaudiKernel/ITimelineSvc.h"

#include <string>

#include "tbb/concurrent_vector.h"

class TimelineSvc: public extends<Service,
                                  ITimelineSvc> {

public:

  StatusCode initialize() override;
  StatusCode reinitialize() override;
  StatusCode finalize() override;

  void registerTimelineEvent(const TimelineEvent & e) override;

  bool isEnabled() const { return m_isEnabled; }

  using extends::extends;

  // Destructor.
  ~TimelineSvc() override = default;

private:

  void outputTimeline();

  StringProperty m_timelineFile{this, "TimelineFile",  "timeline.csv", ""};
  BooleanProperty m_isEnabled{this, "RecordTimeline",  false, ""};
  BooleanProperty m_partial{this, "Partial",  false, ""};

  tbb::concurrent_vector<TimelineEvent> m_events;

};

#endif
