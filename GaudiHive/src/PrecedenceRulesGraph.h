#ifndef GAUDIHIVE_PRECEDENCERULESGRAPH_H
#define GAUDIHIVE_PRECEDENCERULESGRAPH_H

// std includes
#include <algorithm>
#include <chrono>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphml.hpp>
#include <boost/variant.hpp>

// fwk includes
#include "AlgsExecutionStates.h"
#include "EventSlot.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/ICondSvc.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/ITimelineSvc.h"
#include "GaudiKernel/TaggedBool.h"
#include "IGraphVisitor.h"

namespace concurrency
{
  using Concurrent     = Gaudi::tagged_bool<class Concurrent_tag>;
  using PromptDecision = Gaudi::tagged_bool<class PromptDecision_tag>;
  using ModeOr         = Gaudi::tagged_bool<class ModeOr_tag>;
  using AllPass        = Gaudi::tagged_bool<class AllPass_tag>;
  using Inverted       = Gaudi::tagged_bool<class Inverted_tag>;
}

namespace precedence
{
  using boost::static_visitor;

  // Precedence trace utilities ==============================================
  struct AlgoTraceProps {
    AlgoTraceProps() {}
    AlgoTraceProps( const std::string& name, int index, int rank, double runtime )
        : m_name( name ), m_index( index ), m_rank( rank ), m_runtime( runtime )
    {
    }
    std::string m_name;
    int         m_index{-1};
    int         m_rank{-1};
    int         m_runtime{-1}; // ns
    int         m_eccentricity{-1};
  };

  using PrecTrace       = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, AlgoTraceProps>;
  using AlgoTraceVertex = boost::graph_traits<PrecTrace>::vertex_descriptor;

  // Precedence rules utilities ==============================================
  struct AlgoProps {
    AlgoProps() {}
    AlgoProps( Algorithm* algo, uint nodeIndex, uint algoIndex, bool inverted, bool allPass )
        : m_name( algo->name() )
        , m_nodeIndex( nodeIndex )
        , m_algoIndex( algoIndex )
        , m_algorithm( algo )
        , m_inverted( inverted )
        , m_allPass( allPass )
        , m_isIOBound( algo->isIOBound() )
    {
    }

    std::string m_name{""};
    int         m_nodeIndex{-1};
    int         m_algoIndex{-1};
    int         m_rank{-1};
    /// Algorithm representative behind the AlgorithmNode
    Algorithm* m_algorithm{nullptr};

    /// Whether the selection result is negated or not
    bool m_inverted{false};
    /// Whether the selection result is relevant or always "pass"
    bool m_allPass{false};
    /// If an algorithm is blocking
    bool m_isIOBound{false};
  };

  struct DecisionHubProps {
    DecisionHubProps( const std::string& name, uint nodeIndex, concurrency::Concurrent modeConcurrent,
                      concurrency::PromptDecision modePromptDecision, concurrency::ModeOr modeOR,
                      concurrency::AllPass allPass, concurrency::Inverted isInverted )
        : m_name( name )
        , m_nodeIndex( nodeIndex )
        , m_modeConcurrent( modeConcurrent )
        , m_modePromptDecision( modePromptDecision )
        , m_inverted( isInverted )
        , m_modeOR( modeOR )
        , m_allPass( allPass )
    {
    }

    std::string m_name;
    uint        m_nodeIndex;

    /// Whether all daughters will be evaluated concurrently or sequentially
    bool m_modeConcurrent;
    /// Whether to evaluate the hub decision ASA its child decisions allow to do that.
    /// Applicable to both concurrent and sequential cases.
    bool m_modePromptDecision;
    /// Whether the selection result is negated or not
    bool m_inverted{false};
    /// Whether acting as "and" (false) or "or" node (true)
    bool m_modeOR;
    /// Whether always passing regardless of daughter results
    bool m_allPass;
  };

  struct DataProps {
    DataProps( const DataObjID& id ) : m_id( id ) {}

    DataObjID m_id;
  };

  struct CondDataProps : DataProps {
    CondDataProps( const DataObjID& id ) : DataProps( id ) {}
  };

  // Vertex unpacking ========================================================

  struct VertexName : static_visitor<std::string> {
    std::string operator()( const AlgoProps& props ) const { return props.m_name; }

