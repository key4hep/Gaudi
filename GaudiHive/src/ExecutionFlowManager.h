#ifndef EXECUTIONFLOWMANAGER_H_
#define EXECUTIONFLOWMANAGER_H_

#include "ControlFlowGraph.h"

namespace concurrency {

  namespace recursive_CF {

  struct IExecutionFlowManager {
    virtual ~IExecutionFlowManager() {}
  };

  /**@class ExecutionFlowManager ExecutionFlowManager.h GaudiHive/src/ExecutionFlowManager.h
   *
   *  Manage control flow part of the execution flow.
   *  Once initialized, the CF graph is const and can be shared across events
   *
   *  @author  Benedikt Hegner
   *  @author  Illya Shapoval
   */
  class ExecutionFlowManager  : public CommonMessaging<IExecutionFlowManager> {
  public:
    /// Constructor
    ExecutionFlowManager() : m_name("ExecutionFlowManager"), m_CFGraph(0) {};
    /// Destructor
    ~ExecutionFlowManager() override = default;
    /// Initialize the control flow manager
    /// It greps the topalg list and the index map for the algo names
    void initialize(ControlFlowGraph* graph,
                          const std::unordered_map<std::string,unsigned int>& algname_index_map);
    /// Get the flow graph instance
    inline ControlFlowGraph* getCFGraph() const {return m_CFGraph;}
    /// Update states and decisions of algorithms
    void updateEventState(AlgsExecutionStates & algo_states,
                          std::vector<int>& node_decisions) const;
    /// Check whether root decision was resolved
    bool rootDecisionResolved(const std::vector<int>& node_decisions) const;
    /// Print the state of the control flow for a given event
    void printEventState(std::stringstream& ss,
                         AlgsExecutionStates& states,
                         const std::vector<int>& node_decisions,
                         const unsigned int& recursionLevel) const {
      m_CFGraph->printState(ss,states,node_decisions,recursionLevel);}
    /// Retrieve name of the service
    const std::string& name() const override {return m_name;}
    /// Retrieve pointer to service locator
    SmartIF<ISvcLocator>& serviceLocator() const override {return m_CFGraph->serviceLocator();}
  private:
    std::string m_name;
    /// the control flow graph
    ControlFlowGraph* m_CFGraph;
  };
  } // namespace recursive_CF
} // namespace concurrency

#endif /* EXECUTIONFLOWMANAGER_H_ */
