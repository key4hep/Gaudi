/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_ISCHEDULER_H
#define GAUDIKERNEL_ISCHEDULER_H

// Framework include files
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IInterface.h"

// C++ include files
#include <chrono>
#include <functional>
#include <memory>
#include <vector>

/**@class IScheduler IScheduler.h GaudiKernel/IScheduler.h
 *
 *  General interface for algorithm scheduler.
 *
 *  @author  Danilo Piparo
 *  @author  Benedikt Hegner
 *  @version 1.1
 */
class GAUDI_API IScheduler : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IScheduler, 1, 0 );

  /// Make an event available to the scheduler
  virtual StatusCode pushNewEvent( EventContext* eventContext ) = 0;

  /// Make a list of events available to the scheduler
  /// This method makes a bunch creation of new events atomic to the scheduler
  virtual StatusCode pushNewEvents( std::vector<EventContext*>& eventContexts ) = 0;

  /// Retrieve a finished event from the scheduler
  virtual StatusCode popFinishedEvent( EventContext*& eventContext ) = 0;

  /// Try to retrieve a finished event from the scheduler
  virtual StatusCode tryPopFinishedEvent( EventContext*& eventContext ) = 0;

  /// Get the free event processing slots
  virtual unsigned int freeSlots() = 0;

  virtual void dumpState(){};

  /// Method to inform the scheduler about event views
  virtual StatusCode scheduleEventView( const EventContext* sourceContext, const std::string& nodeName,
                                        std::unique_ptr<EventContext> viewContext ) = 0;

  /// Sample occupancy at fixed interval (ms)
  /// Negative value to deactivate, 0 to snapshot every change
  /// Each sample, apply the callback function to the result
  struct OccupancySnapshot {
    std::chrono::system_clock::time_point time;
    std::vector<std::vector<int>>         states;
  };
  virtual void recordOccupancy( int samplePeriod, std::function<void( OccupancySnapshot )> callback ) = 0;
};
#endif