    std::string operator()( const DecisionHubProps& props ) const { return props.m_name; }

    std::string operator()( const DataProps& props ) const { return props.m_id.fullKey(); }
  };

  struct GroupMode : static_visitor<std::string> {
    std::string operator()( const AlgoProps& ) const { return ""; }

    std::string operator()( const DecisionHubProps& props ) const
    {
      return props.m_modeConcurrent ? "Concurrent" : "Sequential";
    }

    std::string operator()( const DataProps& ) const { return ""; }
  };

  struct GroupLogic : static_visitor<std::string> {
    std::string operator()( const AlgoProps& ) const { return ""; }

    std::string operator()( const DecisionHubProps& props ) const { return props.m_modeOR ? "OR" : "AND"; }

    std::string operator()( const DataProps& ) const { return ""; }
  };

  struct GroupExit : static_visitor<std::string> {
    std::string operator()( const AlgoProps& ) const { return ""; }

    std::string operator()( const DecisionHubProps& props ) const
    {
      return props.m_modePromptDecision ? "Early" : "Late";
    }

    std::string operator()( const DataProps& ) const { return ""; }
  };

  struct DecisionNegation : static_visitor<std::string> {
    std::string operator()( const AlgoProps& props ) const { return props.m_inverted ? "Inverted" : "Non-inverted"; }

    std::string operator()( const DecisionHubProps& ) const { return ""; }

    std::string operator()( const DataProps& ) const { return ""; }
  };

  struct AllPass : static_visitor<std::string> {
    std::string operator()( const AlgoProps& props ) const { return props.m_allPass ? "Optimist" : "Realist"; }

    std::string operator()( const DecisionHubProps& props ) const { return props.m_allPass ? "Optimist" : "Realist"; }

    std::string operator()( const DataProps& ) const { return ""; }
  };

  struct CFDecision : static_visitor<std::string> {
    CFDecision( const EventSlot& slot ) : m_slot( slot ) {}

    std::string operator()( const AlgoProps& props ) const
    {
      return std::to_string( m_slot.controlFlowState.at( props.m_nodeIndex ) );
    }

    std::string operator()( const DecisionHubProps& props ) const
    {
      return std::to_string( m_slot.controlFlowState.at( props.m_nodeIndex ) );
    }

    std::string operator()( const DataProps& ) const { return ""; }

    EventSlot m_slot;
  };

  struct EntityState : static_visitor<std::string> {
    EntityState( const EventSlot& slot, SmartIF<ISvcLocator>& svcLocator, bool conditionsEnabled )
        : m_slot( slot ), m_conditionsEnabled( conditionsEnabled )
    {
      SmartIF<IMessageSvc> msgSvc{svcLocator};
      MsgStream            log{msgSvc, "EntityState.Getter"};

      // Figure if we can discover the data object states
      m_whiteboard = svcLocator->service<IHiveWhiteBoard>( "EventDataSvc", false );
      if ( !m_whiteboard.isValid() ) {
        log << MSG::WARNING << "Failed to locate EventDataSvc: no way to add DO "
            << "states to the TTT dump " << endmsg;
      }

      if ( m_conditionsEnabled ) {
        // Figure if we can discover Condition data object states
        m_condSvc = svcLocator->service<ICondSvc>( "CondSvc", false );
        if ( !m_condSvc.isValid() )
          log << MSG::WARNING << "Failed to locate CondSvc: no way to add Condition DO "
              << "states to the TTT dump " << endmsg;
      }

      if ( !m_slot.eventContext->valid() )
        log << MSG::WARNING << "Event context is invalid: no way to add DO states"
            << " in the TTT dump" << endmsg;
    }

    std::string operator()( const AlgoProps& props ) const
    { // Returns algorithm's FSM state
      std::ostringstream oss;
      oss << m_slot.algsStates[props.m_algoIndex];
      return oss.str();
    }

    std::string operator()( const DecisionHubProps& ) const { return ""; }

    std::string operator()( const DataProps& props ) const
    {
      std::string state;

      if ( m_whiteboard.isValid() && m_slot.eventContext->valid() )
        if ( m_whiteboard->selectStore( m_slot.eventContext->slot() ).isSuccess() )
          state = m_whiteboard->exists( props.m_id ) ? "Produced" : "Missing";

      return state;
    }

