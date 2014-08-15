#ifndef EXECUTIONFLOWMANAGER_H_
#define EXECUTIONFLOWMANAGER_H_

#include "ExecutionFlowGraph.h"
#include "EFGraphVisitors.h"

namespace concurrency {

  class IExecutionFlowManager {};

  /**@class ExecutionFlowManager ExecutionFlowManager.h GaudiHive/src/ExecutionFlowManager.h
   *
   *  Manage the execution flow using an execution flow graph
   *  Once initialized, the graph is const and can be shared across events
   *
   *  @author  Benedikt Hegner
   *  @author  Illya Shapoval
   */
  class ExecutionFlowManager  : public CommonMessaging<IExecutionFlowManager> {
  public:
    /// Constructor
    ExecutionFlowManager() : m_name("ExecutionFlowManager"), m_EFGraph(0) {};
    /// Destructor
    virtual ~ExecutionFlowManager() {};
    /// Initialize the control flow manager
    /// It greps the topalg list and the index map for the algo names
    StatusCode initialize(ExecutionFlowGraph* CFGraph,
                            const std::unordered_map<std::string,unsigned int>& algname_index_map);
    StatusCode initialize(ExecutionFlowGraph* CFGraph,
                          const std::unordered_map<std::string,unsigned int>& algname_index_map,
                          std::vector<EventSlot>& eventSlots);
    ///
    void simulateExecutionFlow(IGraphVisitor& visitor) const;
    /// Get the flow graph instance
    inline ExecutionFlowGraph* getExecutionFlowGraph() const {return m_EFGraph;}
    /// A little bit silly, but who cares. ;-)
    bool needsAlgorithmToRun(const unsigned int iAlgo) const;
    /// Update the state of algorithms to controlready, where possible
    void updateEventState(AlgsExecutionStates & algo_states,
                          std::vector<int>& node_decisions) const;
    ///
    void updateDecision(const std::string& algo_name,
                        const int& slotNum,
                        AlgsExecutionStates& states,
                        std::vector<int>& node_decisions) const;
    /// XXX: CF tests.
    void updateEventState(AlgsExecutionStates& algo_states) const;
    /// XXX: CF tests
    void promoteToControlReadyState(AlgsExecutionStates& algo_states,
                                    std::vector<int>& node_decisions,
                                    const int& slotNum=-1) const;
    /// Check all data dependencies of an algorithm are satisfied
    bool algoDataDependenciesSatisfied(const std::string& algo_name, const int& slotNum) const;
    /// Check whether root decision was resolved
    bool rootDecisionResolved(const std::vector<int>& node_decisions) const;
    /// Print the state of the control flow for a given event
    void printEventState(std::stringstream& ss,
                         AlgsExecutionStates& states,
                         const std::vector<int>& node_decisions,
                         const unsigned int& recursionLevel) const {m_EFGraph->printState(ss,states,node_decisions,recursionLevel);}
    /// Promote all algorithms, ready to be executed, to DataReady state
    void touchReadyAlgorithms(IGraphVisitor& visitor) const;
    /// Retrieve name of the service
    const std::string& name() const {return m_name;}
    /// Retrieve pointer to service locator
    SmartIF<ISvcLocator>& serviceLocator() const {return m_EFGraph->serviceLocator();}
  private:
    std::string m_name;
    /// the control flow graph
    ExecutionFlowGraph* m_EFGraph;
  };

} // namespace concurrency

#endif /* EXECUTIONFLOWMANAGER_H_ */
