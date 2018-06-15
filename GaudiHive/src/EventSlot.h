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
      , controlFlowState( numberOfControlFlowNodes, -1 )
      , entryPoint( "" )
      , parentSlot( nullptr ){};

  ~EventSlot(){};

  /// Copy a slot as a subslot with a new context
  EventSlot( EventSlot& original, EventContext* theeventContext )
      : eventContext( theeventContext )
      , algsStates( original.algsStates )
      , complete( false )
      , controlFlowState( original.controlFlowState )
      , entryPoint( "" )
      , parentSlot( &original ){};

  /// Reset all resources in order to reuse the slot
  void reset( EventContext* theeventContext )
  {
    eventContext = theeventContext;
    algsStates.reset();
    complete = false;
    controlFlowState.assign( controlFlowState.size(), -1 );
    subSlotsByNode.clear();
    entryPoint = "";
    parentSlot = nullptr;
    contextToSlot.clear();
    allSubSlots.clear();
  };

  /// Cache for the eventContext
  EventContext* eventContext;
  /// Vector of algorithms states
  AlgsExecutionStates algsStates;
  /// Flags completion of the event
  bool complete;
  /// State of the control flow
  std::vector<int> controlFlowState;
  /// Listing of sub-slots by the node (name) they are attached to
  std::map<std::string, std::vector<unsigned int>> subSlotsByNode;
  /// Name of the node this slot is attached to ("" for top level)
  std::string entryPoint;
  /// Pointer to parent slot (null for top level)
  EventSlot* parentSlot;
  /// Quick lookup for sub-slots by event context (top level only)
  std::map<EventContext*, unsigned int> contextToSlot;
  /// Actual sub-slot instances
  std::vector<EventSlot> allSubSlots;
};

#endif /* EVENTSLOT_H_ */