    std::string operator()( const CondDataProps& props ) const
    {
      std::string state;

      if ( m_condSvc.isValid() && m_slot.eventContext->valid() )
        state = m_condSvc->isValidID( *( m_slot.eventContext ), props.m_id ) ? "Produced" : "Missing";

      return state;
    }

    EventSlot m_slot;

    SmartIF<IHiveWhiteBoard> m_whiteboard;
    SmartIF<ICondSvc>        m_condSvc;
    bool                     m_conditionsEnabled{false};
  };

  struct StartTime : static_visitor<std::string> {
    StartTime( const EventSlot& slot, SmartIF<ISvcLocator>& svcLocator ) : m_slot( slot )
    {
      SmartIF<IMessageSvc> msgSvc{svcLocator};
      MsgStream            log{msgSvc, "StartTime.Getter"};

      // Figure if we can discover the algorithm timings
      m_timelineSvc = svcLocator->service<ITimelineSvc>( "TimelineSvc", false );
      if ( !m_timelineSvc.isValid() ) {
        log << MSG::WARNING << "Failed to locate the TimelineSvc: no way to "
            << "add algorithm start time to the TTT dumps" << endmsg;
      }
    }

    std::string operator()( const AlgoProps& props ) const
    {

      std::string startTime;

      if ( m_timelineSvc.isValid() ) {

        TimelineEvent te{};
        te.algorithm = props.m_name;
        te.slot      = m_slot.eventContext->slot();
        te.event     = m_slot.eventContext->evt();

        m_timelineSvc->getTimelineEvent( te );
        startTime = std::to_string(
            std::chrono::duration_cast<std::chrono::nanoseconds>( te.start.time_since_epoch() ).count() );
      }

      return startTime;
    }

    std::string operator()( const DecisionHubProps& ) const { return ""; }

    std::string operator()( const DataProps& ) const { return ""; }

    EventSlot             m_slot;
    SmartIF<ITimelineSvc> m_timelineSvc;
  };

  struct EndTime : static_visitor<std::string> {
    EndTime( const EventSlot& slot, SmartIF<ISvcLocator>& svcLocator ) : m_slot( slot )
    {
      SmartIF<IMessageSvc> msgSvc{svcLocator};
      MsgStream            log{msgSvc, "EndTime.Getter"};

      // Figure if we can discover the algorithm timings
      m_timelineSvc = svcLocator->service<ITimelineSvc>( "TimelineSvc", false );
      if ( !m_timelineSvc.isValid() )
        log << MSG::WARNING << "Failed to locate the TimelineSvc: no way to add "
            << "algorithm completion time to the TTT dumps" << endmsg;
    }

    std::string operator()( const AlgoProps& props ) const
    {

      std::string endTime;

      if ( m_timelineSvc.isValid() ) {

        TimelineEvent te{};
        te.algorithm = props.m_name;
        te.slot      = m_slot.eventContext->slot();
        te.event     = m_slot.eventContext->evt();

        m_timelineSvc->getTimelineEvent( te );
        endTime =
            std::to_string( std::chrono::duration_cast<std::chrono::nanoseconds>( te.end.time_since_epoch() ).count() );
      }

      return endTime;
    }

    std::string operator()( const DecisionHubProps& ) const { return ""; }

    std::string operator()( const DataProps& ) const { return ""; }

    EventSlot             m_slot;
    SmartIF<ITimelineSvc> m_timelineSvc;
  };

  struct Duration : static_visitor<std::string> {
    Duration( const EventSlot& slot, SmartIF<ISvcLocator>& svcLocator ) : m_slot( slot )
    {
      SmartIF<IMessageSvc> msgSvc{svcLocator};
      MsgStream            log{msgSvc, "Duration.Getter"};

      // Figure if we can discover the algorithm timings
      m_timelineSvc = svcLocator->service<ITimelineSvc>( "TimelineSvc", false );
      if ( !m_timelineSvc.isValid() )
        log << MSG::WARNING << "Failed to locate the TimelineSvc: no way to add "
            << "algorithm's runtimes to the TTT dumps" << endmsg;
    }

