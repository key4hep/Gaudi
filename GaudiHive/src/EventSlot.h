#ifndef EVENTSLOT_H_
#define EVENTSLOT_H_

// Framework includes
#include "AlgsExecutionStates.h"
#include "GaudiKernel/EventContext.h"

// Event slots management -------------------------------------------------
/// Class representing the event slot
class EventSlot
{
public:
  EventSlot( unsigned int numberOfAlgorithms, unsigned int numberOfControlFlowNodes, SmartIF<IMessageSvc> MS )
      : eventContext( nullptr )
      , algsStates( numberOfAlgorithms, MS )
      , complete( false )
      , controlFlowState( numberOfControlFlowNodes, -1 ){};

  ~EventSlot(){};

  /// Reset all resources in order to reuse the slot
  void reset( EventContext* theeventContext )
  {
    eventContext = theeventContext;
    algsStates.reset();
    complete = false;
    controlFlowState.assign( controlFlowState.size(), -1 );
  };

  /// Cache for the eventContext
  EventContext* eventContext;
  /// Vector of algorithms states
  AlgsExecutionStates algsStates;
  /// Flags completion of the event
  bool complete;
  /// State of the control flow
  std::vector<int> controlFlowState;
};

#endif /* EVENTSLOT_H_ */
