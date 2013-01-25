#ifndef GAUDIKERNEL_ISCHEDULER_H
#define GAUDIKERNEL_ISCHEDULER_H

// Framework include files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/EventContext.h"

// C++ include files


/**@class IScheduler IScheduler.h GaudiKernel/IScheduler.h
 *
 *  Interface for the schedulers. It allows to add events to the processing 
 *  and to know how many events are in flight and how many completed.
 * 
 *  @author  Danilo Piparo
 *  @version 1.0
 */
class GAUDI_API IScheduler: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IScheduler,1,0);
  
  /// Make an event available to the scheduler
  virtual StatusCode addEvent(EventContext* eventContext) = 0;
  
  /// Get number of in flight events
  virtual unsigned int eventsInFlight() = 0;
  
  /// Maximum number of events in flight
  virtual unsigned int maxEventsInFlight() = 0;
  
  /// Get number of completed events
  virtual unsigned int completedEvents() = 0;
  
  
};
#endif