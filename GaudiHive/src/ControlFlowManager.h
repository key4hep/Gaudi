#ifndef GAUDIHIVE_CONTROLFLOWMANAGER_H
#define GAUDIHIVE_CONTROLFLOWMANAGER_H

// std includes
#include <vector>
#include <unordered_map>

// fwk includes
#include "AlgsExecutionStates.h"
#include "GaudiKernel/CommonMessaging.h"

namespace concurrency {

  typedef AlgsExecutionStates::State State;

  class ControlFlowNode {
  public:
    /// Constructor
    ControlFlowNode(unsigned int& index, const std::string& name) : m_nodeIndex(index), m_nodeName(name) {};
    /// Destructor
    virtual ~ControlFlowNode() {};
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map) = 0;
    /// XXX: CF tests. Method to set algos to CONTROLREADY, if possible
    virtual void promoteToControlReadyState(AlgsExecutionStates& states,
                                            std::vector<int>& node_decisions) const = 0;
    /// XXX: CF tests. Method to set algos to CONTROLREADY, if possible
    virtual int updateState(AlgsExecutionStates& states,
                            std::vector<int>& node_decisions) const = 0;
    /// Print a string representing the control flow state
    virtual void printState(std::stringstream& output,
    						AlgsExecutionStates& states,
                            const std::vector<int>& node_decisions,
                            const unsigned int& recursionLevel) const = 0;
    /// XXX: CF tests.
    unsigned int getNodeIndex() { return m_nodeIndex; }
    std::string getNodeName() { return m_nodeName; }
    virtual void updateDecision(AlgsExecutionStates& states,
                                std::vector<int>& node_decisions) const = 0;
  protected:
    /// Translation between state id and name
    std::string stateToString(const int& stateId) const;
    unsigned int m_nodeIndex;
    std::string m_nodeName;
  };


  class DecisionNode : public ControlFlowNode {
  public:
    /// Constructor
    DecisionNode(unsigned int& index, const std::string& name, bool modeOR, bool allPass, bool isLazy) :
      ControlFlowNode(index, name),
      m_modeOR(modeOR), m_allPass(allPass), m_isLazy(isLazy), m_daughters()
      {};
    /// Destructor
    virtual ~DecisionNode();
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    /// XXX: CF tests. Method to set algos to CONTROLREADY, if possible
    virtual void promoteToControlReadyState(AlgsExecutionStates& states,
                                            std::vector<int>& node_decisions) const;
    /// XXX: CF tests
    virtual void updateDecision(AlgsExecutionStates& states,
                                std::vector<int>& node_decisions) const;
    /// Method to set algos to CONTROLREADY, if possible
    virtual int updateState(AlgsExecutionStates& states,
                            std::vector<int>& node_decisions) const;
    /// XXX: CF tests. Method to add a parent node
    void addParentNode(DecisionNode* node) { m_parents.push_back(node); }
    /// Add a daughter node
    void addDaughterNode(ControlFlowNode* node) { m_daughters.push_back(node); }
    /// Print a string representing the control flow state
    virtual void printState(std::stringstream& output,
    						AlgsExecutionStates& states,
                            const std::vector<int>& node_decisions,
                            const unsigned int& recursionLevel) const;
  private:
    /// Whether acting as "and" (false) or "or" node (true)
    bool m_modeOR;
    /// Whether always passing regardless of daughter results
    bool m_allPass;
    /// Whether to evaluate lazily - i.e. whether to stop once result known
    bool  m_isLazy;
    /// All direct daughter nodes in the tree
    std::vector<ControlFlowNode*> m_daughters;
    /// XXX: CF tests. All direct parent nodes in the tree
    std::vector<DecisionNode*> m_parents;
  };


  class AlgorithmNode : public ControlFlowNode {
  public:
    AlgorithmNode(unsigned int& index, const std::string& algoName, bool inverted, bool allPass) :
      ControlFlowNode(index, algoName),
      m_algoIndex(0),m_algoName(algoName),m_inverted(inverted),m_allPass(allPass)
      {};
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    /// XXX: CF tests. Method to add a parent node
    void addParentNode(DecisionNode* node) { m_parents.push_back(node); }
    /// XXX: CF tests
    unsigned int getAlgoIndex() { return m_algoIndex; }
    /// Method to set algos to CONTROLREADY, if possible
    virtual int updateState(AlgsExecutionStates& states,
                            std::vector<int>& node_decisions) const;
    /// XXX: CF tests.
    virtual void promoteToControlReadyState(AlgsExecutionStates& states,
                                            std::vector<int>& node_decisions) const;
    /// XXX: CF tests.
    virtual void updateDecision(AlgsExecutionStates& states,
                                std::vector<int>& node_decisions) const;
    /// Print a string representing the control flow state
    virtual void printState(std::stringstream& output,
    						AlgsExecutionStates& states,
                            const std::vector<int>& node_decisions,
                            const unsigned int& recursionLevel) const;
  private:
    /// The index of the algorithm
    unsigned int m_algoIndex;
    /// The name of the algorithm
    std::string m_algoName;
    /// Whether the selection result is negated or not
    bool m_inverted;
    /// Whether the selection result is relevant or always "pass"
    bool m_allPass;
    /// XXX: CF tests
    std::vector<DecisionNode*> m_parents;
  };

