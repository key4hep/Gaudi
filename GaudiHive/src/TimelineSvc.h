
#ifndef GAUDIHIVE_TIMELINESVC_H
#define GAUDIHIVE_TIMELINESVC_H

#include "GaudiKernel/Service.h"
#include "GaudiKernel/ITimelineSvc.h"

#include <string>

#include "tbb/concurrent_vector.h"

class TimelineSvc: public extends1<Service, ITimelineSvc> {

public:

  virtual StatusCode initialize();
  virtual StatusCode reinitialize();
  virtual StatusCode finalize();

  virtual void registerTimelineEvent(const TimelineEvent & e);

  TimelineSvc( const std::string& name, ISvcLocator* svc );

  // Destructor.
  virtual ~TimelineSvc();

private:

  void outputTimeline();

  std::string m_timelineFile;
  tbb::concurrent_vector<TimelineEvent> m_events;

};

#endif
