#ifndef GAUDIHIVE_CONTROLFLOWGRAPH_H
#define GAUDIHIVE_CONTROLFLOWGRAPH_H

// std includes
#include <vector>
#include <unordered_map>
#include <sstream>

// local includes
#include "AlgsExecutionStates.h"

// Framework includes
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/CommonMessaging.h"


namespace concurrency {

  namespace recursive_CF {

  typedef AlgsExecutionStates::State State;
  class ControlFlowGraph;

  class ControlFlowNode {
  public:
    /// Constructor
    ControlFlowNode(ControlFlowGraph& graph, unsigned int nodeIndex, const std::string& name) :
      m_graph(&graph), m_nodeIndex(nodeIndex), m_nodeName(name) {}
    /// Destructor
    virtual ~ControlFlowNode() {}
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map) = 0;
    /// Method to set algos to CONTROLREADY, if possible
    virtual int updateState(AlgsExecutionStates& states,
                            std::vector<int>& node_decisions) const = 0;
    /// Print a string representing the control flow state
    virtual void printState(std::stringstream& output,
                            AlgsExecutionStates& states,
                            const std::vector<int>& node_decisions,
                            const unsigned int& recursionLevel) const = 0;
    /// Get node index
    const unsigned int& getNodeIndex() const { return m_nodeIndex; }
    /// Get node name
    const std::string& getNodeName() const { return m_nodeName; }

  public:
    ControlFlowGraph* m_graph;
  protected:
    /// Translation between state id and name
    std::string stateToString(const int& stateId) const;
    unsigned int m_nodeIndex;
    std::string m_nodeName;
  };


  class DecisionNode : public ControlFlowNode {
  public:
    /// Constructor
    DecisionNode(ControlFlowGraph& graph, unsigned int nodeIndex,
                 const std::string& name, bool modeConcurrent,
                 bool modePromptDecision, bool modeOR, bool allPass) :
      ControlFlowNode(graph, nodeIndex, name),
	  m_modeConcurrent(modeConcurrent), m_modePromptDecision(modePromptDecision),
	  m_modeOR(modeOR), m_allPass(allPass), m_children()
      {}
    /// Destructor
    ~DecisionNode() override;
    /// Initialize
    void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map) override;
    /// Method to set algos to CONTROLREADY, if possible
    int updateState(AlgsExecutionStates& states,
                    std::vector<int>& node_decisions) const override;
    /// Add a daughter node
    void addDaughterNode(ControlFlowNode* node);
    ///
    const std::vector<ControlFlowNode*>& getDaughters() const {return m_children;}
    /// Print a string representing the control flow state
    void printState(std::stringstream& output,
                            AlgsExecutionStates& states,
                            const std::vector<int>& node_decisions,
                            const unsigned int& recursionLevel) const override;
  public:
    /// Whether all daughters will be evaluated concurrently or sequentially
    bool m_modeConcurrent;
    /// Whether to evaluate the hub decision ASA its child decisions allow to do that.
    /// Applicable to both concurrent and sequential cases.
    bool  m_modePromptDecision;
    /// Whether acting as "and" (false) or "or" node (true)
    bool m_modeOR;
    /// Whether always passing regardless of daughter results
    bool m_allPass;
  private:
    /// All direct daughter nodes in the tree
    std::vector<ControlFlowNode*> m_children;
  };

  class AlgorithmNode : public ControlFlowNode {
  public:
    /// Constructor
    AlgorithmNode(ControlFlowGraph& graph, Algorithm* algoPtr,
                  unsigned int nodeIndex, unsigned int algoIndex, bool inverted,
                  bool allPass) :
      ControlFlowNode(graph, nodeIndex, algoPtr->name()),
      m_algoIndex(algoIndex),m_algoName(algoPtr->name()),m_inverted(inverted),
      m_allPass(allPass),m_algorithm(algoPtr) {};

    /// Initialize
    void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map) override;
    /// get Algorithm representatives
    Algorithm* getAlgorithm () const { return m_algorithm; }
    /// Method to set algos to CONTROLREADY, if possible
    int updateState(AlgsExecutionStates& states,
                            std::vector<int>& node_decisions) const override;
    /// Print a string representing the control flow state
    void printState(std::stringstream& output,
                            AlgsExecutionStates& states,
                            const std::vector<int>& node_decisions,
                            const unsigned int& recursionLevel) const override;
  private:
    /// The index of the algorithm
    unsigned int m_algoIndex;
    /// The name of the algorithm
    std::string m_algoName;
    /// Whether the selection result is negated or not
    bool m_inverted;
    /// Whether the selection result is relevant or always "pass"
    bool m_allPass;
    /// Algorithm representative behind the AlgorithmNode
    Algorithm* m_algorithm;
  };


  typedef std::unordered_map<std::string,AlgorithmNode*> AlgoNodesMap;
  typedef std::unordered_map<std::string,DecisionNode*> DecisionHubsMap;


