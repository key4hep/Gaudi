#ifndef GAUDIHIVE_CONTROLFLOWMANAGER_H
#define GAUDIHIVE_CONTROLFLOWMANAGER_H

// std includes
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include <fstream>
#include <sstream>

// fwk includes
#include "AlgsExecutionStates.h"
#include "EventSlot.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/CommonMessaging.h"

namespace concurrency {

  typedef AlgsExecutionStates::State State;
  class ControlFlowGraph;
  class CFVisitor;

  class ControlFlowNode {
    friend CFVisitor;
  public:
    /// Constructor
    ControlFlowNode(ControlFlowGraph& graph, unsigned int nodeIndex, const std::string& name) :
      m_graph(&graph), m_nodeIndex(nodeIndex), m_nodeName(name) {};
    /// Destructor
    virtual ~ControlFlowNode() {};
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map) = 0;
    ///
    virtual bool accept(CFVisitor& visitor) = 0;
    /// XXX: CF tests. Method to set algos to CONTROLREADY, if possible
    virtual bool promoteToControlReadyState(const int& slotNum,
                                            AlgsExecutionStates& states,
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
    unsigned int getNodeIndex() const { return m_nodeIndex; }
    std::string getNodeName() const { return m_nodeName; }
    virtual void updateDecision(const int& slotNum,
                                AlgsExecutionStates& states,
                                std::vector<int>& node_decisions) const = 0;
  protected:
    ControlFlowGraph* m_graph;
    /// Translation between state id and name
    std::string stateToString(const int& stateId) const;
    unsigned int m_nodeIndex;
    std::string m_nodeName;
  };


  class DecisionNode : public ControlFlowNode {
    friend CFVisitor;
  public:
    /// Constructor
    DecisionNode(ControlFlowGraph& graph, unsigned int nodeIndex, const std::string& name, bool modeOR, bool allPass, bool isLazy) :
      ControlFlowNode(graph, nodeIndex, name),
      m_modeOR(modeOR), m_allPass(allPass), m_isLazy(isLazy), m_children()
      {};
    /// Destructor
    virtual ~DecisionNode();
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    virtual bool accept(CFVisitor& visitor);
    /// XXX: CF tests. Method to set algos to CONTROLREADY, if possible
    virtual bool promoteToControlReadyState(const int& slotNum,
                                            AlgsExecutionStates& states,
                                            std::vector<int>& node_decisions) const;
    /// XXX: CF tests
    virtual void updateDecision(const int& slotNum,
                                AlgsExecutionStates& states,
                                std::vector<int>& node_decisions) const;
    /// Method to set algos to CONTROLREADY, if possible
    virtual int updateState(AlgsExecutionStates& states,
                            std::vector<int>& node_decisions) const;
    /// XXX: CF tests. Method to add a parent node
    void addParentNode(DecisionNode* node);
    /// Add a daughter node
    void addDaughterNode(ControlFlowNode* node);
    ///
    std::vector<ControlFlowNode*> getDaughters() const {return m_children;}
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
    std::vector<ControlFlowNode*> m_children;
    /// XXX: CF tests. All direct parent nodes in the tree
    std::vector<DecisionNode*> m_parents;
  };

  class DataNode;

  class AlgorithmNode : public ControlFlowNode {
  public:
    /// Constructor
    AlgorithmNode(ControlFlowGraph& graph, unsigned int nodeIndex, const std::string& algoName, bool inverted, bool allPass) :
      ControlFlowNode(graph, nodeIndex, algoName),
      m_algoIndex(0),m_algoName(algoName),m_inverted(inverted),m_allPass(allPass)
      {};
    /// Destructor
    ~AlgorithmNode();
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    ///
    virtual bool accept(CFVisitor& visitor);
    /// XXX: CF tests. Method to add a parent node
    void addParentNode(DecisionNode* node);

    /// Associate an AlgorithmNode, which is a data supplier for this one
    void addSupplierNode(AlgorithmNode* node) { m_suppliers.push_back(node); }
    /// Associate an AlgorithmNode, which is a data consumer of this one
    void addConsumerNode(AlgorithmNode* node) { m_consumers.push_back(node); }
    /// Get all supplier nodes
    std::vector<AlgorithmNode*> getSupplierNodes() {return m_suppliers;}
    /// Get all consumer nodes
    std::vector<AlgorithmNode*> getConsumerNodes() {return m_consumers;}

