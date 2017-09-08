#ifndef GAUDIHIVE_CONTROLFLOWGRAPH_H
#define GAUDIHIVE_CONTROLFLOWGRAPH_H

// std includes
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include <fstream>
#include <sstream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphml.hpp>
#include <boost/filesystem.hpp>

// fwk includes
#include "AlgsExecutionStates.h"
#include "IGraphVisitor.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/CommonMessaging.h"

namespace boost {

  struct AlgoProps__ {
    AlgoProps__ () {}
    AlgoProps__ (const std::string& name, int index, int rank, double runtime) :
      m_name(name), m_index(index), m_rank(rank), m_runtime(runtime) {}
    std::string m_name;
    int m_index{-1};
    int m_rank{-1};
    double m_runtime{-1.0};
    int m_eccentricity{-1};
  };

  typedef adjacency_list<vecS, vecS, bidirectionalS, AlgoProps__> ExecPlan__;
  typedef graph_traits<ExecPlan__>::vertex_descriptor AlgoVertex;
}

struct Cause__ {
  enum class source {Root, Task};

  source m_source;
  std::string m_sourceName;
};

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
    DecisionNode(ControlFlowGraph& graph, unsigned int nodeIndex, const std::string& name, bool modeConcurrent, bool modePromptDecision, bool modeOR, bool allPass) :
      ControlFlowNode(graph, nodeIndex, name),
	  m_modeConcurrent(modeConcurrent), m_modePromptDecision(modePromptDecision), m_modeOR(modeOR), m_allPass(allPass), m_children()
      {}
    /// Destructor
    ~DecisionNode() override;
    /// Initialize
    void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map) override;
    /// Method to set algos to CONTROLREADY, if possible
    int updateState(AlgsExecutionStates& states,
                            std::vector<int>& node_decisions) const override;
    /// XXX: CF tests. Method to add a parent node
    void addParentNode(DecisionNode* node);
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
    /// XXX: CF tests. All direct parent nodes in the tree
    std::vector<DecisionNode*> m_parents;
  };

  class DataNode;

  class AlgorithmNode : public ControlFlowNode {
  public:
    /// Constructor
    AlgorithmNode(ControlFlowGraph& graph, Algorithm* algoPtr,
                  unsigned int nodeIndex, unsigned int algoIndex, bool inverted,
                  bool allPass) :
      ControlFlowNode(graph, nodeIndex, algoPtr->name()),
      m_algoIndex(algoIndex),m_algoName(algoPtr->name()),m_inverted(inverted),
      m_allPass(allPass),m_rank(-1),m_algorithm(algoPtr),m_isIOBound(algoPtr->isIOBound())
      {};
    /// Destructor
    ~AlgorithmNode();
    /// Initialize
    void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map) override;
    /// XXX: CF tests. Method to add a parent node
    void addParentNode(DecisionNode* node);

    /// Associate an AlgorithmNode, which is a data supplier for this one
    void addSupplierNode(AlgorithmNode* node) { m_suppliers.push_back(node); }
    /// Associate an AlgorithmNode, which is a data consumer of this one
    void addConsumerNode(AlgorithmNode* node) { m_consumers.push_back(node); }
    /// get Algorithm representatives
    Algorithm* getAlgorithm () const { return m_algorithm; }
    /// Get all supplier nodes
    const std::vector<AlgorithmNode*>& getSupplierNodes() const {return m_suppliers;}
    /// Get all consumer nodes
    const std::vector<AlgorithmNode*>& getConsumerNodes() const {return m_consumers;}
    /// Get all parent decision hubs
    const std::vector<DecisionNode*>& getParentDecisionHubs() const {return m_parents;}

    /// Associate an AlgorithmNode, which is a data supplier for this one
    void addOutputDataNode(DataNode* node);
    /// Associate an AlgorithmNode, which is a data consumer of this one
    void addInputDataNode(DataNode* node);
    /// Get all supplier nodes
    const std::vector<DataNode*>& getOutputDataNodes() const {return m_outputs;}
    /// Get all consumer nodes
    const std::vector<DataNode*>& getInputDataNodes() const {return m_inputs;}
    /// Set Algorithm rank
    void setRank(float& rank) {m_rank = rank;}
    /// Get Algorithm rank
    const float& getRank() const {return m_rank;}

    /// XXX: CF tests
    const unsigned int& getAlgoIndex() const { return m_algoIndex; }
    /// Set the I/O-boundness flag
    void setIOBound(bool value) { m_isIOBound = value;}
    /// Check if algorithm is I/O-bound
    bool isIOBound() const {return m_isIOBound;}
    /// Check if positive control flow decision is enforced
    bool isOptimist() const {return m_allPass;};
    /// Check if control flow logic is always inverted
    bool isLiar() const {return m_inverted;};
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
    /// Algorithm rank of any kind
    float m_rank;
    /// Algorithm representative behind the AlgorithmNode
    Algorithm* m_algorithm;
    /// If an algorithm is I/O-bound (in the broad sense of Von Neumann bottleneck)
    bool m_isIOBound;
  };