struct IControlFlowGraph {
  virtual ~IControlFlowGraph() = default;
};

class ControlFlowGraph : public CommonMessaging<IControlFlowGraph> {
public:
    /// Constructor
    ControlFlowGraph(const std::string& name, SmartIF<ISvcLocator> svc) :
     m_svcLocator(svc), m_name(name) {}
    /// Destructor
    ~ControlFlowGraph() override {
      if (m_headNode != 0) delete m_headNode;
    }

    /// Initialize graph
    void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    /// Add a node, which has no parents
    void addHeadNode(const std::string& headName, bool modeConcurrent,
                     bool modePromptDecision, bool modeOR, bool allPass);
    /// Get head node
    DecisionNode* getHeadNode() const { return m_headNode; };
    /// Add algorithm node
    StatusCode addAlgorithmNode(Algorithm* daughterAlgo,
                                const std::string& parentName,
                                bool inverted, bool allPass);
    /// Add a node, which aggregates decisions of direct daughter nodes
    StatusCode addDecisionHubNode(Algorithm* daughterAlgo,
                                  const std::string& parentName,
                                  bool modeConcurrent,
                                  bool modePromptDecision, bool modeOR,
                                  bool allPass);
    /// Get total number of graph nodes
    unsigned int getControlFlowNodeCounter() const {return m_nodeCounter;}
    /// Start revision of states and decisions
    void updateEventState(AlgsExecutionStates& states,
                          std::vector<int>& node_decisions) const;
    /// Print a string representing the control flow state
    void printState(std::stringstream& output,
                    AlgsExecutionStates& states,
                    const std::vector<int>& node_decisions,
                    const unsigned int& recursionLevel) const
    {m_headNode->printState(output,states,node_decisions,recursionLevel);}
    /// Retrieve name of the service
    const std::string& name() const override {return m_name;}
    /// Retrieve pointer to service locator
    SmartIF<ISvcLocator>& serviceLocator() const override {return m_svcLocator;}
    /// Print out control flow of Algorithms and Sequences
    std::string dumpControlFlow() const;
    ///
    void dumpControlFlow(std::ostringstream&, ControlFlowNode*, const int&) const;

private:
    /// the head node of the control flow graph; may want to have multiple ones once supporting trigger paths
    DecisionNode* m_headNode{nullptr};
    /// Index: map of algorithm's name to AlgorithmNode
    AlgoNodesMap m_algoNameToAlgoNodeMap;
    /// Index: map of decision's name to DecisionHub
    DecisionHubsMap m_decisionNameToDecisionHubMap;
    /// Total number of nodes in the graph
    unsigned int m_nodeCounter{0};
    /// Total number of algorithm nodes in the graph
    unsigned int m_algoCounter{0};
    /// Service locator (needed to access the MessageSvc)
    mutable SmartIF<ISvcLocator> m_svcLocator;
    const std::string m_name;

  };

  }  // namespace recursive_CF
} // namespace concurrency


#endif
