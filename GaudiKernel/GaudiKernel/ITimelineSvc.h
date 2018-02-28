#ifndef GAUDIKERNEL_ITIMELINESVC_H
#define GAUDIKERNEL_ITIMELINESVC_H

#include "GaudiKernel/IService.h"

#include <pthread.h>
#include <string>

#include <chrono>

class EventContext;

struct TimelineEvent final {
  using Clock      = std::chrono::high_resolution_clock;
  using time_point = Clock::time_point;

  pthread_t thread;
  size_t slot;
  size_t event;

  std::string algorithm;

  time_point start;
  time_point end;
};

class GAUDI_API ITimelineSvc : virtual public IService
{

public:
  /// InterfaceID
  DeclareInterfaceID( ITimelineSvc, 2, 0 );

  /// RAII helper to record timeline events
  class TimelineRecorder final
  {
  public:
    using Clock = TimelineEvent::Clock;

    TimelineRecorder() = default;
    TimelineRecorder( TimelineEvent& record, std::string alg, const EventContext& ctx );

    TimelineRecorder( const TimelineRecorder& ) = delete;
    TimelineRecorder( TimelineRecorder&& other ) : m_record{other.m_record} { other.m_record = nullptr; }

    TimelineRecorder& operator=( TimelineRecorder&& other )
    {
      std::swap( m_record, other.m_record );
      return *this;
    }

    ~TimelineRecorder()
    {
      if ( m_record ) m_record->end = Clock::now();
    }

  private:
    TimelineEvent* m_record = nullptr;
  };

  virtual void registerTimelineEvent( const TimelineEvent& ) = 0;
  virtual TimelineRecorder getRecorder( std::string alg, const EventContext& ctx ) = 0;
  // Augment a partially pre-filled TimelineEvent object with matching info
  virtual bool getTimelineEvent( TimelineEvent& ) const = 0;
  virtual bool isEnabled() const                        = 0;
};

#endif
