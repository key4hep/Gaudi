#ifndef EXECUTIONFLOWMANAGER_H_
#define EXECUTIONFLOWMANAGER_H_

#include "IGraphVisitor.h"
#include "PrecedenceRulesGraph.h"

namespace concurrency {

  struct IExecutionFlowManager {
    virtual ~IExecutionFlowManager() {}
  };

  /**@class ExecutionFlowManager ExecutionFlowManager.h GaudiHive/src/ExecutionFlowManager.h
   *
   *  Manage the execution flow using a graph of task precedence rules
   *  Once initialized, the graph is const and can be shared across events
   *
   *  @author  Benedikt Hegner
   *  @author  Illya Shapoval
   */
  class ExecutionFlowManager  : public CommonMessaging<IExecutionFlowManager> {
  public:
    /// Constructor
    ExecutionFlowManager() : m_name("ExecutionFlowManager"), m_PRGraph(0) {};
    /// Destructor
    ~ExecutionFlowManager() override = default;
    /// Initialize the control flow manager
    /// It greps the topalg list and the index map for the algo names
    StatusCode initialize(PrecedenceRulesGraph* graph,
                            const std::unordered_map<std::string,unsigned int>& algname_index_map);
    StatusCode initialize(PrecedenceRulesGraph* graph,
                          const std::unordered_map<std::string,unsigned int>& algname_index_map,
                          const std::string& mode,
                          bool enableCondSvc);
    ///
    void simulateExecutionFlow(IGraphVisitor& visitor) const;
    /// Get the flow graph instance
    inline PrecedenceRulesGraph* getPrecedenceRulesGraph() const {return m_PRGraph;}
    /// A little bit silly, but who cares. ;-)
    bool needsAlgorithmToRun(const unsigned int iAlgo) const;
    /// Update the state of algorithms to controlready, where possible
    void updateEventState(AlgsExecutionStates & algo_states,
                          std::vector<int>& node_decisions) const;
    ///
    void updateDecision(const std::string& algo_name, IGraphVisitor& visitor) const;
    /// XXX: CF tests.
    void updateEventState(AlgsExecutionStates& algo_states) const;
    /// Check whether root decision was resolved
    bool rootDecisionResolved(const std::vector<int>& node_decisions) const;
    /// Print the state of the control flow for a given event
    void printEventState(std::stringstream& ss,
                         AlgsExecutionStates& states,
                         const std::vector<int>& node_decisions,
                         const unsigned int& recursionLevel) const {m_PRGraph->printState(ss,states,node_decisions,recursionLevel);}
    /// Promote all algorithms, ready to be executed, to DataReady state
    void touchReadyAlgorithms(IGraphVisitor& visitor) const;
    /// Retrieve name of the service
    const std::string& name() const override {return m_name;}
    /// Retrieve pointer to service locator
    SmartIF<ISvcLocator>& serviceLocator() const override {return m_PRGraph->serviceLocator();}
  private:
    std::string m_name;
    /// the graph of precedence rules
    PrecedenceRulesGraph* m_PRGraph;
  };

} // namespace concurrency

#endif /* EXECUTIONFLOWMANAGER_H_ */