    std::string operator()( const AlgoProps& props ) const
    {

      std::string time;

      if ( m_timelineSvc.isValid() ) {

        TimelineEvent te;
        te.algorithm = props.m_name;
        te.slot      = m_slot.eventContext->slot();
        te.event     = m_slot.eventContext->evt();

        m_timelineSvc->getTimelineEvent( te );
        time = std::to_string( std::chrono::duration_cast<std::chrono::nanoseconds>( te.end - te.start ).count() );
      }

      return time;
    }

    std::string operator()( const DecisionHubProps& ) const { return ""; }

    std::string operator()( const DataProps& ) const { return ""; }

    EventSlot             m_slot;
    SmartIF<ITimelineSvc> m_timelineSvc;
  };

  struct Operations : static_visitor<std::string> {
    std::string operator()( const AlgoProps& props ) const { return props.m_isIOBound ? "IO-bound" : "CPU-bound"; }

    std::string operator()( const DecisionHubProps& ) const { return ""; }

    std::string operator()( const DataProps& ) const { return ""; }
  };

  static inline std::ostream& operator<<( std::ostream& os, const AlgoProps& ) { return os << "Algorithm"; }
  static inline std::ostream& operator<<( std::ostream& os, const DecisionHubProps& ) { return os << "DecisionHub"; }
  static inline std::ostream& operator<<( std::ostream& os, const DataProps& ) { return os << "Data"; }
  static inline std::ostream& operator<<( std::ostream& os, const CondDataProps& ) { return os << "ConditionData"; }

  //=========================================================================

  using VariantVertexProps = boost::variant<AlgoProps, DecisionHubProps, DataProps, CondDataProps>;
  using PRGraph            = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VariantVertexProps>;
  using PRVertexDesc       = boost::graph_traits<PRGraph>::vertex_descriptor;

} // namespace prules

struct Cause {
  enum class source { Root, Task };

  source      m_source;
  std::string m_sourceName;
};

namespace concurrency
{
  class PrecedenceRulesGraph;

  using precedence::PRVertexDesc;
  using precedence::AlgoProps;
  using precedence::DecisionHubProps;
  using precedence::DataProps;
  using precedence::CondDataProps;
  using precedence::VariantVertexProps;

  // ==========================================================================
  class ControlFlowNode
  {
  public:
    /// Constructor
    ControlFlowNode( PrecedenceRulesGraph& graph, unsigned int nodeIndex, const std::string& name )
        : m_graph( &graph ), m_nodeIndex( nodeIndex ), m_nodeName( name )
    {
    }
    /// Destructor
    virtual ~ControlFlowNode() {}

    /// Visitor entry point
    virtual bool accept( IGraphVisitor& visitor ) = 0;
    /// Print a string representing the control flow state
    virtual void printState( std::stringstream& output, AlgsExecutionStates& states,
                             const std::vector<int>& node_decisions, const unsigned int& recursionLevel ) const = 0;
    /// Get node index
    const unsigned int& getNodeIndex() const { return m_nodeIndex; }
    /// Get node name
    const std::string& getNodeName() const { return m_nodeName; }

  public:
    PrecedenceRulesGraph* m_graph;

  protected:
    /// Translation between state id and name
    std::string stateToString( const int& stateId ) const;
    unsigned int m_nodeIndex;
    std::string  m_nodeName;
  };

  class DecisionNode : public ControlFlowNode
  {
  public:
    /// Constructor
    DecisionNode( PrecedenceRulesGraph& graph, unsigned int nodeIndex, const std::string& name,
                  Concurrent modeConcurrent, PromptDecision modePromptDecision, ModeOr modeOR, AllPass allPass,
                  Inverted isInverted )
        : ControlFlowNode( graph, nodeIndex, name )
        , m_modeConcurrent( modeConcurrent )
        , m_modePromptDecision( modePromptDecision )
        , m_modeOR( modeOR )
        , m_allPass( allPass )
        , m_inverted( isInverted )
        , m_children()
    {
    }

    /// Destructor
    ~DecisionNode() override;

