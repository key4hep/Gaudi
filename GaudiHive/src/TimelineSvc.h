
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

  bool isEnabled() const override { return m_isEnabled; }

  TimelineSvc( const std::string& name, ISvcLocator* svc );

  // Destructor.
  ~TimelineSvc() override;

private:

  void outputTimeline();

  bool m_isEnabled;
  bool m_partial;
  std::string m_timelineFile;
  tbb::concurrent_vector<TimelineEvent> m_events;

};

#endif
