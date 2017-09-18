#ifndef EVENTSLOT_H_
#define EVENTSLOT_H_

// Framework includes
#include "GaudiKernel/EventContext.h"
#include "AlgsExecutionStates.h"
#include "DataFlowManager.h"

// Event slots management -------------------------------------------------
/// Class representing the event slot
class EventSlot{
public:
  EventSlot(const std::vector<DataObjIDColl>& algoDependencies,
            unsigned int numberOfAlgorithms,
            unsigned int numberOfControlFlowNodes,
            SmartIF<IMessageSvc> MS):
              eventContext(nullptr),
              algsStates(numberOfAlgorithms,MS),
              complete(false),
              dataFlowMgr(algoDependencies),
              controlFlowState(numberOfControlFlowNodes,-1){};

  ~EventSlot(){};

  /// Reset all resources in order to reuse the slot
  void reset(EventContext* theeventContext){
    eventContext=theeventContext;
    algsStates.reset();
    dataFlowMgr.reset();
    complete=false;
    controlFlowState.assign(controlFlowState.size(),-1);
  };

  /// Cache for the eventContext
  EventContext* eventContext;
  /// Vector of algorithms states
  AlgsExecutionStates algsStates;
  /// Flags completion of the event
  bool complete;
  /// DataFlowManager of this slot
  DataFlowManager dataFlowMgr;
  /// State of the control flow
  std::vector<int> controlFlowState;
};

#endif /* EVENTSLOT_H_ */