    /// Associate an AlgorithmNode, which is a data supplier for this one
    void addOutputDataNode(DataNode* node);
    /// Associate an AlgorithmNode, which is a data consumer of this one
    void addInputDataNode(DataNode* node);
    /// Get all supplier nodes
    std::vector<DataNode*> getOutputDataNodes() {return m_outputs;}
    /// Get all consumer nodes
    std::vector<DataNode*> getInputDataNodes() {return m_inputs;}

    /// XXX: CF tests
    unsigned int getAlgoIndex() const { return m_algoIndex; }
    /// Method to check whether the Algorithm has its all data dependency satisfied
    bool dataDependenciesSatisfied(const int& slotNum) const;
    bool dataDependenciesSatisfied(AlgsExecutionStates& states) const;
    /// Method to set algos to CONTROLREADY, if possible
    virtual int updateState(AlgsExecutionStates& states,
                            std::vector<int>& node_decisions) const;
    /// XXX: CF tests
    virtual bool promoteToControlReadyState(const int& slotNum,
                                            AlgsExecutionStates& states,
                                            std::vector<int>& node_decisions) const;
    ///
    bool promoteToDataReadyState(const int& slotNum) const;
    /// XXX: CF tests
    virtual void updateDecision(const int& slotNum,
                                AlgsExecutionStates& states,
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

    /// Vectors, used in data dependencies realm
    /// AlgorithmNodes that represent algorithms producing an input needed for the algorithm
    std::vector<AlgorithmNode*> m_suppliers;
    /// AlgorithmNodes that represent algorithms which need the output of the algorithm
    std::vector<AlgorithmNode*> m_consumers;

    /// Vectors, used in augmented data dependencies realm
    /// Outputs of the algorithm, represented as DataNode's
    std::vector<DataNode*> m_outputs;
    /// Inputs of the algorithm, represented as DataNode's
    std::vector<DataNode*> m_inputs;
  };

class DataNode {
public:
    /// Constructor
    DataNode(ControlFlowGraph& graph, const std::string& path) : m_graph(&graph), m_data_object_path(path) {};
    /// Destructor
    ~DataNode() {};
    std::string getPath() {return m_data_object_path;}
    /// Associate an AlgorithmNode, which is a data supplier for this one
    void addProducerNode(AlgorithmNode* node) {
      if (std::find(m_producers.begin(),m_producers.end(),node) == m_producers.end())
        m_producers.push_back(node);
    }
    /// Associate an AlgorithmNode, which is a data consumer of this one
    void addConsumerNode(AlgorithmNode* node) {
      if (std::find(m_producers.begin(),m_producers.end(),node) == m_producers.end())
        m_consumers.push_back(node);
    }
    /// Get all data object producers
    std::vector<AlgorithmNode*> getProducers() {return m_producers;}
    /// Get all data object consumers
    std::vector<AlgorithmNode*> getConsumers() {return m_consumers;}
private:
    ControlFlowGraph* m_graph;
    std::string m_data_object_path;
    std::vector<AlgorithmNode*> m_producers;
    std::vector<AlgorithmNode*> m_consumers;
  };

typedef std::unordered_map<std::string,AlgorithmNode*> AlgoNodesMap;
typedef std::unordered_map<std::string,DecisionNode*> DecisionHubsMap;
typedef std::unordered_map<std::string,DataNode*> DataNodesMap;
typedef std::unordered_map<std::string,const DataObjectDescriptorCollection*> AlgoInputsMap;
typedef std::unordered_map<std::string,const DataObjectDescriptorCollection*> AlgoOutputsMap;

class ControlFlowManager;
class IControlFlowGraph {};

class ControlFlowGraph : public CommonMessaging<IControlFlowGraph> {
  friend ControlFlowManager;
public:
    /// Constructor
    ControlFlowGraph(const std::string& name, SmartIF<ISvcLocator> svc) :
     m_headNode(0), m_nodeCounter(0), m_svcLocator(svc), m_name(name), m_initTime(std::chrono::high_resolution_clock::now()),
     m_eventSlots(nullptr) {};
    /// Destructor
    ~ControlFlowGraph() {
      if (m_headNode != 0) delete m_headNode;
    };
    /// Initialize graph
    StatusCode initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    StatusCode initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map,
                          std::vector<EventSlot>& eventSlots);
    /// Register algorithm in the Data Dependency index
    void registerIODataObjects(const Algorithm* algo);
    /// Build data dependency realm WITHOUT data object nodes: just interconnect algorithm nodes directly
    StatusCode buildDataDependenciesRealm();
    /// Build data dependency realm WITH data object nodes participating
    StatusCode buildAugmentedDataDependenciesRealm();
    /// Add a node, which has no parents
    void addHeadNode(const std::string& headName, bool modeOR, bool allPass, bool isLazy);
    /// Add algorithm node
    StatusCode addAlgorithmNode(Algorithm* daughterAlgo, const std::string& parentName, bool inverted, bool allPass);
    /// Get the AlgorithmNode from by algorithm name using graph index
    AlgorithmNode* getAlgorithmNode(const std::string& algoName) const;
    /// Add DataNode that represents DataObject
    StatusCode addDataNode(const std::string& dataPath);
    /// Get DataNode by DataObject path using graph index
    DataNode* getDataNode(const std::string& dataPath) const;
    /// Add a node, which aggregates decisions of direct daughter nodes
    StatusCode addDecisionHubNode(Algorithm* daughterAlgo, const std::string& parentName, bool modeOR, bool allPass, bool isLazy);
    /// Get total number of graph nodes
    unsigned int getControlFlowNodeCounter() const {return m_nodeCounter;}
    /// XXX CF tests. Is needed for older CF implementation
    void updateEventState(AlgsExecutionStates& states,
                          std::vector<int>& node_decisions) const;
    /// A method to update algorithm node decision, and propagate it upwards
    void updateDecision(const std::string& algo_name,
                        const int& slotNum,
                        AlgsExecutionStates& states,
                        std::vector<int>& node_decisions) const;
    /// Print a string representing the control flow state
    void printState(std::stringstream& output,
                    AlgsExecutionStates& states,
                    const std::vector<int>& node_decisions,
                    const unsigned int& recursionLevel) const {m_headNode->printState(output,states,node_decisions,recursionLevel);};
    ///
    std::vector<AlgorithmNode*> getDataIndependentNodes() const;
    /// Retrieve name of the service
    const std::string& name() const {return m_name;}
    /// Retrieve pointer to service locator
    SmartIF<ISvcLocator>& serviceLocator() const {return m_svcLocator;}
    ///
    const std::chrono::system_clock::time_point getInitTime() const {return m_initTime;};
    ///
    AlgsExecutionStates& getAlgoStates(const int& slotNum) const {return m_eventSlots->at(slotNum).algsStates;};
    ///
    std::vector<int>& getNodeDecisions(const int& slotNum) const {return m_eventSlots->at(slotNum).controlFlowState;}

private:
    /// the head node of the control flow graph; may want to have multiple ones once supporting trigger paths
    DecisionNode* m_headNode;
    /// Index: map of algorithm's name to AlgorithmNode
    AlgoNodesMap m_algoNameToAlgoNodeMap;
    /// Index: map of decision's name to DecisionHub
    DecisionHubsMap m_decisionNameToDecisionHubMap;
    /// Index: map of data path to DataNode
    DataNodesMap m_dataPathToDataNodeMap;
    /// Indexes: maps of algorithm's name to algorithm's inputs/outputs
    AlgoInputsMap m_algoNameToAlgoInputsMap;
    AlgoOutputsMap m_algoNameToAlgoOutputsMap;
    /// Total number of nodes in the graph
    unsigned int m_nodeCounter;
    /// Service locator (needed to access the MessageSvc)
    mutable SmartIF<ISvcLocator> m_svcLocator;
    const std::string m_name;
    const std::chrono::system_clock::time_point m_initTime;
    ///
    std::vector<EventSlot>* m_eventSlots;
  };


class IVisitor {
public:
  virtual ~IVisitor() {};
  virtual bool visitEnter(DecisionNode&) = 0;
  virtual bool visit(DecisionNode&) = 0;
  virtual bool visitLeave(DecisionNode&) = 0;
  virtual bool visitEnter(AlgorithmNode&) = 0;
  virtual bool visit(AlgorithmNode& node) = 0;
};

class CFVisitor : public IVisitor {
public:
  /// Constructor
  CFVisitor(const int& slotNum) : m_algosCompleted(0), m_slotNum(slotNum) {};
  /// Destructor
  virtual ~CFVisitor() {};