    /// Visitor entry point
    bool accept( IGraphVisitor& visitor ) override;
    /// Add a parent node
    void addParentNode( DecisionNode* node );
    /// Add a daughter node
    void addDaughterNode( ControlFlowNode* node );
    /// Get children nodes
    const std::vector<ControlFlowNode*>& getDaughters() const { return m_children; }
    /// Print a string representing the control flow state
    void printState( std::stringstream& output, AlgsExecutionStates& states, const std::vector<int>& node_decisions,
                     const unsigned int& recursionLevel ) const override;

  public:
    /// Whether all daughters will be evaluated concurrently or sequentially
    bool m_modeConcurrent;
    /// Whether to evaluate the hub decision ASA its child decisions allow to do that.
    /// Applicable to both concurrent and sequential cases.
    bool m_modePromptDecision;
    /// Whether acting as "and" (false) or "or" node (true)
    bool m_modeOR;
    /// Whether always passing regardless of daughter results
    bool m_allPass;
    /// Whether the selection result is negated or not
    bool m_inverted{false};
    /// All direct daughter nodes in the tree
    std::vector<ControlFlowNode*> m_children;
    /// Direct parent nodes
    std::vector<DecisionNode*> m_parents;
  };

  // ==========================================================================
  class DataNode;

  class AlgorithmNode : public ControlFlowNode
  {
  public:
    /// Constructor
    AlgorithmNode( PrecedenceRulesGraph& graph, Algorithm* algoPtr, unsigned int nodeIndex, unsigned int algoIndex,
                   bool inverted, bool allPass )
        : ControlFlowNode( graph, nodeIndex, algoPtr->name() )
        , m_algorithm( algoPtr )
        , m_algoIndex( algoIndex )
        , m_algoName( algoPtr->name() )
        , m_inverted( inverted )
        , m_allPass( allPass )
        , m_rank( -1 )
        , m_isIOBound( algoPtr->isIOBound() ){};
    /// Destructor
    ~AlgorithmNode();

    /// Visitor entry point
    bool accept( IGraphVisitor& visitor ) override;

    /// Add a parent node
    void addParentNode( DecisionNode* node );
    /// Get all parent decision hubs
    const std::vector<DecisionNode*>& getParentDecisionHubs() const { return m_parents; }

    /// Associate an AlgorithmNode, which is a data supplier for this one
    void addOutputDataNode( DataNode* node );
    /// Associate an AlgorithmNode, which is a data consumer of this one
    void addInputDataNode( DataNode* node );
    /// Get all supplier nodes
    const std::vector<DataNode*>& getOutputDataNodes() const { return m_outputs; }
    /// Get all consumer nodes
    const std::vector<DataNode*>& getInputDataNodes() const { return m_inputs; }

    /// Set Algorithm rank
    void setRank( float& rank ) { m_rank = rank; }
    /// Get Algorithm rank
    const float& getRank() const { return m_rank; }

    /// get Algorithm representatives
    Algorithm* getAlgorithm() const { return m_algorithm; }
    /// Get algorithm index
    const unsigned int& getAlgoIndex() const { return m_algoIndex; }

    /// Set the I/O-boundness flag
    void setIOBound( bool value ) { m_isIOBound = value; }
    /// Check if algorithm is I/O-bound
    bool isIOBound() const { return m_isIOBound; }

    /// Check if positive control flow decision is enforced
    bool isOptimist() const { return m_allPass; };
    /// Check if control flow logic is always inverted
    bool isLiar() const { return m_inverted; };

    /// Print a string representing the control flow state
    void printState( std::stringstream& output, AlgsExecutionStates& states, const std::vector<int>& node_decisions,
                     const unsigned int& recursionLevel ) const override;

  public:
    /// Control flow parents of an AlgorithmNode (DecisionNodes)
    std::vector<DecisionNode*> m_parents;

  private:
    /// Algorithm representative behind the AlgorithmNode
    Algorithm* m_algorithm;
    /// The index of the algorithm
    unsigned int m_algoIndex;
    /// The name of the algorithm
    std::string m_algoName;
    /// Whether the selection result is negated or not
    bool m_inverted;
    /// Whether the selection result is relevant or always "pass"
    bool m_allPass;
    /// Algorithm rank of any kind
    float m_rank;
    /// If an algorithm is blocking
    bool m_isIOBound;

