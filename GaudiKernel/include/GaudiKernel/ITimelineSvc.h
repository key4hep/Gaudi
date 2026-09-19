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
#pragma once

#include <GaudiKernel/IService.h>

#include <string>

class EventContext;
struct TimelineEvent;

class GAUDI_API ITimelineSvc : virtual public IService {

public:
  /// InterfaceID
  DeclareInterfaceID( ITimelineSvc, 2, 0 );

  /// RAII helper to record timeline events
  class TimelineRecorder final {
  public:
    TimelineRecorder() = default;
    // Construct a TimelineRecorder with an empty record object, algorithm name, and event context
    TimelineRecorder( TimelineEvent& record, std::string alg, const EventContext& ctx );
    // Construct a TimelineRecorder with a record object with pre-filled algorithm name and event context
    TimelineRecorder( TimelineEvent& record );

    TimelineRecorder( const TimelineRecorder& )            = delete;
    TimelineRecorder& operator=( const TimelineRecorder& ) = delete;
    TimelineRecorder( TimelineRecorder&& other ) : m_record{ std::exchange( other.m_record, nullptr ) } {}

    TimelineRecorder& operator=( TimelineRecorder&& other ) {
      m_record = std::exchange( other.m_record, nullptr );
      return *this;
    }

    ~TimelineRecorder();

  private:
    TimelineEvent* m_record = nullptr;
  };

  virtual TimelineRecorder getRecorder( std::string alg, const EventContext& ctx ) = 0;
  // Augment a partially pre-filled TimelineEvent object with matching info
  [[deprecated( "Use getFirstMatching or getLastMatching instead" )]] virtual bool
  getTimelineEvent( TimelineEvent& ) const = 0;
  // Augment a partially pre-filled TimelineEvent object with matching info. If there are multiple events for the same
  // algorithm and event context, the first one (by start time) is returned.
  virtual bool getFirstMatching( TimelineEvent& ) const = 0;
  // Augment a partially pre-filled TimelineEvent object with matching info. If there are multiple events for the same
  // algorithm and event context, the first one (by start time) is returned.
  virtual bool getLastMatching( TimelineEvent& ) const = 0;
  virtual bool isEnabled() const                       = 0;
};
