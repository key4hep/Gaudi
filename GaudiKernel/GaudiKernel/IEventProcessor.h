// $Id: IEventProcessor.h,v 1.6 2005/01/11 17:11:06 mato Exp $
#ifndef GAUDIKERNEL_IEVENTPROCESSOR_H
#define GAUDIKERNEL_IEVENTPROCESSOR_H 1

// Include files
#include "GaudiKernel/IInterface.h"

// Forward declarations
class IEventProcessor;

/** @class IEventProcessor IEventProcessor.h GaudiKernel/IEventProcessor.h
 *  The IEventProcessor is the interface to process events.
 *
 *  @author Markus Frank
 *   @version 2
 */
class GAUDI_API IEventProcessor: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IEventProcessor,3,0);
  /// Process single event
  virtual StatusCode executeEvent(void* par=0) = 0;
  /// Process the maxevt events as a Run (beginRun() and endRun() called)
  virtual StatusCode executeRun(int maxevt) = 0;
  /// Process the next maxevt events
  virtual StatusCode nextEvent(int maxevt) = 0;
  /// Schedule a stop of the current event processing
  virtual StatusCode stopRun() = 0;
};

#endif // GAUDIKERNEL_IEVENTPROCESSOR_H

