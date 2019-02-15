#include "PrecedenceRulesGraph.h"
#include "Visitors/Promoters.h"

#include "GaudiKernel/DataHandleHolderVisitor.h"

#include <boost/property_map/transform_value_property_map.hpp>
#include <fstream>

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

namespace {
  //---------------------------------------------------------------------------
  /// Translation between state id and name
  const char* stateToString( const int& stateId ) {
    switch ( stateId ) {
    case 0:
      return "FALSE";
    case 1:
      return "TRUE";
    default:
      return "UNDEFINED";
    }
  }
} // namespace

namespace concurrency {

  //---------------------------------------------------------------------------
  void DecisionNode::addParentNode( DecisionNode* node ) {

    if ( std::find( m_parents.begin(), m_parents.end(), node ) == m_parents.end() ) m_parents.push_back( node );
  }

  //--------------------------------------------------------------------------
  void DecisionNode::addDaughterNode( ControlFlowNode* node ) {

    if ( std::find( m_children.begin(), m_children.end(), node ) == m_children.end() ) m_children.push_back( node );
  }

  //---------------------------------------------------------------------------
  void DecisionNode::printState( std::stringstream& output, AlgsExecutionStates& states,
                                 const std::vector<int>& node_decisions, const unsigned int& recursionLevel ) const {

    output << std::string( recursionLevel, ' ' ) << m_nodeName << " (" << m_nodeIndex << ")"
           << ", w/ decision: " << stateToString( node_decisions[m_nodeIndex] ) << "(" << node_decisions[m_nodeIndex]
           << ")" << std::endl;

    for ( auto daughter : m_children ) daughter->printState( output, states, node_decisions, recursionLevel + 2 );
  }