class DataNode {
public:
    /// Constructor
    DataNode(ControlFlowGraph& graph, const DataObjID& path): m_graph(&graph), m_data_object_path(path) {}
    /// Destructor
    ~DataNode() {}
    const DataObjID& getPath() {return m_data_object_path;}
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
public:
    ControlFlowGraph* m_graph;
private:
    DataObjID m_data_object_path;
    std::vector<AlgorithmNode*> m_producers;
    std::vector<AlgorithmNode*> m_consumers;
  };

  typedef std::unordered_map<std::string,AlgorithmNode*> AlgoNodesMap;
  typedef std::unordered_map<std::string,DecisionNode*> DecisionHubsMap;
  typedef std::unordered_map<DataObjID,DataNode*,DataObjID_Hasher> DataNodesMap;

  typedef std::unordered_map<std::string, DataObjIDColl > AlgoInputsMap;
  typedef std::unordered_map<std::string, DataObjIDColl > AlgoOutputsMap;


struct IControlFlowGraph {
  virtual ~IControlFlowGraph() = default;
};

class ControlFlowGraph : public CommonMessaging<IControlFlowGraph> {
public:
    /// Constructor
    ControlFlowGraph(const std::string& name, SmartIF<ISvcLocator> svc) :
     m_headNode(0), m_nodeCounter(0), m_algoCounter(0), m_svcLocator(svc), m_name(name),
     m_initTime(std::chrono::system_clock::now()) {}
    /// Destructor
    ~ControlFlowGraph() override {
      if (m_headNode != 0) delete m_headNode;
    }
    /// Initialize graph
    StatusCode initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    /// Register algorithm in the Data Dependency index
    void registerIODataObjects(const Algorithm* algo);
    /// Build data dependency realm WITH data object nodes participating
    StatusCode buildAugmentedDataDependenciesRealm();
    /// Add a node, which has no parents
    void addHeadNode(const std::string& headName, bool modeConcurrent, bool modePromptDecision, bool modeOR, bool allPass);
    /// Get head node
    DecisionNode* getHeadNode() const { return m_headNode; };
    /// Add algorithm node
    StatusCode addAlgorithmNode(Algorithm* daughterAlgo, const std::string& parentName, bool inverted, bool allPass);
    /// Get the AlgorithmNode from by algorithm name using graph index
    AlgorithmNode* getAlgorithmNode(const std::string& algoName) const;
    /// Add DataNode that represents DataObject
    StatusCode addDataNode(const DataObjID& dataPath);
    /// Get DataNode by DataObject path using graph index
    DataNode* getDataNode(const DataObjID& dataPath) const;
    /// Add a node, which aggregates decisions of direct daughter nodes
    StatusCode addDecisionHubNode(Algorithm* daughterAlgo, const std::string& parentName, bool modeConcurrent, bool modePromptDecision, bool modeOR, bool allPass);
    /// Get total number of graph nodes
    unsigned int getControlFlowNodeCounter() const {return m_nodeCounter;}
    /// XXX CF tests. Is needed for older CF implementation
    void updateEventState(AlgsExecutionStates& states,
                          std::vector<int>& node_decisions) const;
    /// A method to update algorithm node decision, and propagate it upwards
    void accept(const std::string& algo_name, IGraphVisitor& visitor) const;
    /// Print a string representing the control flow state
    void printState(std::stringstream& output,
                    AlgsExecutionStates& states,
                    const std::vector<int>& node_decisions,
                    const unsigned int& recursionLevel) const {m_headNode->printState(output,states,node_decisions,recursionLevel);}
    /// Retrieve name of the service
    const std::string& name() const override {return m_name;}
    /// Retrieve pointer to service locator
    SmartIF<ISvcLocator>& serviceLocator() const override {return m_svcLocator;}
    ///
    const std::chrono::system_clock::time_point getInitTime() const {return m_initTime;}
    /// Print out all data origins and destinations, as reflected in the EF graph
    std::string dumpDataFlow() const;
    /// Print out control flow of Algorithms and Sequences
    std::string dumpControlFlow() const;
    /// dump to file encountered execution plan
    void dumpExecutionPlan(const boost::filesystem::path&);
    /// set cause-effect connection between two algorithms in the execution plan
    void addEdgeToExecutionPlan(const AlgorithmNode* u, const AlgorithmNode* v);
    ///
    void dumpControlFlow(std::ostringstream&, ControlFlowNode*, const int&) const;

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
    /// Total number of algorithm nodes in the graph
    unsigned int m_algoCounter;
    /// Service locator (needed to access the MessageSvc)
    mutable SmartIF<ISvcLocator> m_svcLocator;
    const std::string m_name;
    const std::chrono::system_clock::time_point m_initTime;
    /// temporary items to experiment with execution planning
    boost::ExecPlan__ m_ExecPlan;
    std::map<std::string,boost::AlgoVertex> m_exec_plan_map;

  };

  }  // namespace recursive_CF
} // namespace concurrency


#endif
