/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
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
