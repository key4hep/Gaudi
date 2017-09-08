#include "ControlFlowGraph.h"
#include "GaudiKernel/DataHandleHolderVisitor.h"

namespace concurrency {
  namespace recursive_CF {

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
  void DecisionNode::initialize( const std::unordered_map<std::string, unsigned int>& algname_index_map )
  {

    for ( auto daughter : m_children ) daughter->initialize( algname_index_map );
  }

  //---------------------------------------------------------------------------
  void DecisionNode::addParentNode( DecisionNode* node )
  {

    if ( std::find( m_parents.begin(), m_parents.end(), node ) == m_parents.end() ) m_parents.push_back( node );
  }

  //--------------------------------------------------------------------------
  void DecisionNode::addDaughterNode( ControlFlowNode* node )
  {

    if ( std::find( m_children.begin(), m_children.end(), node ) == m_children.end() ) m_children.push_back( node );
  }

  //---------------------------------------------------------------------------
  void DecisionNode::printState( std::stringstream& output, AlgsExecutionStates& states,
                                 const std::vector<int>& node_decisions, const unsigned int& recursionLevel ) const
  {

    output << std::string( recursionLevel, ' ' ) << m_nodeName << " (" << m_nodeIndex << ")"
           << ", w/ decision: " << stateToString( node_decisions[m_nodeIndex] ) << "(" << node_decisions[m_nodeIndex]
           << ")" << std::endl;
    for ( auto daughter : m_children ) {
      daughter->printState( output, states, node_decisions, recursionLevel + 2 );
    }
  }

  //---------------------------------------------------------------------------
  int DecisionNode::updateState( AlgsExecutionStates& states, std::vector<int>& node_decisions ) const
  {
    // check whether we already had a result earlier
    //    if (-1 != node_decisions[m_nodeIndex] ) { return node_decisions[m_nodeIndex]; }
    int decision           = ( ( m_allPass && m_modePromptDecision ) ? 1 : -1 );
    bool hasUndecidedChild = false;
    for ( auto daughter : m_children ) {
      if ( m_modePromptDecision && ( -1 != decision || hasUndecidedChild ) ) {
        node_decisions[m_nodeIndex] = decision;
        return decision;
      } // if prompt decision, return once result is known already or we can't fully evaluate right now because one daugther
      // decision is missing still
      auto res = daughter->updateState( states, node_decisions );
      if ( -1 == res ) {
        hasUndecidedChild = true;
      } else if ( false == m_modeOR && res == 0 ) {
        decision = 0;
      } // "and"-mode (once first result false, the overall decision is false)
      else if ( true == m_modeOR && res == 1 ) {
        decision = 1;
      } // "or"-mode  (once first result true, the overall decision is true)
    }
    // what to do with yet undefined answers depends on whether AND or OR mode applies
    if ( !hasUndecidedChild && -1 == decision ) {
      // OR mode: all results known, and none true -> reject
      if ( true == m_modeOR ) {
        decision = 0;
      }
      // AND mode: all results known, and no false -> accept
      else {
        decision = 1;
      }
    }
    // in all other cases I stay with previous decisions
    node_decisions[m_nodeIndex] = decision;
    if ( m_allPass ) decision   = 1;
    return decision;
  }