  virtual bool visitEnter(DecisionNode& node) {

    if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != 1)
      return true;
    return false;
  }

  virtual bool visit(DecisionNode& node) {

    //std::cout << "1-st level Decision: " << node.getNodeName() << std::endl;
    bool allChildDecisionsResolved = true;
    for (auto child : node.getDaughters()) {
      int& childDecision = child->m_graph->getNodeDecisions(m_slotNum)[child->getNodeIndex()];

      if (childDecision == 1 && node.m_modeOR && node.m_isLazy) {
        node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] = 1;
        return true;
      }

      if (childDecision == -1) {
        allChildDecisionsResolved = false;
      }
    }

    if (allChildDecisionsResolved)
      node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] = 1;

    return allChildDecisionsResolved;
  }

  virtual bool visitLeave(DecisionNode& node) {

    if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != 1)
      return true;
    return false;
  }


  virtual bool visitEnter(AlgorithmNode& node) {

    if (node.m_graph->getNodeDecisions(m_slotNum)[node.getNodeIndex()] != 1)
      return true;
    return false;
  }


  virtual bool visit(AlgorithmNode& node) {

    std::vector<int>& decisions = node.m_graph->getNodeDecisions(m_slotNum);
    AlgsExecutionStates& states = node.m_graph->getAlgoStates(m_slotNum);
    int& decision = decisions[node.getNodeIndex()];

    if (State::INITIAL == states[node.getAlgoIndex()]) {
      states.updateState(node.getAlgoIndex(), State::CONTROLREADY);
      if (node.dataDependenciesSatisfied(m_slotNum)) {
        states.updateState(node.getAlgoIndex(), State::DATAREADY);
        states.updateState(node.getAlgoIndex(), State::SCHEDULED);
        states.updateState(node.getAlgoIndex(), State::EVTACCEPTED);
        decision = 1;
        ++m_algosCompleted;
        //std::cout << "Algorithm decided: " << node.getNodeName() << std::endl;
        return true;
      }
    } else if (State::CONTROLREADY == states[node.getAlgoIndex()] && node.dataDependenciesSatisfied(m_slotNum)) {
      states.updateState(node.getAlgoIndex(), State::DATAREADY);
      states.updateState(node.getAlgoIndex(), State::SCHEDULED);
      states.updateState(node.getAlgoIndex(), State::EVTACCEPTED);
      decision = 1;
      ++m_algosCompleted;
      //std::cout << "Algorithm decided: " << node.getNodeName() << std::endl;
      return true;
    }

    return false;
  }
