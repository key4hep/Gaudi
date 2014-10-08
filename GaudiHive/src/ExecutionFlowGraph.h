#ifndef GAUDIHIVE_EXECUTIONFLOWGRAPH_H
#define GAUDIHIVE_EXECUTIONFLOWGRAPH_H

// std includes
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include <fstream>
#include <sstream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphml.hpp>

// fwk includes
#include "AlgsExecutionStates.h"
#include "EventSlot.h"
#include "IGraphVisitor.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/CommonMessaging.h"

#include "CPUCruncher.h"

namespace boost {

  struct AlgoNodeStruct {
    AlgoNodeStruct () {}
    AlgoNodeStruct (const std::string& name, const int index = -1, const int& data_rank = -1, const double& runtime = -1) :
      m_name(name), m_index(index), m_dataRank(data_rank), m_runtime(runtime) {}
    std::string m_name;
    int m_index;
    int m_dataRank;
    double m_runtime;
  };

  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, AlgoNodeStruct> ExecPlan;
  typedef graph_traits<ExecPlan>::vertex_descriptor AlgoVertex;
}

namespace concurrency {

  typedef AlgsExecutionStates::State State;
  class ExecutionFlowGraph;

  class ControlFlowNode {
  public:
    /// Constructor
    ControlFlowNode(ExecutionFlowGraph& graph, unsigned int nodeIndex, const std::string& name) :
      m_graph(&graph), m_nodeIndex(nodeIndex), m_nodeName(name) {};
    /// Destructor
    virtual ~ControlFlowNode() {};
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map) = 0;
    ///
    virtual bool accept(IGraphVisitor& visitor) = 0;
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
    const unsigned int& getNodeIndex() const { return m_nodeIndex; }
    const std::string& getNodeName() const { return m_nodeName; }
    virtual void updateDecision(const int& slotNum,
                                AlgsExecutionStates& states,
                                std::vector<int>& node_decisions,
                                const AlgorithmNode* requestor = nullptr) const = 0;
  public:
    ExecutionFlowGraph* m_graph;
  protected:
    /// Translation between state id and name
    std::string stateToString(const int& stateId) const;
    unsigned int m_nodeIndex;
    std::string m_nodeName;
  };


  class DecisionNode : public ControlFlowNode {
  public:
    /// Constructor
    DecisionNode(ExecutionFlowGraph& graph, unsigned int nodeIndex, const std::string& name, bool modeOR, bool allPass, bool isLazy) :
      ControlFlowNode(graph, nodeIndex, name),
      m_modeOR(modeOR), m_allPass(allPass), m_isLazy(isLazy), m_children()
      {};
    /// Destructor
    virtual ~DecisionNode();
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    virtual bool accept(IGraphVisitor& visitor);
    /// XXX: CF tests. Method to set algos to CONTROLREADY, if possible
    virtual bool promoteToControlReadyState(const int& slotNum,
                                            AlgsExecutionStates& states,
                                            std::vector<int>& node_decisions) const;
    /// XXX: CF tests
    virtual void updateDecision(const int& slotNum,
                                AlgsExecutionStates& states,
                                std::vector<int>& node_decisions,
                                const AlgorithmNode* requestor = nullptr) const;
    /// Method to set algos to CONTROLREADY, if possible
    virtual int updateState(AlgsExecutionStates& states,
                            std::vector<int>& node_decisions) const;
    /// XXX: CF tests. Method to add a parent node
    void addParentNode(DecisionNode* node);
    /// Add a daughter node
    void addDaughterNode(ControlFlowNode* node);
    ///
    const std::vector<ControlFlowNode*>& getDaughters() const {return m_children;}
    /// Print a string representing the control flow state
    virtual void printState(std::stringstream& output,
                            AlgsExecutionStates& states,
                            const std::vector<int>& node_decisions,
                            const unsigned int& recursionLevel) const;
  public:
    /// Whether acting as "and" (false) or "or" node (true)
    bool m_modeOR;
    /// Whether always passing regardless of daughter results
    bool m_allPass;
    /// Whether to evaluate lazily - i.e. whether to stop once result known
    bool  m_isLazy;
  private:
    /// All direct daughter nodes in the tree
    std::vector<ControlFlowNode*> m_children;
    /// XXX: CF tests. All direct parent nodes in the tree
    std::vector<DecisionNode*> m_parents;
  };

  class DataNode;

  class AlgorithmNode : public ControlFlowNode {
  public:
    /// Constructor
    AlgorithmNode(ExecutionFlowGraph& graph, unsigned int nodeIndex, const std::string& algoName, bool inverted, bool allPass) :
      ControlFlowNode(graph, nodeIndex, algoName),
      m_algoIndex(0),m_algoName(algoName),m_inverted(inverted),m_allPass(allPass),m_outputRank(-1)
      {};
    /// Destructor
    ~AlgorithmNode();
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    ///
    virtual bool accept(IGraphVisitor& visitor);
    /// XXX: CF tests. Method to add a parent node
    void addParentNode(DecisionNode* node);

    /// Associate an AlgorithmNode, which is a data supplier for this one
    void addSupplierNode(AlgorithmNode* node) { m_suppliers.push_back(node); }
    /// Associate an AlgorithmNode, which is a data consumer of this one
    void addConsumerNode(AlgorithmNode* node) { m_consumers.push_back(node); }
    /// Attach Algorithm representative
    void attachAlgorithm(IAlgorithm* ialgo) { m_representatives.push_back(ialgo); }
    /// get Algorithm representatives
    const std::vector<IAlgorithm*>& getAlgorithmRepresentatives () const { return m_representatives; }
    /// Get all supplier nodes
    const std::vector<AlgorithmNode*>& getSupplierNodes() const {return m_suppliers;}
    /// Get all consumer nodes
    const std::vector<AlgorithmNode*>& getConsumerNodes() const {return m_consumers;}

    /// Associate an AlgorithmNode, which is a data supplier for this one
    void addOutputDataNode(DataNode* node);
    /// Associate an AlgorithmNode, which is a data consumer of this one
    void addInputDataNode(DataNode* node);
    /// Get all supplier nodes
    const std::vector<DataNode*>& getOutputDataNodes() const {return m_outputs;}
    /// Get all consumer nodes
    const std::vector<DataNode*>& getInputDataNodes() const {return m_inputs;}
    /// Set output data rank
    void setOutputDataRank(unsigned int& rank) {m_outputRank = rank;}
    /// Set output data rank
    const int& getOutputDataRank() const {return m_outputRank;}

    /// XXX: CF tests
    const unsigned int& getAlgoIndex() const { return m_algoIndex; }
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
    bool promoteToDataReadyState(const int& slotNum, const AlgorithmNode* requestor = nullptr) const;
    /// XXX: CF tests
    virtual void updateDecision(const int& slotNum,
                                AlgsExecutionStates& states,
                                std::vector<int>& node_decisions,
                                const AlgorithmNode* requestor = nullptr) const;
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
    /// Output rank, defined as number of output data products
    int m_outputRank;
    /// Representatives (including clones) of the node
    std::vector<IAlgorithm*> m_representatives;
  };