    /// Algorithm outputs (DataNodes)
    std::vector<DataNode*> m_outputs;
    /// Algorithm inputs (DataNodes)
    std::vector<DataNode*> m_inputs;
  };

  // ==========================================================================
  class DataNode
  {
  public:
    /// Constructor
    DataNode( PrecedenceRulesGraph& graph, const DataObjID& path ) : m_graph( &graph ), m_data_object_path( path ) {}

    /// Destructor
    virtual ~DataNode() = default;

    const DataObjID& getPath() { return m_data_object_path; }

    /// Entry point for a visitor
    virtual bool accept( IGraphVisitor& visitor )
    {
      if ( visitor.visitEnter( *this ) ) return visitor.visit( *this );
      return true;
    }
    /// Add relationship to producer AlgorithmNode
    void addProducerNode( AlgorithmNode* node )
    {
      if ( std::find( m_producers.begin(), m_producers.end(), node ) == m_producers.end() )
        m_producers.push_back( node );
    }
    /// Add relationship to consumer AlgorithmNode
    void addConsumerNode( AlgorithmNode* node )
    {
      if ( std::find( m_consumers.begin(), m_consumers.end(), node ) == m_consumers.end() )
        m_consumers.push_back( node );
    }
    /// Get all data object producers
    const std::vector<AlgorithmNode*>& getProducers() const { return m_producers; }
    /// Get all data object consumers
    const std::vector<AlgorithmNode*>& getConsumers() const { return m_consumers; }

  public:
    PrecedenceRulesGraph* m_graph;

  private:
    DataObjID                   m_data_object_path;
    std::vector<AlgorithmNode*> m_producers;
    std::vector<AlgorithmNode*> m_consumers;
  };

  class ConditionNode : public DataNode
  {
  public:
    /// Constructor
    ConditionNode( PrecedenceRulesGraph& graph, const DataObjID& path, SmartIF<ICondSvc> condSvc )
        : DataNode( graph, path ), m_condSvc( condSvc )
    {
    }

    /// Need to hide the (identical) base method with this one so that
    /// visitEnter(ConditionNode&) and visit(ConditionNode&) are called.
    /// using DataNode::accept; ?
    bool accept( IGraphVisitor& visitor ) override
    {
      if ( visitor.visitEnter( *this ) ) return visitor.visit( *this );
      return true;
    }

  public:
    // Service for Conditions handling
    SmartIF<ICondSvc> m_condSvc;
  };

  // ==========================================================================
  using AlgoNodesMap    = std::unordered_map<std::string, AlgorithmNode*>;
  using DecisionHubsMap = std::unordered_map<std::string, DecisionNode*>;
  using DataNodesMap    = std::unordered_map<DataObjID, DataNode*, DataObjID_Hasher>;

  using AlgoInputsMap  = std::unordered_map<std::string, DataObjIDColl>;
  using AlgoOutputsMap = std::unordered_map<std::string, DataObjIDColl>;

  struct IPrecedenceRulesGraph {
    virtual ~IPrecedenceRulesGraph() = default;
  };

  class PrecedenceRulesGraph : public CommonMessaging<IPrecedenceRulesGraph>
  {
  public:
    /// Constructor
    PrecedenceRulesGraph( const std::string& name, SmartIF<ISvcLocator> svc )
        : m_headNode( 0 )
        , m_nodeCounter( 0 )
        , m_algoCounter( 0 )
        , m_svcLocator( svc )
        , m_name( name )
        , m_initTime( std::chrono::system_clock::now() )
    {
      // make sure that CommonMessaging is initialized
      setUpMessaging();
    }
    /// Destructor
    ~PrecedenceRulesGraph() override
    {
      if ( m_headNode != 0 ) delete m_headNode;
    }

    /// Initialize graph
    StatusCode initialize();

    /// An entry point to visit all graph nodes
    void accept( IGraphVisitor& visitor ) const;

    /// Add DataNode that represents DataObject
    StatusCode addDataNode( const DataObjID& dataPath );
    /// Get DataNode by DataObject path using graph index
    DataNode* getDataNode( const DataObjID& dataPath ) const;
    /// Register algorithm in the Data Dependency index
    void registerIODataObjects( const Algorithm* algo );
    /// Build data dependency realm WITH data object nodes participating
    StatusCode buildDataDependenciesRealm();