public:
  int m_algosCompleted;
private:
  int m_slotNum;
};



class IControlFlowManager {};

/**@class ControlFlowManager ControlFlowManager.h GaudiHive/src/ControlFlowManager.h
 *
 *  Manage the control flow using a tree structure
 *  Once initialized, the tree is const and can be shared across events
 *
 *  @author  Benedikt Hegner
 *  @version 1.0
 */
class ControlFlowManager  : public CommonMessaging<IControlFlowManager> {
public:
  /// Constructor
  ControlFlowManager() : m_name("ControlFlowManager"), m_CFGraph(0) {};
  /// Destructor
  virtual ~ControlFlowManager() {};
  /// Initialize the control flow manager
  /// It greps the topalg list and the index map for the algo names
  StatusCode initialize(ControlFlowGraph* CFGraph,
                          const std::unordered_map<std::string,unsigned int>& algname_index_map);
  StatusCode initialize(ControlFlowGraph* CFGraph,
                        const std::unordered_map<std::string,unsigned int>& algname_index_map,
                        std::vector<EventSlot>& eventSlots);
  ///
  void simulateExecutionFlow() const;
  /// Get the flow graph instance
  ControlFlowGraph* getControlFlowGraph() const {return m_CFGraph;}
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
                       const unsigned int& recursionLevel) const {m_CFGraph->printState(ss,states,node_decisions,recursionLevel);}
  /// Retrieve name of the service
  const std::string& name() const {return m_name;}
  /// Retrieve pointer to service locator
  SmartIF<ISvcLocator>& serviceLocator() const {return m_CFGraph->serviceLocator();}
private:
  std::string m_name;
  /// the control flow graph
  ControlFlowGraph* m_CFGraph;
};


} // namespace concurrency


#endif