  //---------------------------------------------------------------------------
  bool DecisionNode::accept( IGraphVisitor& visitor ) {

    if ( visitor.visitEnter( *this ) ) {
      // try to aggregate a decision
      bool result = visitor.visit( *this );
      return !result;
    }

    return false; // visitor was rejected (since the decision node has an aggregated decision already)
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::printState( std::stringstream& output, AlgsExecutionStates& states,
                                  const std::vector<int>& node_decisions, const unsigned int& recursionLevel ) const {
    output << std::string( recursionLevel, ' ' ) << m_nodeName << " (" << m_nodeIndex << ")"
           << ", w/ decision: " << stateToString( node_decisions[m_nodeIndex] ) << "(" << node_decisions[m_nodeIndex]
           << ")"
           << ", in state: " << states[m_algoIndex] << std::endl;
  }

  //---------------------------------------------------------------------------
  bool AlgorithmNode::accept( IGraphVisitor& visitor ) {

    if ( visitor.visitEnter( *this ) ) {
      visitor.visit( *this );
      return true; // visitor was accepted to promote the algorithm
    }

    return false; // visitor was rejected (since the algorithm already produced a decision)
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::addParentNode( DecisionNode* node ) {

    if ( std::find( m_parents.begin(), m_parents.end(), node ) == m_parents.end() ) m_parents.push_back( node );
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::addOutputDataNode( DataNode* node ) {

    if ( std::find( m_outputs.begin(), m_outputs.end(), node ) == m_outputs.end() ) m_outputs.push_back( node );
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::addInputDataNode( DataNode* node ) {

    if ( std::find( m_inputs.begin(), m_inputs.end(), node ) == m_inputs.end() ) m_inputs.push_back( node );
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::initialize() {
    if ( serviceLocator()->existsService( "CondSvc" ) ) {
      SmartIF<ICondSvc> condSvc{serviceLocator()->service( "CondSvc" )};
      if ( condSvc.isValid() ) {
        info() << "CondSvc found. DF precedence rules will be augmented with 'Conditions'" << endmsg;
        m_conditionsRealmEnabled = true;
      }
    }

    // Detach condition algorithms from the CF realm
    if ( m_conditionsRealmEnabled ) {
      SmartIF<ICondSvc> condSvc{serviceLocator()->service( "CondSvc", false )};
      auto&             condAlgs = condSvc->condAlgs();
      for ( const auto algo : condAlgs ) {
        auto itA = m_algoNameToAlgoNodeMap.find( algo->name() );
        if ( itA != m_algoNameToAlgoNodeMap.end() ) {
          concurrency::AlgorithmNode* algoNode = itA->second.get();
          debug() << "Detaching condition algorithm '" << algo->name() << "' from the CF realm.." << endmsg;
          for ( auto parent : algoNode->getParentDecisionHubs() ) {
            parent->m_children.erase( std::remove( parent->m_children.begin(), parent->m_children.end(), algoNode ),
                                      parent->m_children.end() );
            // clean up also auxiliary BGL-based graph of precedence rules
            if ( m_enableAnalysis )
              boost::remove_edge( node( algoNode->getNodeName() ), node( parent->getNodeName() ), m_PRGraph );
          }
          algoNode->m_parents.clear();

        } else {
          warning() << "Algorithm '" << algo->name() << "' is not registered in the graph" << endmsg;
        }
      }
    }

    StatusCode sc = buildDataDependenciesRealm();

    if ( !sc.isSuccess() ) error() << "Could not build the data dependency realm." << endmsg;

    ON_DEBUG debug() << dumpDataFlow() << endmsg;

    return sc;
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::registerIODataObjects( const Gaudi::Algorithm* algo ) {

    const std::string& algoName = algo->name();

    m_algoNameToAlgoInputsMap[algoName]  = algo->inputDataObjs();
    m_algoNameToAlgoOutputsMap[algoName] = algo->outputDataObjs();

    ON_VERBOSE {
      verbose() << "    Inputs of " << algoName << ": ";
      for ( auto tag : algo->inputDataObjs() ) verbose() << tag << " | ";
      verbose() << endmsg;

      verbose() << "    Outputs of " << algoName << ": ";
      for ( auto tag : algo->outputDataObjs() ) verbose() << tag << " | ";
      verbose() << endmsg;
    }
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::buildDataDependenciesRealm() {

    StatusCode global_sc( StatusCode::SUCCESS, true );

    // Production of DataNodes by AlgorithmNodes (DataNodes are created here)
    for ( auto& algo : m_algoNameToAlgoNodeMap ) {

      auto& outputs = m_algoNameToAlgoOutputsMap[algo.first];
      for ( auto output : outputs ) {
        const auto sc = addDataNode( output );
        if ( !sc.isSuccess() ) {
          error() << "Extra producer (" << algo.first << ") for DataObject @ " << output
                  << " has been detected: this is not allowed." << endmsg;
          global_sc = sc;
        }
        auto dataNode = getDataNode( output );
        dataNode->addProducerNode( algo.second.get() );
        algo.second->addOutputDataNode( dataNode );

        // Mirror the action above in the BGL-based graph
        if ( m_enableAnalysis )
          boost::add_edge( node( algo.second->getNodeName() ), node( output.fullKey() ), m_PRGraph );
      }
    }

    // Consumption of DataNodes by AlgorithmNodes
    for ( auto& algo : m_algoNameToAlgoNodeMap ) {

      for ( auto input : m_algoNameToAlgoInputsMap[algo.first] ) {

        auto itP = m_dataPathToDataNodeMap.find( input );

        DataNode* dataNode = ( itP != m_dataPathToDataNodeMap.end() ? getDataNode( input ) : nullptr );
        if ( dataNode ) {
          dataNode->addConsumerNode( algo.second.get() );
          algo.second->addInputDataNode( dataNode );

          // Mirror the action above in the BGL-based graph
          if ( m_enableAnalysis )
            boost::add_edge( node( input.fullKey() ), node( algo.second->getNodeName() ), m_PRGraph );
        }
      }
    }

    return global_sc;
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::addAlgorithmNode( Gaudi::Algorithm* algo, const std::string& parentName,
                                                     bool inverted, bool allPass ) {

    StatusCode sc = StatusCode::SUCCESS;

    /// Create new, or fetch existent, AlgorithmNode

    auto& algoName = algo->name();

    concurrency::AlgorithmNode* algoNode;

    auto itA = m_algoNameToAlgoNodeMap.find( algoName );
    if ( itA != m_algoNameToAlgoNodeMap.end() ) {
      algoNode = itA->second.get();
    } else {
      auto r = m_algoNameToAlgoNodeMap.emplace(
          algoName, std::make_unique<concurrency::AlgorithmNode>( *this, algo, m_nodeCounter, m_algoCounter, inverted,
                                                                  allPass ) );
      algoNode = r.first->second.get();

      // Mirror AlgorithmNode in the BGL-based graph
      if ( m_enableAnalysis ) {
        boost::add_vertex( AlgoProps( algo, m_nodeCounter, m_algoCounter, inverted, allPass ), m_PRGraph );
      }
      ++m_nodeCounter;
      ++m_algoCounter;
      ON_VERBOSE verbose() << "AlgorithmNode '" << algoName << "' added @ " << algoNode << endmsg;

      registerIODataObjects( algo );
    }

    /// Attach AlgorithmNode to its parent DecisionNode
    auto itP = m_decisionNameToDecisionHubMap.find( parentName );
    if ( itP != m_decisionNameToDecisionHubMap.end() ) {
      auto parentNode = itP->second.get();

      parentNode->addDaughterNode( algoNode );
      algoNode->addParentNode( parentNode );

      // Mirror algorithm to CF parent relationship in the BGL-based graph
      if ( m_enableAnalysis ) boost::add_edge( node( algo->name() ), node( parentName ), m_PRGraph );

      ON_VERBOSE verbose() << "Attached AlgorithmNode '" << algo->name() << "' to parent DecisionNode '" << parentName
                           << "'" << endmsg;
    } else {
      sc = StatusCode::FAILURE;
      error() << "Parent DecisionNode '" << parentName << "' was not found" << endmsg;
    }

    return sc;
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::addDataNode( const DataObjID& dataPath ) {

    auto itD = m_dataPathToDataNodeMap.find( dataPath );
    if ( itD != m_dataPathToDataNodeMap.end() ) return StatusCode::SUCCESS;

    std::unique_ptr<concurrency::DataNode> dataNode;
    if ( !m_conditionsRealmEnabled ) {
      dataNode = std::make_unique<concurrency::DataNode>( *this, dataPath );
      ON_VERBOSE verbose() << "  DataNode " << dataPath << " added @ " << dataNode.get() << endmsg;
      // Mirror the action above in the BGL-based graph
      if ( m_enableAnalysis ) boost::add_vertex( DataProps( dataPath ), m_PRGraph );
    } else {
      SmartIF<ICondSvc> condSvc{serviceLocator()->service( "CondSvc", false )};
      if ( condSvc->isRegistered( dataPath ) ) {
        dataNode = std::make_unique<concurrency::ConditionNode>( *this, dataPath, condSvc );
        ON_VERBOSE verbose() << "  ConditionNode " << dataPath << " added @ " << dataNode.get() << endmsg;
        // Mirror the action above in the BGL-based graph
        if ( m_enableAnalysis ) boost::add_vertex( CondDataProps( dataPath ), m_PRGraph );
      } else {
        dataNode = std::make_unique<concurrency::DataNode>( *this, dataPath );
        ON_VERBOSE verbose() << "  DataNode " << dataPath << " added @ " << dataNode.get() << endmsg;
        // Mirror the action above in the BGL-based graph
        if ( m_enableAnalysis ) boost::add_vertex( DataProps( dataPath ), m_PRGraph );
      }
    }
    m_dataPathToDataNodeMap.emplace( dataPath, std::move( dataNode ) );
    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::addDecisionHubNode( Gaudi::Algorithm* decisionHubAlgo, const std::string& parentName,
                                                       Concurrent modeConcurrent, PromptDecision modePromptDecision,
                                                       ModeOr modeOR, AllPass allPass, Inverted isInverted ) {

    StatusCode sc = StatusCode::SUCCESS;

    /// Create new, or fetch existent, DecisionNode

    auto& decisionHubName = decisionHubAlgo->name();

    auto                       itA = m_decisionNameToDecisionHubMap.find( decisionHubName );
    concurrency::DecisionNode* decisionHubNode;
    if ( itA != m_decisionNameToDecisionHubMap.end() ) {
      decisionHubNode = itA->second.get();
    } else {
      auto r = m_decisionNameToDecisionHubMap.emplace(
          decisionHubName,
          std::make_unique<concurrency::DecisionNode>( *this, m_nodeCounter, decisionHubName, modeConcurrent,
                                                       modePromptDecision, modeOR, allPass, isInverted ) );
      decisionHubNode = r.first->second.get();
      // Mirror DecisionNode in the BGL-based graph
      if ( m_enableAnalysis ) {
        boost::add_vertex( DecisionHubProps( decisionHubName, m_nodeCounter, modeConcurrent, modePromptDecision, modeOR,
                                             allPass, isInverted ),
                           m_PRGraph );
      }

      ++m_nodeCounter;

      ON_VERBOSE verbose() << "DecisionNode '" << decisionHubName << "' added @ " << decisionHubNode << endmsg;
    }

    /// Attach DecisionNode to its parent DecisionNode
    auto itP = m_decisionNameToDecisionHubMap.find( parentName );
    if ( itP != m_decisionNameToDecisionHubMap.end() ) {
      auto parentNode = itP->second.get();
      parentNode->addDaughterNode( decisionHubNode );
      decisionHubNode->addParentNode( parentNode );

      // Mirror DecisionNode-to-DecisionNode relationship in the BGL-based graph
      if ( m_enableAnalysis ) boost::add_edge( node( decisionHubName ), node( parentName ), m_PRGraph );

      ON_VERBOSE verbose() << "Attached DecisionNode '" << decisionHubName << "' to parent DecisionNode '" << parentName
                           << "'" << endmsg;
    } else {
      sc = StatusCode::FAILURE;
      error() << "Parent DecisionNode '" << parentName << "' was not found" << endmsg;
    }

    return sc;
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::addHeadNode( const std::string& headName, concurrency::Concurrent modeConcurrent,
                                          concurrency::PromptDecision modePromptDecision, concurrency::ModeOr modeOR,
                                          concurrency::AllPass allPass, concurrency::Inverted isInverted ) {

    auto itH = m_decisionNameToDecisionHubMap.find( headName );
    if ( itH != m_decisionNameToDecisionHubMap.end() ) {
      m_headNode = itH->second.get();
    } else {
      auto r = m_decisionNameToDecisionHubMap.emplace(
          headName, std::make_unique<concurrency::DecisionNode>( *this, m_nodeCounter, headName, modeConcurrent,
                                                                 modePromptDecision, modeOR, allPass, isInverted ) );
      m_headNode = r.first->second.get();

      // Mirror the action above in the BGL-based graph
      if ( m_enableAnalysis ) {
        boost::add_vertex( DecisionHubProps( headName, m_nodeCounter, modeConcurrent, modePromptDecision, modeOR,
                                             allPass, isInverted ),
                           m_PRGraph );
      }

      ++m_nodeCounter;
    }
  }

  //---------------------------------------------------------------------------
  PRVertexDesc PrecedenceRulesGraph::node( const std::string& name ) const {
    auto vp = vertices( m_PRGraph );
    auto i  = std::find_if( vp.first, vp.second, [&]( const PRVertexDesc& v ) {
      return boost::apply_visitor( precedence::VertexName(), m_PRGraph[v] ) == name;
    } );
    return i != vp.second ? *i : PRVertexDesc{};
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::accept( IGraphVisitor& visitor ) const {
    // iterate through Algorithm nodes
    for ( auto& pr : m_algoNameToAlgoNodeMap ) pr.second->accept( visitor );

    // iterate through DecisionHub nodes
    for ( auto& pr : m_decisionNameToDecisionHubMap ) pr.second->accept( visitor );

    // iterate through Data [and Conditions] nodes
    for ( auto& pr : m_dataPathToDataNodeMap ) pr.second->accept( visitor );
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::rankAlgorithms( IGraphVisitor& ranker ) const {

    info() << "Starting ranking by data outputs .. " << endmsg;
    for ( auto& pair : m_algoNameToAlgoNodeMap ) {
      ON_DEBUG debug() << "  Ranking " << pair.first << "... " << endmsg;
      pair.second->accept( ranker );
      ON_DEBUG debug() << "  ... rank of " << pair.first << ": " << pair.second->getRank() << endmsg;
    }
  }

  std::string PrecedenceRulesGraph::dumpControlFlow() const {
    std::ostringstream ost;
    dumpControlFlow( ost, m_headNode, 0 );
    return ost.str();
  }

  void PrecedenceRulesGraph::dumpControlFlow( std::ostringstream& ost, ControlFlowNode* node,
                                              const int& indent ) const {
    ost << std::string( indent * 2, ' ' );
    DecisionNode*  dn = dynamic_cast<DecisionNode*>( node );
    AlgorithmNode* an = dynamic_cast<AlgorithmNode*>( node );
    if ( dn != 0 ) {
      if ( node != m_headNode ) {
        ost << node->getNodeName() << " [Seq] ";
        ost << ( ( dn->m_modeConcurrent ) ? " [Concurrent] " : " [Sequential] " );
        ost << ( ( dn->m_modePromptDecision ) ? " [Prompt] " : "" );
        ost << ( ( dn->m_modeOR ) ? " [OR] " : "" );
        ost << ( ( dn->m_allPass ) ? " [PASS] " : "" );
        ost << "\n";
      }
      for ( const auto& i : dn->getDaughters() ) dumpControlFlow( ost, i, indent + 1 );
    } else if ( an != 0 ) {
      ost << node->getNodeName() << " [Alg] ";
      if ( an != 0 ) {
        auto ar = an->getAlgorithm();
        ost << " [n= " << ar->cardinality() << "]";
        ost << ( ( !ar->isClonable() ) ? " [unclonable] " : "" );
      }
      ost << "\n";
    }
  }

  //---------------------------------------------------------------------------
  std::string PrecedenceRulesGraph::dumpDataFlow() const {

    const char         idt[] = "      ";
    std::ostringstream ost;

    ost << "\n" << idt << "====================================\n";
    ost << idt << "Data origins and destinations:\n";
    ost << idt << "====================================\n";

    for ( auto& pair : m_dataPathToDataNodeMap ) {

      for ( auto algoNode : pair.second->getProducers() ) ost << idt << "  " << algoNode->getNodeName() << "\n";

      ost << idt << "  V\n";
      ost << idt << "  o " << pair.first << "\n";
      ost << idt << "  V\n";

      for ( auto algoNode : pair.second->getConsumers() ) ost << idt << "  " << algoNode->getNodeName() << "\n";

      ost << idt << "====================================\n";
    }

    return ost.str();
  }

  //---------------------------------------------------------------------------

  void PrecedenceRulesGraph::dumpPrecRules( const boost::filesystem::path& fileName, const EventSlot& slot ) {
    boost::filesystem::ofstream myfile;
    myfile.open( fileName, std::ios::app );

    // Declare properties to dump
    boost::dynamic_properties dp;

    dp.property( "Entity", boost::make_transform_value_property_map(
                               []( const VariantVertexProps& v ) { return boost::lexical_cast<std::string>( v ); },
                               boost::get( boost::vertex_bundle, m_PRGraph ) ) );

    auto add_prop = [&]( auto name, auto&& vis ) {
      dp.property( name, boost::make_transform_value_property_map(
                             [vis = std::forward<decltype( vis )>( vis )]( const VariantVertexProps& v ) {
                               return boost::apply_visitor( vis, v );
                             },
                             boost::get( boost::vertex_bundle, m_PRGraph ) ) );
    };

    add_prop( "Name", precedence::VertexName() );
    add_prop( "Mode", precedence::GroupMode() );
    add_prop( "Logic", precedence::GroupLogic() );
    add_prop( "Decision Negation", precedence::DecisionNegation() );
    add_prop( "Negative Decision Inversion", precedence::AllPass() );
    add_prop( "Exit Policy", precedence::GroupExit() );
    add_prop( "Operations", precedence::Operations() );
    add_prop( "CF Decision", precedence::CFDecision( slot ) );
    add_prop( "State", precedence::EntityState( slot, serviceLocator(), m_conditionsRealmEnabled ) );
    add_prop( "Start Time (Epoch ns)", precedence::StartTime( slot, serviceLocator() ) );
    add_prop( "End Time (Epoch ns)", precedence::EndTime( slot, serviceLocator() ) );
    add_prop( "Runtime (ns)", precedence::Duration( slot, serviceLocator() ) );

    boost::write_graphml( myfile, m_PRGraph, dp );

    myfile.close();
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::dumpPrecTrace( const boost::filesystem::path& fileName ) {
    boost::filesystem::ofstream myfile;
    myfile.open( fileName, std::ios::app );

    // Fill runtimes (as this could not be done on the fly during trace assembling)
    SmartIF<ITimelineSvc> timelineSvc = m_svcLocator->service<ITimelineSvc>( "TimelineSvc", false );
    if ( !timelineSvc.isValid() ) {
      warning() << "Failed to get the TimelineSvc, timing will not be added to "
                << "the task precedence trace dump" << endmsg;
    } else {

      for ( auto vp = vertices( m_precTrace ); vp.first != vp.second; ++vp.first ) {
        TimelineEvent te{};
        te.algorithm = m_precTrace[*vp.first].m_name;
        timelineSvc->getTimelineEvent( te );
        int runtime = std::chrono::duration_cast<std::chrono::microseconds>( te.end - te.start ).count();
        m_precTrace[*vp.first].m_runtime = runtime;
      }
    }

    // Declare properties to dump
    boost::dynamic_properties dp;
    using boost::get;
    using precedence::AlgoTraceProps;
    dp.property( "Name", get( &AlgoTraceProps::m_name, m_precTrace ) );
    dp.property( "Rank", get( &AlgoTraceProps::m_rank, m_precTrace ) );
    dp.property( "Runtime", get( &AlgoTraceProps::m_runtime, m_precTrace ) );

    boost::write_graphml( myfile, m_precTrace, dp );

    myfile.close();
  }

  void PrecedenceRulesGraph::addEdgeToPrecTrace( const AlgorithmNode* u, const AlgorithmNode* v ) {

    std::string u_name = u == nullptr ? "ENTRY" : u->getNodeName();
    std::string v_name = v->getNodeName();

    precedence::AlgoTraceVertex source;

    if ( !u ) {
      auto itT = m_prec_trace_map.find( "ENTRY" );
      if ( itT != m_prec_trace_map.end() ) {
        source = itT->second;
      } else {
        source = boost::add_vertex( precedence::AlgoTraceProps( "ENTRY", -1, -1, -1.0 ), m_precTrace );
        m_prec_trace_map["ENTRY"] = source;
      }
    } else {
      auto itS = m_prec_trace_map.find( u_name );
      if ( itS != m_prec_trace_map.end() ) {
        source = itS->second;
      } else {

        source =
            boost::add_vertex( precedence::AlgoTraceProps( u_name, u->getAlgoIndex(), u->getRank(), -1 ), m_precTrace );
        m_prec_trace_map[u_name] = source;
      }
    }

    precedence::AlgoTraceVertex target;

    auto itP = m_prec_trace_map.find( v_name );
    if ( itP != m_prec_trace_map.end() ) {
      target = itP->second;
    } else {

      target =
          boost::add_vertex( precedence::AlgoTraceProps( v_name, v->getAlgoIndex(), v->getRank(), -1 ), m_precTrace );
      m_prec_trace_map[v_name] = target;
    }

    boost::add_edge( source, target, m_precTrace );

    ON_DEBUG debug() << u_name << "-->" << v_name << " precedence trait added" << endmsg;
  }

} // namespace concurrency