typedef std::unordered_map<std::string,AlgorithmNode*> GraphAlgoMap;
typedef std::unordered_map<std::string,DecisionNode*> GraphAggregateMap;

class IControlFlowGraph {};

class ControlFlowGraph : public CommonMessaging<IControlFlowGraph> {
public:
    /// Constructor
    ControlFlowGraph(const std::string& name, SmartIF<ISvcLocator> svc) :
      m_headNode(0), m_nodeCounter(0), m_svcLocator(svc), m_name(name) {};
    /// Destructor
    ~ControlFlowGraph() {
      if (m_headNode != 0) delete m_headNode;
    };
    /// Initialize graph
    void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    /// Add a node, which has no parents
    void addHeadNode(const std::string& headName, bool modeOR, bool allPass, bool isLazy);
    /// Add algorithm node
    void addAlgorithmNode(const std::string& daughterName, const std::string& parentName, bool inverted, bool allPass);
    /// Add a node, which aggregates decisions of direct daughter nodes
    void addAggregateNode(const std::string& daughterName, const std::string& parentName, bool modeOR, bool allPass, bool isLazy);
    /// Get total number of graph nodes
    unsigned int getControlFlowNodeCounter() const {return m_nodeCounter;}
    /// XXX CF tests. Is needed for older CF implementation
    void updateEventState(AlgsExecutionStates& states,
                          std::vector<int>& node_decisions) const;
    /// A method to update algorithm node decision, and propagate it upwards
    void updateDecision(const std::string& algo_name,
                        AlgsExecutionStates& states,
                        std::vector<int>& node_decisions) const;
    /// XXX CF tests. A method to promote algorithm to Control Ready state (is used only to trigger the chain reaction of execution)
    void promoteToControlReadyState(AlgsExecutionStates& states,
                                    std::vector<int>& node_decisions) const;
    /// Print a string representing the control flow state
    void printState(std::stringstream& output,
                    AlgsExecutionStates& states,
                    const std::vector<int>& node_decisions,
                    const unsigned int& recursionLevel) const {m_headNode->printState(output,states,node_decisions,recursionLevel);};
    /// Retrieve name of the service
    const std::string& name() const {return m_name;}
    /// Retrieve pointer to service locator
    SmartIF<ISvcLocator>& serviceLocator() const {return m_svcLocator;}
private:
    /// the head node of the control flow graph; may want to have multiple ones once supporting trigger paths
    DecisionNode* m_headNode;
    GraphAlgoMap m_graphAlgoMap;
    GraphAggregateMap m_graphAggMap;
    unsigned int m_nodeCounter;
    /// Service locator (needed to access the MessageSvc)
    mutable SmartIF<ISvcLocator> m_svcLocator;
    const std::string m_name;
  };

/**@class ControlFlowManager ControlFlowManager.h GaudiHive/src/ControlFlowManager.h
 *
 *  Manage the control flow using a tree structure
 *  Once initialized, the tree is const and can be shared across events
 *
 *  @author  Benedikt Hegner
 *  @version 1.0
 */
class ControlFlowManager{
public:
  /// Constructor
  ControlFlowManager() : m_CFGraph(0) {};
  /// Destructor
  virtual ~ControlFlowManager() {};
  ///
  ControlFlowGraph* getControlFlowGraph() {return m_CFGraph;}
  /// A little bit silly, but who cares. ;-)
  bool needsAlgorithmToRun(const unsigned int iAlgo) const;
  /// Update the state of algorithms to controlready, where possible
  void updateEventState(AlgsExecutionStates & algo_states,
                        std::vector<int>& node_decisions) const;
  ///
  void updateDecision(const std::string& algo_name,
                      AlgsExecutionStates& states,
                      std::vector<int>& node_decisions) const;
  /// XXX: CF tests.
  void updateEventState(AlgsExecutionStates& algo_states) const;
  /// XXX: CF tests
  void promoteToControlReadyState(AlgsExecutionStates& algo_states,
                                  std::vector<int>& node_decisions) const;
  /// Initialize the control flow manager
  /// It greps the topalg list and the index map for the algo names
  void initialize(ControlFlowGraph* CFGraph,
                  const std::unordered_map<std::string,unsigned int>& algname_index_map);
  /// Print the state of the control flow for a given event
  void printEventState(std::stringstream& ss,AlgsExecutionStates& states,
                       const std::vector<int>& node_decisions,
                       const unsigned int& recursionLevel) const {m_CFGraph->printState(ss,states,node_decisions,recursionLevel);}
private:
  /// the control flow graph
  ControlFlowGraph* m_CFGraph;
};


} // namespace concurrency


#endif
