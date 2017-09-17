#include "PrecedenceRulesGraph.h"
#include "PRGraphVisitors.h"

#include <fstream>
#include <boost/property_map/transform_value_property_map.hpp>

#include "GaudiKernel/DataHandleHolderVisitor.h"

#define ON_DEBUG if (msgLevel(MSG::DEBUG))
#define ON_VERBOSE if (msgLevel(MSG::VERBOSE))

namespace concurrency
{

  //---------------------------------------------------------------------------
  std::string ControlFlowNode::stateToString( const int& stateId ) const
  {

    if ( 0 == stateId )
      return "FALSE";
    else if ( 1 == stateId )
      return "TRUE";
    else
      return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  DecisionNode::~DecisionNode()
  {

    for ( auto node : m_children ) delete node;
  }

  //---------------------------------------------------------------------------
  void DecisionNode::addParentNode( DecisionNode* node )
  {

    if ( std::find( m_parents.begin(), m_parents.end(), node ) == m_parents.end() )
      m_parents.push_back( node );
  }

  //--------------------------------------------------------------------------
  void DecisionNode::addDaughterNode( ControlFlowNode* node )
  {

    if ( std::find( m_children.begin(), m_children.end(), node ) == m_children.end() )
      m_children.push_back( node );
  }

  //---------------------------------------------------------------------------
  void DecisionNode::printState( std::stringstream& output, AlgsExecutionStates& states,
                                 const std::vector<int>& node_decisions,
                                 const unsigned int& recursionLevel ) const
  {

    output << std::string( recursionLevel, ' ' ) << m_nodeName << " ("
           << m_nodeIndex << ")" << ", w/ decision: "
           << stateToString( node_decisions[m_nodeIndex] ) << "("
           << node_decisions[m_nodeIndex] << ")" << std::endl;

    for ( auto daughter : m_children )
      daughter->printState( output, states, node_decisions, recursionLevel + 2 );
  }

  //---------------------------------------------------------------------------
  bool DecisionNode::accept( IGraphVisitor& visitor )
  {

    if ( visitor.visitEnter( *this ) ) {
      // try to aggregate a decision
      bool result = visitor.visit( *this );

      // if a decision was made for this node, propagate the result upwards
      if ( result ) {
        for ( auto parent : m_parents ) {
          parent->accept( visitor );
        }
        return false;
      }

      // if no decision can be made yet, request further information downwards
      for ( auto child : m_children ) {
        bool result = child->accept( visitor );
        if (!m_modeConcurrent)
          if ( result ) break; //stop on first unresolved child if its decision hub is sequential
      }

      return true; // visitor was accepted to try to aggregate the node's decision
    }

    return false; // visitor was rejected (since the decision node has an aggregated decision already)
  }

  //---------------------------------------------------------------------------
  AlgorithmNode::~AlgorithmNode()
  {

    for ( auto node : m_outputs ) {
      delete node;
    }
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::printState( std::stringstream& output, AlgsExecutionStates& states,
                                  const std::vector<int>& node_decisions,
                                  const unsigned int& recursionLevel ) const
  {
    output << std::string( recursionLevel, ' ' ) << m_nodeName << " ("
           << m_nodeIndex << ")" << ", w/ decision: "
           << stateToString( node_decisions[m_nodeIndex] )
           << "(" << node_decisions[m_nodeIndex] << ")" << ", in state: "
           << AlgsExecutionStates::stateNames[states[m_algoIndex]] << std::endl;
  }

  //---------------------------------------------------------------------------
  bool AlgorithmNode::accept( IGraphVisitor& visitor )
  {

    if ( visitor.visitEnter( *this ) ) {
      visitor.visit( *this );
      return true; // visitor was accepted to promote the algorithm
    }

    return false; // visitor was rejected (since the algorithm already produced a decision)
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::addParentNode( DecisionNode* node )
  {

    if ( std::find( m_parents.begin(), m_parents.end(), node ) == m_parents.end() )
      m_parents.push_back( node );
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::addOutputDataNode( DataNode* node )
  {

    if ( std::find( m_outputs.begin(), m_outputs.end(), node ) == m_outputs.end() )
      m_outputs.push_back( node );
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::addInputDataNode( DataNode* node )
  {

    if ( std::find( m_inputs.begin(), m_inputs.end(), node ) == m_inputs.end() )
      m_inputs.push_back( node );
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::initialize()
  {

    if (serviceLocator()->existsService("CondSvc")) {
      SmartIF<ICondSvc> condSvc {serviceLocator()->service("CondSvc")};
      if (condSvc.isValid())  {
        info() << "CondSvc found. DF precedence rules will be augmented with 'Conditions'"
               << endmsg;
        m_conditionsRealmEnabled = true;
      }
    }

    // Detach condition algorithms from the CF realm
    if (m_conditionsRealmEnabled) {
      SmartIF<ICondSvc> condSvc {serviceLocator()->service("CondSvc", false)};
      auto& condAlgs = condSvc->condAlgs();
      for (const auto algo : condAlgs) {
        auto itA   = m_algoNameToAlgoNodeMap.find( algo->name() );
        concurrency::AlgorithmNode* algoNode;
        if ( itA != m_algoNameToAlgoNodeMap.end() ) {
          algoNode = itA->second;
          debug() << "Detaching condition algorithm '" << algo->name()
                  << "' from the CF realm.." << endmsg;
          for (auto parent : algoNode->getParentDecisionHubs()) {
            parent->m_children.erase(std::remove(parent->m_children.begin(),
                                                 parent->m_children.end(),
                                                 algoNode),
                                     parent->m_children.end());
          }
          algoNode->m_parents.clear();

        } else {
          warning() << "Algorithm '" << algo->name()
                    << "' is not registered in the graph" << endmsg;
        }
      }
    }

    StatusCode sc = buildDataDependenciesRealm();

    if ( !sc.isSuccess() ) error() << "Could not build the data dependency realm." << endmsg;

    ON_DEBUG debug() << dumpDataFlow() << endmsg;

    return sc;
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::registerIODataObjects( const Algorithm* algo )
  {

    const std::string& algoName = algo->name();

    m_algoNameToAlgoInputsMap[algoName] = algo->inputDataObjs();
    m_algoNameToAlgoOutputsMap[algoName] = algo->outputDataObjs();

    ON_VERBOSE {
      verbose() << "    Inputs of " << algoName << ": ";
      for (auto tag : algo->inputDataObjs())
        verbose() << tag << " | ";
      verbose() << endmsg;

      verbose() << "    Outputs of " << algoName << ": ";
      for (auto tag : algo->outputDataObjs())
        verbose() << tag << " | ";
      verbose() << endmsg;
    }
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::buildDataDependenciesRealm()
  {

    StatusCode global_sc( StatusCode::SUCCESS, true );

    // Production of DataNodes by AlgorithmNodes (DataNodes are created here)
    for (auto algo : m_algoNameToAlgoNodeMap) {

      auto& outputs = m_algoNameToAlgoOutputsMap[algo.first];
      for (auto output : outputs) {
        const auto sc = addDataNode(output);
        if (!sc.isSuccess()) {
          error() << "Extra producer (" << algo.first << ") for DataObject @ "
                  << output
                  << " has been detected: this is not allowed." << endmsg;
          global_sc = sc;
        }
        auto dataNode = getDataNode(output);
        dataNode->addProducerNode(algo.second);
        algo.second->addOutputDataNode(dataNode);
        ON_DEBUG { // Mirror the action above in the BGL-based graph
          boost::add_edge(node(algo.second->getNodeName()),
                          node(output.fullKey()),
                          m_PRGraph);
        }
      }
    }

    // Consumption of DataNodes by AlgorithmNodes
    for ( auto algo : m_algoNameToAlgoNodeMap ) {

      for (auto input : m_algoNameToAlgoInputsMap[algo.first]) {

        DataNode* dataNode = nullptr;

        auto itP = m_dataPathToDataNodeMap.find(input);

        if (itP != m_dataPathToDataNodeMap.end())
          dataNode = getDataNode(input);

        if (dataNode) {
          dataNode->addConsumerNode(algo.second);
          algo.second->addInputDataNode(dataNode);
          ON_DEBUG { // Mirror the action above in the BGL-based graph
            boost::add_edge(node(input.fullKey()),
                            node(algo.second->getNodeName()),
                            m_PRGraph);
          }
        }
      }
    }

    return global_sc;
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::addAlgorithmNode(Algorithm* algo,
                                                    const std::string& parentName,
                                                    bool inverted, bool allPass)
  {

    StatusCode sc = StatusCode::SUCCESS;

    // Create new, or fetch existent, AlgorithmNode
    auto& algoName = algo->name();
    auto itA   = m_algoNameToAlgoNodeMap.find( algoName );
    concurrency::AlgorithmNode* algoNode;
    if ( itA != m_algoNameToAlgoNodeMap.end() ) {
      algoNode = itA->second;
    } else {
      algoNode = new concurrency::AlgorithmNode(*this,algo,m_nodeCounter,
                                                m_algoCounter,inverted,allPass);
      ON_DEBUG { // Mirror the action above in the BGL-based graph
        auto source = boost::add_vertex(AlgoProps(algo,m_nodeCounter,
                                                  m_algoCounter,inverted,
                                                  allPass),
                                        m_PRGraph);
        boost::add_edge(source, node(parentName), m_PRGraph);
      }
      ++m_nodeCounter;
      ++m_algoCounter;
      m_algoNameToAlgoNodeMap[algoName] = algoNode;
      ON_VERBOSE verbose() << "AlgoNode " << algoName << " added @ "
                                   << algoNode << endmsg;
      registerIODataObjects(algo);
    }

    // Attach AlgorithmNode to its CF decision hub
    auto itP = m_decisionNameToDecisionHubMap.find( parentName );
    if ( itP != m_decisionNameToDecisionHubMap.end() ) {
      auto parentNode = itP->second;
      ON_VERBOSE verbose() << "Attaching AlgorithmNode '" << algo->name()
                           << "' to DecisionNode '" << parentName << "'" << endmsg;

      parentNode->addDaughterNode( algoNode );
      algoNode->addParentNode( parentNode );
    } else {
      sc = StatusCode::FAILURE;
      error() << "Requested DecisionNode '" << parentName << "' was not found"
              << endmsg;
    }

    return sc;
  }

  //---------------------------------------------------------------------------
  AlgorithmNode* PrecedenceRulesGraph::getAlgorithmNode( const std::string& algoName ) const
  {

    return m_algoNameToAlgoNodeMap.at( algoName );
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::addDataNode( const DataObjID& dataPath )
  {

    StatusCode sc;

    auto itD = m_dataPathToDataNodeMap.find( dataPath );
    concurrency::DataNode* dataNode;
    if ( itD != m_dataPathToDataNodeMap.end() ) {
      dataNode = itD->second;
      sc = StatusCode::SUCCESS;
    } else {
      if (!m_conditionsRealmEnabled) {
        dataNode = new concurrency::DataNode(*this, dataPath);
        ON_VERBOSE verbose() << "  DataNode for " << dataPath << " added @ "
                             << dataNode << endmsg;
      } else {
        SmartIF<ICondSvc> condSvc {serviceLocator()->service("CondSvc",false)};
        if (condSvc->isRegistered(dataPath)) {
          dataNode = new concurrency::ConditionNode(*this, dataPath, condSvc);
          ON_VERBOSE verbose() << "  ConditionNode for " << dataPath
                               << " added @ " << dataNode << endmsg;
        } else {
          dataNode = new concurrency::DataNode(*this, dataPath);
          ON_VERBOSE verbose() << "  DataNode for " << dataPath
                               << " added @ " << dataNode << endmsg;
        }
      }

      m_dataPathToDataNodeMap[dataPath] = dataNode;

      sc = StatusCode::SUCCESS;
      ON_DEBUG { // Mirror the action above in the BGL-based graph
        boost::add_vertex(DataProps(dataPath),m_PRGraph);
      }
    }

    return sc;
  }

  //---------------------------------------------------------------------------
  DataNode* PrecedenceRulesGraph::getDataNode( const DataObjID& dataPath ) const
  {

    return m_dataPathToDataNodeMap.at( dataPath );
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::addDecisionHubNode(Algorithm* decisionHubAlgo,
                                                      const std::string& parentName,
		                                              bool modeConcurrent,
		                                              bool modePromptDecision,
		                                              bool modeOR, bool allPass)
  {

    StatusCode sc = StatusCode::SUCCESS;

    auto& decisionHubName = decisionHubAlgo->name();

    auto itP = m_decisionNameToDecisionHubMap.find( parentName );
    concurrency::DecisionNode* parentNode;
    if ( itP != m_decisionNameToDecisionHubMap.end() ) {
      parentNode = itP->second;
      auto itA   = m_decisionNameToDecisionHubMap.find( decisionHubName );
      concurrency::DecisionNode* decisionHubNode;
      if ( itA != m_decisionNameToDecisionHubMap.end() ) {
        decisionHubNode = itA->second;
      } else {
        decisionHubNode =
            new concurrency::DecisionNode(*this,m_nodeCounter,decisionHubName,
                                          modeConcurrent,modePromptDecision,
                                          modeOR,allPass);
        m_decisionNameToDecisionHubMap[decisionHubName] = decisionHubNode;

        ON_DEBUG { // Mirror the action above in the BGL-based graph
          auto source = boost::add_vertex(DecisionHubProps(decisionHubName,
                                                           m_nodeCounter,
                                                           modeConcurrent,
                                                           modePromptDecision,
                                                           modeOR, allPass),
                                          m_PRGraph);
          boost::add_edge(source, node(parentName), m_PRGraph);
        }

        ++m_nodeCounter;

        ON_VERBOSE verbose() << "Decision hub node " << decisionHubName
                             << " added @ " << decisionHubNode << endmsg;
      }

      parentNode->addDaughterNode( decisionHubNode );
      decisionHubNode->addParentNode( parentNode );
    } else {
      sc = StatusCode::FAILURE;
      error() << "Decision hub node " << parentName
              << ", requested to be parent, is not registered." << endmsg;
    }

    return sc;
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::addHeadNode(const std::string& headName,
                                         bool modeConcurrent,
                                         bool modePromptDecision,
                                         bool modeOR,
                                         bool allPass)
  {

    auto itH = m_decisionNameToDecisionHubMap.find( headName );
    if ( itH != m_decisionNameToDecisionHubMap.end() ) {
      m_headNode = itH->second;
    } else {
      m_headNode = new concurrency::DecisionNode( *this, m_nodeCounter,
                                                 headName, modeConcurrent,
                                                 modePromptDecision, modeOR,
                                                 allPass );
      m_decisionNameToDecisionHubMap[headName] = m_headNode;

      ON_DEBUG { // Mirror the action above in the BGL-based graph
        boost::add_vertex(DecisionHubProps(headName,m_nodeCounter,modeConcurrent,
                                           modePromptDecision,modeOR,allPass),
                          m_PRGraph);
      }

      ++m_nodeCounter;
    }

  }

  //---------------------------------------------------------------------------
  PRVertexDesc PrecedenceRulesGraph::node(const std::string& name) const {

    PRVertexDesc target{};

    for (auto vp = vertices(m_PRGraph); vp.first != vp.second; ++vp.first) {
      PRVertexDesc v = *vp.first;
      if (boost::apply_visitor(VertexName(),m_PRGraph[v]) == name) {
        target = v;
        break;
      }
    }

    return target;
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::accept(const std::string& algo_name,
                                    IGraphVisitor& visitor) const
  {
    getAlgorithmNode( algo_name )->accept(visitor);
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::rankAlgorithms( IGraphVisitor& ranker ) const
  {

    info() << "Starting ranking by data outputs .. " << endmsg;
    for (auto& pair : m_algoNameToAlgoNodeMap) {
      ON_DEBUG debug() << "  Ranking " << pair.first << "... " << endmsg;
      pair.second->accept(ranker);
      ON_DEBUG debug() << "  ... rank of " << pair.first << ": "
                       << pair.second->getRank() << endmsg;
    }
  }

  std::string PrecedenceRulesGraph::dumpControlFlow() const {
    std::ostringstream ost;
    dumpControlFlow(ost,m_headNode,0);
    return ost.str();
  }

  void PrecedenceRulesGraph::dumpControlFlow(std::ostringstream& ost,
                                             ControlFlowNode* node,
                                             const int& indent) const {
    ost << std::string(indent*2, ' ');
    DecisionNode  *dn = dynamic_cast<DecisionNode*> (node);
    AlgorithmNode *an = dynamic_cast<AlgorithmNode*> (node);
    if ( dn != 0 ) {
      if (node != m_headNode) {
        ost << node->getNodeName() << " [Seq] ";
        ost << ( (dn->m_modeConcurrent) ? " [Concurrent] " : " [Sequential] " );
        ost << ( (dn->m_modePromptDecision) ? " [Prompt] " : "" );
        ost << ( (dn->m_modeOR) ? " [OR] " : "" );
        ost << ( (dn->m_allPass) ? " [PASS] " : "" );
        ost << "\n";
      }
      const std::vector<ControlFlowNode*>& dth = dn->getDaughters();
      for (std::vector<ControlFlowNode*>::const_iterator itr= dth.begin();
           itr != dth.end(); ++itr) {
        dumpControlFlow(ost,*itr,indent+1);
      }
    } else if (an != 0) {
      ost << node->getNodeName() << " [Alg] ";
      if (an != 0) {
        auto ar = an->getAlgorithm();
        ost << " [n= " << ar->cardinality() << "]";
        ost << ( (! ar->isClonable()) ? " [unclonable] " : "" );
      }
      ost << "\n";
    }

  }

  //---------------------------------------------------------------------------
  std::string PrecedenceRulesGraph::dumpDataFlow() const
  {

    const char idt[] = "      ";
    std::ostringstream ost;

    ost << "\n" << idt << "====================================\n";
    ost << idt << "Data origins and destinations:\n";
    ost << idt << "====================================\n";

    for ( auto& pair : m_dataPathToDataNodeMap ) {

      for ( auto algoNode : pair.second->getProducers() )
        ost << idt << "  " << algoNode->getNodeName() << "\n";

      ost << idt << "  V\n";
      ost << idt << "  o " << pair.first << "\n";
      ost << idt << "  V\n";

      for ( auto algoNode : pair.second->getConsumers() )
        ost << idt << "  " << algoNode->getNodeName() << "\n";

      ost << idt << "====================================\n";
    }

    return ost.str();
  }

  //---------------------------------------------------------------------------

  void PrecedenceRulesGraph::dumpPrecRules(const boost::filesystem::path& fileName)
  {
    boost::filesystem::ofstream myfile;
    myfile.open( fileName, std::ios::app );

    boost::dynamic_properties dp;

    dp.property("Entity", boost::make_transform_value_property_map(
       [](VariantVertexProps const& v) { return boost::lexical_cast<std::string>(v); },
       boost::get(boost::vertex_bundle, m_PRGraph)));

    dp.property("Name", boost::make_transform_value_property_map(
       [](VariantVertexProps const& v) {return boost::apply_visitor(VertexName(), v);},
       boost::get(boost::vertex_bundle, m_PRGraph)));

    dp.property("Mode", boost::make_transform_value_property_map(
       [](VariantVertexProps const& v) {return boost::apply_visitor(GroupMode(), v);},
       boost::get(boost::vertex_bundle, m_PRGraph)));

    dp.property("Logic", boost::make_transform_value_property_map(
       [](VariantVertexProps const& v) {return boost::apply_visitor(GroupLogic(), v);},
       boost::get(boost::vertex_bundle, m_PRGraph)));

    dp.property("InvertedDecision", boost::make_transform_value_property_map(
       [](VariantVertexProps const& v) {return boost::apply_visitor(InvertedDecision(), v);},
       boost::get(boost::vertex_bundle, m_PRGraph)));

    dp.property("AllPass", boost::make_transform_value_property_map(
       [](VariantVertexProps const& v) {return boost::apply_visitor(AllPass(), v);},
       boost::get(boost::vertex_bundle, m_PRGraph)));

    dp.property("PromptDecision", boost::make_transform_value_property_map(
       [](VariantVertexProps const& v) {return boost::apply_visitor(GroupDecision(), v);},
       boost::get(boost::vertex_bundle, m_PRGraph)));

    dp.property("Operations", boost::make_transform_value_property_map(
       [](VariantVertexProps const& v) {return boost::apply_visitor(Operations(), v);},
       boost::get(boost::vertex_bundle, m_PRGraph)));


    boost::write_graphml( myfile, m_PRGraph, dp );

    myfile.close();
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::dumpPrecTrace(const boost::filesystem::path& fileName)
  {
    boost::filesystem::ofstream myfile;
    myfile.open( fileName, std::ios::app );

    boost::dynamic_properties dp;
    dp.property( "name", boost::get( &boost::AlgoTraceProps::m_name, m_precTrace ) );
    dp.property( "index", boost::get( &boost::AlgoTraceProps::m_index, m_precTrace ) );
    dp.property( "rank", boost::get( &boost::AlgoTraceProps::m_rank, m_precTrace ) );
    dp.property( "runtime", boost::get( &boost::AlgoTraceProps::m_runtime, m_precTrace ) );

    boost::write_graphml( myfile, m_precTrace, dp );

    myfile.close();
  }

  void PrecedenceRulesGraph::addEdgeToPrecTrace( const AlgorithmNode* u,
                                                 const AlgorithmNode* v )
  {

    boost::AlgoTraceVertex source;
    float runtime( 0. );
    if ( u == nullptr ) {
      auto itT = m_prec_trace_map.find( "ENTRY" );
      if ( itT != m_prec_trace_map.end() ) {
        source = itT->second;
      } else {
        source = boost::add_vertex( boost::AlgoTraceProps("ENTRY",-999,-999,0),m_precTrace);
        m_prec_trace_map["ENTRY"] = source;
      }
    } else {
      auto itS = m_prec_trace_map.find( u->getNodeName() );
      if ( itS != m_prec_trace_map.end() ) {
        source = itS->second;
      } else {
        auto alg = u->getAlgorithm();
        try {
          const Gaudi::Details::PropertyBase& p = alg->getProperty( "AvgRuntime" );
          runtime = std::stof( p.toString() );
        } catch(...) {
          ON_DEBUG debug() << "no AvgRuntime for " << alg->name() << endmsg;
          runtime = 1.;
        }

        source = boost::add_vertex(boost::AlgoTraceProps(u->getNodeName(),
                                                         u->getAlgoIndex(),
                                                         u->getRank(),
                                                         runtime ),
                                   m_precTrace);
        m_prec_trace_map[u->getNodeName()] = source;
      }
    }

    boost::AlgoTraceVertex target;
    auto itP = m_prec_trace_map.find( v->getNodeName() );
    if ( itP != m_prec_trace_map.end() ) {
      target = itP->second;
    } else {
      auto alg = v->getAlgorithm();
      try {
        const Gaudi::Details::PropertyBase& p = alg->getProperty( "AvgRuntime" );
        runtime = std::stof( p.toString() );
      } catch(...) {
        ON_DEBUG debug() << "no AvgRuntime for " << alg->name() << endmsg;
        runtime = 1.;
      }

      target = boost::add_vertex(boost::AlgoTraceProps(v->getNodeName(),
                                                       v->getAlgoIndex(),
                                                       v->getRank(),
                                                       runtime ),
                                 m_precTrace);
      m_prec_trace_map[v->getNodeName()] = target;
    }

    ON_DEBUG debug() << "Edge added to execution plan" << endmsg;
    boost::add_edge(source, target, m_precTrace);
  }

} // namespace
