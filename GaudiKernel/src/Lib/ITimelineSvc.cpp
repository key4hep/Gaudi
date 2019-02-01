#include "GaudiKernel/ITimelineSvc.h"
#include "GaudiKernel/EventContext.h"

ITimelineSvc::TimelineRecorder::TimelineRecorder( TimelineEvent& record, std::string alg, const EventContext& ctx )
    : m_record{&record} {
  m_record->thread    = pthread_self();
  m_record->slot      = ctx.slot();
  m_record->event     = ctx.evt();
  m_record->algorithm = std::move( alg );
  m_record->start     = Clock::now();
}