  //---------------------------------------------------------------------------
  AlgorithmNode::~AlgorithmNode()
  {

    for ( auto node : m_outputs ) {
      delete node;
    }
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::initialize( const std::unordered_map<std::string, unsigned int>& algname_index_map )
  {

    m_algoIndex = algname_index_map.at( m_algoName );
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::printState( std::stringstream& output, AlgsExecutionStates& states,
                                  const std::vector<int>& node_decisions, const unsigned int& recursionLevel ) const
  {
    output << std::string( recursionLevel, ' ' ) << m_nodeName << " (" << m_nodeIndex << ")"
           << ", w/ decision: " << stateToString( node_decisions[m_nodeIndex] ) << "(" << node_decisions[m_nodeIndex]
           << ")"
           << ", in state: " << AlgsExecutionStates::stateNames[states[m_algoIndex]] << std::endl;
  }

  //---------------------------------------------------------------------------
  int AlgorithmNode::updateState( AlgsExecutionStates& states, std::vector<int>& node_decisions ) const
  {
    // check whether we already had a result earlier
    //    if (-1 != node_decisions[m_nodeIndex] ) { return node_decisions[m_nodeIndex]; }
    // since we reached this point in the control flow, this algorithm is supposed to run
    // if it hasn't already
    const State& state    = states[m_algoIndex];
    unsigned int decision = -1;
    if ( State::INITIAL == state ) {
      states.updateState( m_algoIndex, State::CONTROLREADY ).ignore();
    }
    // now derive the proper result to pass back
    if ( true == m_allPass ) {
      decision = 1;
    } else if ( State::EVTACCEPTED == state ) {
      decision = !m_inverted;
    } else if ( State::EVTREJECTED == state ) {
      decision = m_inverted;
    } else {
      decision = -1; // result not known yet
    }
    node_decisions[m_nodeIndex] = decision;
    return decision;
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::addParentNode( DecisionNode* node )
  {

    if ( std::find( m_parents.begin(), m_parents.end(), node ) == m_parents.end() ) m_parents.push_back( node );
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::addOutputDataNode( DataNode* node )
  {

    if ( std::find( m_outputs.begin(), m_outputs.end(), node ) == m_outputs.end() ) m_outputs.push_back( node );
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::addInputDataNode( DataNode* node )
  {

    if ( std::find( m_inputs.begin(), m_inputs.end(), node ) == m_inputs.end() ) m_inputs.push_back( node );
  }

  //---------------------------------------------------------------------------
  StatusCode ControlFlowGraph::initialize( const std::unordered_map<std::string, unsigned int>& algname_index_map )
  {

    m_headNode->initialize( algname_index_map );
    StatusCode sc = buildAugmentedDataDependenciesRealm();

    if ( !sc.isSuccess() ) error() << "Could not build the data dependency realm." << endmsg;

    if (msgLevel(MSG::DEBUG))
      debug() << dumpDataFlow() << endmsg;

    return sc;
  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::registerIODataObjects( const Algorithm* algo )
  {

    const std::string& algoName = algo->name();

    m_algoNameToAlgoInputsMap[algoName] = algo->inputDataObjs();
    m_algoNameToAlgoOutputsMap[algoName] = algo->outputDataObjs();

    if (msgLevel(MSG::VERBOSE)) {
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
  StatusCode ControlFlowGraph::buildAugmentedDataDependenciesRealm()
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
        }
      }
    }

    return global_sc;
  }

  //---------------------------------------------------------------------------
  StatusCode ControlFlowGraph::addAlgorithmNode(Algorithm* algo,
                                                    const std::string& parentName,
                                                    bool inverted, bool allPass)
  {

    StatusCode sc = StatusCode::SUCCESS;

    auto& algoName = algo->name();

    auto itP = m_decisionNameToDecisionHubMap.find( parentName );
    concurrency::recursive_CF::DecisionNode* parentNode;
    if ( itP != m_decisionNameToDecisionHubMap.end() ) {
      parentNode = itP->second;
      auto itA   = m_algoNameToAlgoNodeMap.find( algoName );
      concurrency::recursive_CF::AlgorithmNode* algoNode;
      if ( itA != m_algoNameToAlgoNodeMap.end() ) {
        algoNode = itA->second;
      } else {
        algoNode = new concurrency::recursive_CF::AlgorithmNode(*this,algo,m_nodeCounter,
                                                  m_algoCounter,inverted,allPass);
        ++m_nodeCounter;
        ++m_algoCounter;
        m_algoNameToAlgoNodeMap[algoName] = algoNode;
        if (msgLevel(MSG::VERBOSE))
          verbose() << "AlgoNode " << algoName << " added @ " << algoNode << endmsg;
        registerIODataObjects(algo);
      }

      parentNode->addDaughterNode( algoNode );
      algoNode->addParentNode( parentNode );
    } else {
      sc = StatusCode::FAILURE;
      error() << "Decision hub node " << parentName
              << ", requested to be parent, is not registered." << endmsg;
    }

    return sc;
  }

  //---------------------------------------------------------------------------
  AlgorithmNode* ControlFlowGraph::getAlgorithmNode( const std::string& algoName ) const
  {

    return m_algoNameToAlgoNodeMap.at( algoName );
  }

  //---------------------------------------------------------------------------
  StatusCode ControlFlowGraph::addDataNode( const DataObjID& dataPath )
  {

    StatusCode sc;

    auto itD = m_dataPathToDataNodeMap.find( dataPath );
    concurrency::recursive_CF::DataNode* dataNode;
    if ( itD != m_dataPathToDataNodeMap.end() ) {
      dataNode = itD->second;
      sc = StatusCode::SUCCESS;
    } else {
      dataNode                          = new concurrency::recursive_CF::DataNode( *this, dataPath );
      m_dataPathToDataNodeMap[dataPath] = dataNode;
      if (msgLevel(MSG::VERBOSE))
        verbose() << "  DataNode for " << dataPath << " added @ " << dataNode << endmsg;
      sc = StatusCode::SUCCESS;
    }

    return sc;
  }

  //---------------------------------------------------------------------------
  DataNode* ControlFlowGraph::getDataNode( const DataObjID& dataPath ) const
  {

    return m_dataPathToDataNodeMap.at( dataPath );
  }

  //---------------------------------------------------------------------------
  StatusCode ControlFlowGraph::addDecisionHubNode( Algorithm* decisionHubAlgo, const std::string& parentName,
		                                              bool modeConcurrent, bool modePromptDecision, bool modeOR, bool allPass)
  {

    StatusCode sc = StatusCode::SUCCESS;

    auto& decisionHubName = decisionHubAlgo->name();

    auto itP = m_decisionNameToDecisionHubMap.find( parentName );
    concurrency::recursive_CF::DecisionNode* parentNode;
    if ( itP != m_decisionNameToDecisionHubMap.end() ) {
      parentNode = itP->second;
      auto itA   = m_decisionNameToDecisionHubMap.find( decisionHubName );
      concurrency::recursive_CF::DecisionNode* decisionHubNode;
      if ( itA != m_decisionNameToDecisionHubMap.end() ) {
        decisionHubNode = itA->second;
      } else {
        decisionHubNode =
            new concurrency::recursive_CF::DecisionNode( *this, m_nodeCounter, decisionHubName, modeConcurrent, modePromptDecision, modeOR, allPass);
        ++m_nodeCounter;
        m_decisionNameToDecisionHubMap[decisionHubName] = decisionHubNode;
        if (msgLevel(MSG::VERBOSE))
          verbose() << "Decision hub node " << decisionHubName << " added @ " << decisionHubNode << endmsg;
      }

      parentNode->addDaughterNode( decisionHubNode );
      decisionHubNode->addParentNode( parentNode );
    } else {
      sc = StatusCode::FAILURE;
      error() << "Decision hub node " << parentName << ", requested to be parent, is not registered."
              << endmsg;
    }

    return sc;
  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::addHeadNode( const std::string& headName, bool modeConcurrent, bool modePromptDecision, bool modeOR, bool allPass)
  {

    auto itH = m_decisionNameToDecisionHubMap.find( headName );
    if ( itH != m_decisionNameToDecisionHubMap.end() ) {
      m_headNode = itH->second;
    } else {
      m_headNode = new concurrency::recursive_CF::DecisionNode( *this, m_nodeCounter, headName, modeConcurrent, modePromptDecision, modeOR, allPass );
      ++m_nodeCounter;
      m_decisionNameToDecisionHubMap[headName] = m_headNode;
    }
  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::updateEventState( AlgsExecutionStates& algo_states, std::vector<int>& node_decisions ) const
  {
    m_headNode->updateState( algo_states, node_decisions );
  }

  std::string ControlFlowGraph::dumpControlFlow() const {
    std::ostringstream ost;
    dumpControlFlow(ost,m_headNode,0);
    return ost.str();
  }

  void ControlFlowGraph::dumpControlFlow(std::ostringstream& ost,
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
  std::string ControlFlowGraph::dumpDataFlow() const
  {

    const char idt[] = "      ";
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

  void ControlFlowGraph::dumpExecutionPlan(const boost::filesystem::path& fileName)
  {
    boost::filesystem::ofstream myfile;
    myfile.open( fileName, std::ios::app );

    boost::dynamic_properties dp;
    dp.property( "name", boost::get( &boost::AlgoProps__::m_name, m_ExecPlan ) );
    dp.property( "index", boost::get( &boost::AlgoProps__::m_index, m_ExecPlan ) );
    dp.property( "rank", boost::get( &boost::AlgoProps__::m_rank, m_ExecPlan ) );
    dp.property( "runtime", boost::get( &boost::AlgoProps__::m_runtime, m_ExecPlan ) );

    boost::write_graphml( myfile, m_ExecPlan, dp );

    myfile.close();
  }

  void ControlFlowGraph::addEdgeToExecutionPlan( const AlgorithmNode* u, const AlgorithmNode* v )
  {

    boost::AlgoVertex source;
    float runtime( 0. );
    if ( u == nullptr ) {
      auto itT = m_exec_plan_map.find( "ENTRY" );
      if ( itT != m_exec_plan_map.end() ) {
        source = itT->second;
      } else {
        source                   = boost::add_vertex( boost::AlgoProps__( "ENTRY", -999, -999, 0 ), m_ExecPlan );
        m_exec_plan_map["ENTRY"] = source;
      }
    } else {
      auto itS = m_exec_plan_map.find( u->getNodeName() );
      if ( itS != m_exec_plan_map.end() ) {
        source = itS->second;
      } else {
        auto alg = u->getAlgorithm();
        try {
          const Gaudi::Details::PropertyBase& p = alg->getProperty( "AvgRuntime" );
          runtime = std::stof( p.toString() );
        } catch(...) {
          if (msgLevel(MSG::DEBUG))
            debug() << "no AvgRuntime for " << alg->name() << endmsg;
          runtime = 1.;
        }

        source = boost::add_vertex( boost::AlgoProps__( u->getNodeName(), u->getAlgoIndex(), u->getRank(), runtime ),
                                    m_ExecPlan );
        m_exec_plan_map[u->getNodeName()] = source;
      }
    }

    boost::AlgoVertex target;
    auto itP = m_exec_plan_map.find( v->getNodeName() );
    if ( itP != m_exec_plan_map.end() ) {
      target = itP->second;
    } else {
      auto alg = v->getAlgorithm();
      try {
        const Gaudi::Details::PropertyBase& p = alg->getProperty( "AvgRuntime" );
        runtime = std::stof( p.toString() );
      } catch(...) {
        if (msgLevel(MSG::DEBUG))
          debug() << "no AvgRuntime for " << alg->name() << endmsg;
        runtime = 1.;
      }

      target = boost::add_vertex( boost::AlgoProps__( v->getNodeName(), v->getAlgoIndex(), v->getRank(), runtime ),
                                  m_ExecPlan );
      m_exec_plan_map[v->getNodeName()] = target;
    }

    if (msgLevel(MSG::DEBUG))
      debug() << "Edge added to execution plan" << endmsg;
    boost::add_edge(source, target, m_ExecPlan);
  }
  }  // namespace
} // namespace