class DataNode {
public:
    /// Constructor
    DataNode(ExecutionFlowGraph& graph, const std::string& path) : m_graph(&graph), m_data_object_path(path) {};
    /// Destructor
    ~DataNode() {};
    const std::string& getPath() {return m_data_object_path;}
    /// Associate an AlgorithmNode, which is a data supplier for this one
    void addProducerNode(AlgorithmNode* node) {
      if (std::find(m_producers.begin(),m_producers.end(),node) == m_producers.end())
        m_producers.push_back(node);
    }
    /// Associate an AlgorithmNode, which is a data consumer of this one
    void addConsumerNode(AlgorithmNode* node) {
      if (std::find(m_consumers.begin(),m_consumers.end(),node) == m_consumers.end())
        m_consumers.push_back(node);
    }
    /// Get all data object producers
    const std::vector<AlgorithmNode*>& getProducers() const {return m_producers;}
    /// Get all data object consumers
    const std::vector<AlgorithmNode*>& getConsumers() const {return m_consumers;}
private:
    ExecutionFlowGraph* m_graph;
    std::string m_data_object_path;
    std::vector<AlgorithmNode*> m_producers;
    std::vector<AlgorithmNode*> m_consumers;
  };

typedef std::unordered_map<std::string,AlgorithmNode*> AlgoNodesMap;
typedef std::unordered_map<std::string,DecisionNode*> DecisionHubsMap;
typedef std::unordered_map<std::string,DataNode*> DataNodesMap;
typedef std::unordered_map<std::string,const DataObjectDescriptorCollection*> AlgoInputsMap;
typedef std::unordered_map<std::string,const DataObjectDescriptorCollection*> AlgoOutputsMap;

class ExecutionFlowManager;
class IExecutionFlowGraph {};

class ExecutionFlowGraph : public CommonMessaging<IExecutionFlowGraph> {
  friend ExecutionFlowManager;
public:
    /// Constructor
    ExecutionFlowGraph(const std::string& name, SmartIF<ISvcLocator> svc) :
     m_headNode(0), m_nodeCounter(0), m_svcLocator(svc), m_name(name), m_initTime(std::chrono::high_resolution_clock::now()),
     m_eventSlots(nullptr) {};
    /// Destructor
    ~ExecutionFlowGraph() {
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
    /// Attach pointers to real Algorithms (and their clones) to Algorithm nodes of the graph
    template<class T>
    void attachAlgorithmsToNodes(const std::string& algo_name, const T& container) {
      auto node = getAlgorithmNode(algo_name);
      for (auto ialgoIt = container.unsafe_begin(); ialgoIt != container.unsafe_end(); ++ialgoIt)
        node->attachAlgorithm(*ialgoIt);
    }
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
    /// Rank Algorithm nodes by the number of data outputs
    void rankAlgorithms(IGraphVisitor& ranker) const;
    /// Print a string representing the control flow state
    void printState(std::stringstream& output,
                    AlgsExecutionStates& states,
                    const std::vector<int>& node_decisions,
                    const unsigned int& recursionLevel) const {m_headNode->printState(output,states,node_decisions,recursionLevel);};
    ///
    const std::vector<AlgorithmNode*> getDataIndependentNodes() const;
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
    /// Print out all data origins and destinations, as reflected in the EF graph
    void dumpDataFlow() const;
    /// dump to file encountered execution plan
    void dumpExecutionPlan();
    /// set cause-effect connection between two algorithms in the execution plan
    void addEdgeToExecutionPlan(const AlgorithmNode* u, const AlgorithmNode* v);

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
    /// temporary items to experiment with execution planning
    boost::ExecPlan m_ExecPlan;
    std::map<std::string,boost::AlgoVertex> m_exec_plan_map;
  };


} // namespace concurrency


#endif