    /// Add a node, which has no parents
    void addHeadNode( const std::string& headName, concurrency::Concurrent, concurrency::PromptDecision,
                      concurrency::ModeOr, concurrency::AllPass, concurrency::Inverted );
    /// Get head node
    DecisionNode* getHeadNode() const { return m_headNode; };
    /// Add algorithm node
    StatusCode addAlgorithmNode( Algorithm* daughterAlgo, const std::string& parentName, bool inverted, bool allPass );
    /// Get the AlgorithmNode from by algorithm name using graph index
    AlgorithmNode* getAlgorithmNode( const std::string& algoName ) const;
    /// Add a node, which aggregates decisions of direct daughter nodes
    StatusCode addDecisionHubNode( Algorithm* daughterAlgo, const std::string& parentName, concurrency::Concurrent,
                                   concurrency::PromptDecision, concurrency::ModeOr, concurrency::AllPass,
                                   concurrency::Inverted );
    /// Get total number of control flow graph nodes
    unsigned int getControlFlowNodeCounter() const { return m_nodeCounter; }

    /// Rank Algorithm nodes by the number of data outputs
    void rankAlgorithms( IGraphVisitor& ranker ) const;

    /// Retrieve name of the service
    const std::string& name() const override { return m_name; }
    /// Retrieve pointer to service locator
    SmartIF<ISvcLocator>& serviceLocator() const override { return m_svcLocator; }
    ///
    const std::chrono::system_clock::time_point getInitTime() const { return m_initTime; }

    /// Print a string representing the control flow state
    void printState( std::stringstream& output, AlgsExecutionStates& states, const std::vector<int>& node_decisions,
                     const unsigned int& recursionLevel ) const
    {
      m_headNode->printState( output, states, node_decisions, recursionLevel );
    }

    /// BGL-based facilities
    void         enableAnalysis() { m_enableAnalysis = true; };
    PRVertexDesc node( const std::string& ) const;

    /// Print out all data origins and destinations, as reflected in the EF graph
    std::string dumpDataFlow() const;
    /// Print out control flow of Algorithms and Sequences
    std::string dumpControlFlow() const;
    /// dump to file the precedence rules
    void dumpPrecRules( const boost::filesystem::path&, const EventSlot& slot );
    /// dump to file the precedence trace
    void dumpPrecTrace( const boost::filesystem::path& );
    /// set cause-effect connection between two algorithms in the precedence trace
    void addEdgeToPrecTrace( const AlgorithmNode* u, const AlgorithmNode* v );
    ///
    void dumpControlFlow( std::ostringstream&, ControlFlowNode*, const int& ) const;

  private:
    /// the head node of the control flow graph
    DecisionNode* m_headNode;
    /// Index: map of algorithm's name to AlgorithmNode
    AlgoNodesMap m_algoNameToAlgoNodeMap;
    /// Index: map of decision's name to DecisionHub
    DecisionHubsMap m_decisionNameToDecisionHubMap;
    /// Index: map of data path to DataNode
    DataNodesMap m_dataPathToDataNodeMap;
    /// Indexes: maps of algorithm's name to algorithm's inputs/outputs
    AlgoInputsMap  m_algoNameToAlgoInputsMap;
    AlgoOutputsMap m_algoNameToAlgoOutputsMap;

    /// Total number of nodes in the graph
    unsigned int m_nodeCounter;
    /// Total number of algorithm nodes in the graph
    unsigned int m_algoCounter;

    /// Service locator (needed to access the MessageSvc)
    mutable SmartIF<ISvcLocator> m_svcLocator;
    const std::string            m_name;

    const std::chrono::system_clock::time_point m_initTime;

    /// facilities for algorithm precedence tracing
    precedence::PrecTrace m_precTrace;
    std::map<std::string, precedence::AlgoTraceVertex> m_prec_trace_map;
    bool m_enableAnalysis{false};
    /// BGL-based graph of precedence rules
    precedence::PRGraph m_PRGraph;

    /// Enable conditions realm of precedence rules
    bool m_conditionsRealmEnabled{false};
  };

} // namespace concurrency
#endif
