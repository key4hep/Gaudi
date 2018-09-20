#ifndef EVENTSLOT_H_
#define EVENTSLOT_H_

// Framework includes
#include "AlgsExecutionStates.h"

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

class EventContext;

/// Class representing an event slot
struct EventSlot {
  /// Construct a slot
  EventSlot( unsigned int numberOfAlgorithms, unsigned int numberOfControlFlowNodes, SmartIF<IMessageSvc> MS )
      : algsStates( numberOfAlgorithms, MS ), controlFlowState( numberOfControlFlowNodes, -1 ){};

  /// Construct a (sub)slot, nested to 'original' parent slot, with CF states copied from the parent
  EventSlot( EventSlot& original, EventContext* theeventContext, const std::string& nodeName )
      : eventContext( theeventContext )
      , algsStates( original.algsStates )
      , controlFlowState( original.controlFlowState )
      , entryPoint( nodeName )
      , parentSlot( &original )
  {
    algsStates.reset();
  };

  /// Reset all resources in order to reuse the slot (thread-unsafe)
  void reset( EventContext* theeventContext )
  {
    eventContext = theeventContext;
    algsStates.reset();
    controlFlowState.assign( controlFlowState.size(), -1 );
    complete = false;
    entryPoint.clear();
    parentSlot = nullptr;
    contextToSlot.clear();
    subSlotsByNode.clear();
    allSubSlots.clear();
  };

  /// Add a subslot to the slot (this constructs a new slot and registers it with the parent one)
  void addSubSlot( EventContext* viewContext, const std::string& nodeName )
  {
    unsigned int lastIndex = allSubSlots.size();

    // Store index of the new slot in lookup structures
    contextToSlot.emplace( viewContext, lastIndex );

    auto search = subSlotsByNode.find( nodeName );
    if ( search != subSlotsByNode.end() )
      subSlotsByNode[nodeName].push_back( lastIndex );
    else
      subSlotsByNode.emplace( std::piecewise_construct, std::forward_as_tuple( nodeName ),
                              std::forward_as_tuple( 1, lastIndex ) );

    // Make new slot and nest it into the top slot
    allSubSlots.emplace_back( *this, viewContext, nodeName );
  }

  /// Disable event views for a given CF view node by registering an empty container
  /// Contact B. W. Wynne for more details on the reasoning about this design choice
  void disableSubSlots( const std::string& nodeName )
  {
    subSlotsByNode.emplace( std::piecewise_construct, std::forward_as_tuple( nodeName ), std::forward_as_tuple() );
  }

  /// Cache for the eventContext
  EventContext* eventContext = nullptr;
  /// Vector of algorithms states
  AlgsExecutionStates algsStates;
  /// State of the control flow
  std::vector<int> controlFlowState;
  /// Flags completion of the event
  bool complete = false;

  /// Event Views bookkeeping (TODO: optimize view bookkeeping)

  /// Name of the node this slot is attached to ("" for top level)
  std::string entryPoint;
  /// Pointer to parent slot (null for top level)
  EventSlot* parentSlot = nullptr;
  /// Quick lookup for sub-slots by event context (top level only)
  std::unordered_map<EventContext*, unsigned int> contextToSlot;
  /// Listing of sub-slots by the node (name) they are attached to
  std::unordered_map<std::string, std::vector<unsigned int>> subSlotsByNode;
  /// Actual sub-slot instances
  std::vector<EventSlot> allSubSlots;
};

#endif /* EVENTSLOT_H_ */
