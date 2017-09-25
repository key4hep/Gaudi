#ifndef GAUDIKERNEL_ISCHEDULERSVC_H
#define GAUDIKERNEL_ISCHEDULERSVC_H

// Framework include files
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IScheduler.h"

// C++ include files

/**@class IScheduler IScheduler.h GaudiKernel/IScheduler.h
 *
 *  Interface for the schedulers. It allows to add events to the processing
 *  and to know how many events are in flight and how many completed.
 *
 *  @author  Danilo Piparo
 *  @version 1.0
 */
class GAUDI_API IScheduler : virtual public IInterface
{
public:
  /// InterfaceID
  DeclareInterfaceID( IScheduler, 1, 0 );

  /// Make an event available to the scheduler
  virtual StatusCode addEvent( EventContext* eventContext ) = 0;

  /// Get number of in flight events
  virtual unsigned int getEventsInFlight() = 0;

  /// Get number of completed events
  virtual unsigned int getCompletedEvents() = 0;
};
#endif