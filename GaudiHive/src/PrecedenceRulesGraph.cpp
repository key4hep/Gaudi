#include "PrecedenceRulesGraph.h"
#include "PRGraphVisitors.h"

#include "GaudiKernel/DataHandleHolderVisitor.h"

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
  void DecisionNode::updateDecision( const int& slotNum, AlgsExecutionStates& states, std::vector<int>& node_decisions,
                                     const AlgorithmNode* requestor ) const
  {

    int decision           = ( ( m_allPass && m_modePromptDecision ) ? 1 : -1 );
    bool keepGoing         = true;
    bool hasUndecidedChild = false;
    // std::cout << "++++++++++++++++++++BEGIN(UPDATING)++++++++++++++++++++" << std::endl;
    // std::cout << "UPDATING DAUGHTERS of DECISION NODE: " << m_nodeName << std::endl;

    for ( auto daughter : m_children ) {
      // if prompt decision, return once result is known already or we can't fully evaluate
      // right now because one daughter decision is missing still
      // std::cout << "----UPDATING DAUGHTER: " << daughter->getNodeName() << std::endl;
      if ( m_modePromptDecision && !keepGoing ) {
        node_decisions[m_nodeIndex] = decision;
        // std::cout << "STOPPING ITERATION OVER (UPDATING) DECISION NODE CHILDREN: " << m_nodeName << std::endl;
        break;
        // return;
      }

      // modified
      int& res = node_decisions[daughter->getNodeIndex()];
      if ( -1 == res ) {
        hasUndecidedChild = true;
        if ( typeid( *daughter ) != typeid( concurrency::DecisionNode ) ) {
          auto algod = (AlgorithmNode*)daughter;
          algod->promoteToControlReadyState( slotNum, states, node_decisions );
          bool result             = algod->promoteToDataReadyState( slotNum, requestor );
          if ( result ) keepGoing = false;
        } else {
          daughter->updateDecision( slotNum, states, node_decisions, requestor );
        }

        // "and"-mode (once first result false, the overall decision is false)
      } else if ( false == m_modeOR && res == 0 ) {
        decision  = 0;
        keepGoing = false;
        // "or"-mode  (once first result true, the overall decision is true)
      } else if ( true == m_modeOR && res == 1 ) {
        decision  = 1;
        keepGoing = false;
      }
    }

    // what to do with yet undefined answers depends on whether AND or OR mode applies
    if ( !hasUndecidedChild && -1 == decision ) {
      // OR mode: all results known, and none true -> reject
      if ( true == m_modeOR ) {
        decision = 0;
        // AND mode: all results known, and no false -> accept
      } else {
        decision = 1;
      }
    }

    // in all other cases I stay with previous decisions
    node_decisions[m_nodeIndex] = decision;

    // propagate decision upwards through the decision graph
    if ( -1 != decision )
      for ( auto p : m_parents ) p->updateDecision( slotNum, states, node_decisions, requestor );

    // std::cout << "++++++++++++++++++++END(UPDATING)++++++++++++++++++++" << std::endl;
  }

  //---------------------------------------------------------------------------
  bool DecisionNode::promoteToControlReadyState( const int& slotNum, AlgsExecutionStates& states,
                                                 std::vector<int>& node_decisions ) const
  {
    // std::cout << "REACHED DECISNODE " << m_nodeName << std::endl;
    if ( -1 != node_decisions[m_nodeIndex] ) {
      return true;
    }

    for ( auto daughter : m_children ) {
      auto res = node_decisions[daughter->getNodeIndex()];
      if ( -1 == res ) {
        daughter->promoteToControlReadyState( slotNum, states, node_decisions );
        if ( m_modePromptDecision ) return true;
      } else if ( m_modePromptDecision ) {
        if ( ( false == m_modeOR && res == 0 ) || ( true == m_modeOR && res == 1 ) ) return true;
      }
    }

    return true;
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
  void AlgorithmNode::initialize( const std::unordered_map<std::string, unsigned int>& algname_index_map )
  {

    m_algoIndex = algname_index_map.at( m_algoName );
  }

  //---------------------------------------------------------------------------
  bool AlgorithmNode::promoteToControlReadyState( const int& /*slotNum*/, AlgsExecutionStates& states,
                                                  std::vector<int>& /*node_decisions*/ ) const
  {

    auto& state = states[m_algoIndex];
    bool result = false;

    if ( State::INITIAL == state ) {
      states.updateState( m_algoIndex, State::CONTROLREADY ).ignore();
      // std::cout << "----> UPDATING ALGORITHM to CONTROLREADY: " << m_algoName << std::endl;
      result = true;
    } else if ( State::CONTROLREADY == state ) {
      result = true;
    }

    return result;
  }

  //---------------------------------------------------------------------------
  bool AlgorithmNode::promoteToDataReadyState( const int& slotNum, const AlgorithmNode* /*requestor*/ ) const
  {

    auto& states = m_graph->getAlgoStates( slotNum );
    auto& state  = states[m_algoIndex];
    bool result  = false;

    if ( State::CONTROLREADY == state ) {
      if ( dataDependenciesSatisfied( slotNum ) ) {
        // std::cout << "----> UPDATING ALGORITHM to DATAREADY: " << m_algoName << std::endl;
        states.updateState( m_algoIndex, State::DATAREADY ).ignore();
        result = true;

        // m_graph->addEdgeToExecutionPlan(requestor, this);

        /*
        auto xtime = std::chrono::high_resolution_clock::now();
        std::stringstream s;
        s << getNodeName() << ", "
          << (xtime-m_graph->getInitTime()).count() << "\n";
        std::ofstream myfile;
        myfile.open("DRTiming.csv", std::ios::app);
        myfile << s.str();
        myfile.close();
        */
      }
    } else if ( State::DATAREADY == state ) {
      result = true;
    } else if ( State::SCHEDULED == state ) {
      result = true;
    }

    return result;
  }

  //---------------------------------------------------------------------------
  bool AlgorithmNode::dataDependenciesSatisfied( const int& slotNum ) const
  {

    bool result = true; //return true if an algorithm has no data inputs
    auto& states = m_graph->getAlgoStates( slotNum );

    for ( auto dataNode : m_inputs ) {
      // return false if the input has no producers at all (normally this case must be
      // forbidden, and must be invalidated at configuration time)
      result = false;
      for ( auto algoNode : dataNode->getProducers() )
        if ( State::EVTACCEPTED == states[algoNode->getAlgoIndex()] ) {
          result = true;
          break; // skip checking other producers if one was found to be executed
        }

      if (!result) break; // skip checking other inputs if this input was not produced yet
    }

    return result;
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
  void AlgorithmNode::updateDecision( const int& slotNum, AlgsExecutionStates& states, std::vector<int>& node_decisions,
                                      const AlgorithmNode* /*requestor*/ ) const
  {

    const State& state = states[m_algoIndex];
    int decision       = -1;
    //requestor          = this;

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

    if ( -1 != decision ) {
      auto& slot = (*m_graph->m_eventSlots)[slotNum];
      auto promoter = DataReadyPromoter(slot);
      for ( auto output : m_outputs )
        for ( auto consumer : output->getConsumers() )
          if (State::CONTROLREADY == states[consumer->getAlgoIndex()])
            consumer->accept(promoter);

      auto vis = concurrency::Supervisor(slot);
      for ( auto p : m_parents ) {
        //p->updateDecision( slotNum, states, node_decisions, requestor );
        p->accept(vis);
      }

    }
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
  StatusCode PrecedenceRulesGraph::initialize( const std::unordered_map<std::string, unsigned int>& algname_index_map )
  {

    m_headNode->initialize( algname_index_map );
    // StatusCode sc = buildDataDependenciesRealm();
    StatusCode sc = buildAugmentedDataDependenciesRealm();

    if ( !sc.isSuccess() ) error() << "Could not build the data dependency realm." << endmsg;

    return sc;
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::initialize( const std::unordered_map<std::string, unsigned int>& algname_index_map,
                                             std::vector<EventSlot>& eventSlots )
  {

    m_eventSlots = &eventSlots;
    m_headNode->initialize( algname_index_map );
    // StatusCode sc = buildDataDependenciesRealm();
    StatusCode sc = buildAugmentedDataDependenciesRealm();

    if ( !sc.isSuccess() ) error() << "Could not build the data dependency realm." << endmsg;

    if (msgLevel(MSG::DEBUG))
      debug() << dumpDataFlow() << endmsg;

    return sc;
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::registerIODataObjects( const Algorithm* algo )
  {

    const std::string& algoName = algo->name();

    DataObjIDColl inputObjs, outputObjs;
    DHHVisitor avis( inputObjs, outputObjs );
    algo->acceptDHVisitor( &avis );

    m_algoNameToAlgoInputsMap[algoName] = inputObjs;
    m_algoNameToAlgoOutputsMap[algoName] = outputObjs;

    if (msgLevel(MSG::DEBUG)) {
      debug() << "Inputs of " << algoName << ": ";
      for (auto tag : inputObjs)
        debug() << tag << " | ";
      debug() << endmsg;

      debug() << "Outputs of " << algoName << ": ";
      for (auto tag : outputObjs)
        debug() << tag << " | ";
      debug() << endmsg;
    }
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::buildDataDependenciesRealm()
  {

    StatusCode global_sc( StatusCode::SUCCESS );

    for ( auto algo : m_algoNameToAlgoNodeMap ) {

      auto targetNode = m_algoNameToAlgoNodeMap[algo.first];

      // Find producers for all the inputs of the target node
      auto& targetInCollection = m_algoNameToAlgoInputsMap[algo.first];
      for (auto inputTag : targetInCollection) {
        for (auto producer : m_algoNameToAlgoOutputsMap) {
          auto& outputs = m_algoNameToAlgoOutputsMap[producer.first];
          for (auto outputTag : outputs) {
            if (inputTag == outputTag) {
              auto& known_producers = targetNode->getSupplierNodes();
              auto valid_producer   = m_algoNameToAlgoNodeMap[producer.first];
              auto& known_consumers = valid_producer->getConsumerNodes();
              if ( std::find( known_producers.begin(), known_producers.end(), valid_producer ) ==
                   known_producers.end() )
                targetNode->addSupplierNode( valid_producer );
              if ( std::find( known_consumers.begin(), known_consumers.end(), targetNode ) == known_consumers.end() )
                valid_producer->addConsumerNode( targetNode );
            }
          }
        }
      }

      // Find consumers for all the outputs of the target node
      auto& targetOutCollection = m_algoNameToAlgoOutputsMap[algo.first];
      for (auto outputTag : targetOutCollection) {
        for (auto consumer : m_algoNameToAlgoInputsMap) {
          auto& inputs = m_algoNameToAlgoInputsMap[consumer.first];
          for (auto inputTag : inputs) {
            if (inputTag == outputTag) {
              auto& known_consumers = targetNode->getConsumerNodes();
              auto valid_consumer   = m_algoNameToAlgoNodeMap[consumer.first];
              auto& known_producers = valid_consumer->getSupplierNodes();
              if ( std::find( known_producers.begin(), known_producers.end(), targetNode ) == known_producers.end() )
                valid_consumer->addSupplierNode( targetNode );
              if ( std::find( known_consumers.begin(), known_consumers.end(), valid_consumer ) ==
                   known_consumers.end() )
                targetNode->addConsumerNode( valid_consumer );
            }
          }
        }
      }
    }
    return global_sc;
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::buildAugmentedDataDependenciesRealm()
  {

    StatusCode global_sc( StatusCode::SUCCESS, true );

    // Create the DataObjects (DO) realm (represented by DataNodes in the graph),
    // connected to DO producers (AlgorithmNodes)
    for (auto algo : m_algoNameToAlgoNodeMap) {

      auto& outCollection = m_algoNameToAlgoOutputsMap[algo.first];
      for (auto outputTag : outCollection) {
        const auto sc = addDataNode(outputTag);
        if (!sc.isSuccess()) {
          error() << "Extra producer (" << algo.first << ") for DataObject @ "
                  << outputTag
                  << " has been detected: this is not allowed." << endmsg;
          global_sc = sc;
        }
        auto dataNode = getDataNode(outputTag);
        dataNode->addProducerNode(algo.second);
        algo.second->addOutputDataNode(dataNode);
      }
    }

    // Connect previously created DO realm to DO consumers (AlgorithmNodes)
    for ( auto algo : m_algoNameToAlgoNodeMap ) {
      auto& inCollection = m_algoNameToAlgoInputsMap[algo.first];
      for (auto inputTag : inCollection) {
        DataNode* dataNode = nullptr;
        auto primaryPath = inputTag;
        auto itP = m_dataPathToDataNodeMap.find(primaryPath);
        if (itP != m_dataPathToDataNodeMap.end()) {
          dataNode = getDataNode(primaryPath);
          //if (!inCollection[inputTag].alternativeDataProductNames().empty())
          //  warning() << "Dropping all alternative data dependencies in the graph, but '" << primaryPath
          //            << "', for algorithm " << algo.first << endmsg;
          //} else {
          //  for (auto alterPath : inCollection[inputTag].alternativeDataProductNames()) {
          //    auto itAP = m_dataPathToDataNodeMap.find(alterPath);
          //    if (itAP != m_dataPathToDataNodeMap.end()) {
          //      dataNode = getDataNode(alterPath);
          //      warning() << "Dropping all alternative data dependencies in the graph, but '" << alterPath
          //                << "', for algorithm " << algo.first << endmsg;
          //      break;
          //    }
          //}
        }

        if (dataNode) {
          dataNode->addConsumerNode(algo.second);
          algo.second->addInputDataNode(dataNode);
        }
      }
    }

    return global_sc;
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::addAlgorithmNode( Algorithm* algo, const std::string& parentName, bool inverted,
                                                   bool allPass )
  {

    StatusCode sc = StatusCode::SUCCESS;

    auto& algoName = algo->name();

    auto itP = m_decisionNameToDecisionHubMap.find( parentName );
    concurrency::DecisionNode* parentNode;
    if ( itP != m_decisionNameToDecisionHubMap.end() ) {
      parentNode = itP->second;
      auto itA   = m_algoNameToAlgoNodeMap.find( algoName );
      concurrency::AlgorithmNode* algoNode;
      if ( itA != m_algoNameToAlgoNodeMap.end() ) {
        algoNode = itA->second;
      } else {
        algoNode = new concurrency::AlgorithmNode( *this, m_nodeCounter, algoName, inverted, allPass, algo->isIOBound() );
        ++m_nodeCounter;
        m_algoNameToAlgoNodeMap[algoName] = algoNode;
        if (msgLevel(MSG::DEBUG))
          debug() << "AlgoNode " << algoName << " added @ " << algoNode << endmsg;
        registerIODataObjects(algo);
      }

      parentNode->addDaughterNode( algoNode );
      algoNode->addParentNode( parentNode );
    } else {
      sc = StatusCode::FAILURE;
      error() << "Decision hub node " << parentName << ", requested to be parent, is not registered."
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
      dataNode                          = new concurrency::DataNode( *this, dataPath );
      m_dataPathToDataNodeMap[dataPath] = dataNode;
      if (msgLevel(MSG::DEBUG))
        debug() << "  DataNode for " << dataPath << " added @ " << dataNode << endmsg;
      sc = StatusCode::SUCCESS;
    }

    return sc;
  }

  //---------------------------------------------------------------------------
  DataNode* PrecedenceRulesGraph::getDataNode( const DataObjID& dataPath ) const
  {

    return m_dataPathToDataNodeMap.at( dataPath );
  }

  //---------------------------------------------------------------------------
  StatusCode PrecedenceRulesGraph::addDecisionHubNode( Algorithm* decisionHubAlgo, const std::string& parentName,
		                                              bool modeConcurrent, bool modePromptDecision, bool modeOR, bool allPass)
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
            new concurrency::DecisionNode( *this, m_nodeCounter, decisionHubName, modeConcurrent, modePromptDecision, modeOR, allPass);
        ++m_nodeCounter;
        m_decisionNameToDecisionHubMap[decisionHubName] = decisionHubNode;
        if (msgLevel(MSG::DEBUG))
          debug() << "Decision hub node " << decisionHubName << " added @ " << decisionHubNode << endmsg;
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
  void PrecedenceRulesGraph::addHeadNode( const std::string& headName, bool modeConcurrent, bool modePromptDecision, bool modeOR, bool allPass)
  {

    auto itH = m_decisionNameToDecisionHubMap.find( headName );
    if ( itH != m_decisionNameToDecisionHubMap.end() ) {
      m_headNode = itH->second;
    } else {
      m_headNode = new concurrency::DecisionNode( *this, m_nodeCounter, headName, modeConcurrent, modePromptDecision, modeOR, allPass );
      ++m_nodeCounter;
      m_decisionNameToDecisionHubMap[headName] = m_headNode;
    }
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::updateEventState( AlgsExecutionStates& algo_states, std::vector<int>& node_decisions ) const
  {
    m_headNode->updateState( algo_states, node_decisions );
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::updateDecision( const std::string& algo_name, const int& slotNum,
                                           AlgsExecutionStates& /*algo_states*/, std::vector<int>& /*node_decisions*/ ) const
  {
    //if (msgLevel(MSG::DEBUG))
    //  debug() << "(UPDATING)Setting decision of algorithm " << algo_name << " and propagating it upwards.." << endmsg;
    //getAlgorithmNode( algo_name )->updateDecision( slotNum, algo_states, node_decisions );
    auto& slot = (*m_eventSlots)[slotNum];
    auto updater = DecisionUpdater(slot);
    getAlgorithmNode( algo_name )->accept(updater);
  }

  //---------------------------------------------------------------------------
  void PrecedenceRulesGraph::rankAlgorithms( IGraphVisitor& ranker ) const
  {

    info() << "Starting ranking by data outputs .. " << endmsg;
    for (auto& pair : m_algoNameToAlgoNodeMap) {
      if (msgLevel(MSG::DEBUG))
        debug() << "  Ranking " << pair.first << "... " << endmsg;
      pair.second->accept(ranker);
      if (msgLevel(MSG::DEBUG))
        debug() << "  ... rank of " << pair.first << ": " << pair.second->getRank() << endmsg;
    }
  }

  //---------------------------------------------------------------------------
  const std::vector<AlgorithmNode*> PrecedenceRulesGraph::getDataIndependentNodes() const
  {

    std::vector<AlgorithmNode*> result;

    for (auto node : m_algoNameToAlgoInputsMap) {
      DataObjIDColl collection = (node.second);
      if (collection.empty())
        result.push_back(getAlgorithmNode(node.first));
    }

    return result;
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

  void PrecedenceRulesGraph::dumpExecutionPlan()
  {
    std::ofstream myfile;
    myfile.open( "ExecutionPlan.graphml", std::ios::app );

    boost::dynamic_properties dp;
    dp.property( "name", boost::get( &boost::AlgoNodeStruct::m_name, m_ExecPlan ) );
    dp.property( "index", boost::get( &boost::AlgoNodeStruct::m_index, m_ExecPlan ) );
    dp.property( "rank", boost::get( &boost::AlgoNodeStruct::m_rank, m_ExecPlan ) );
    dp.property( "runtime", boost::get( &boost::AlgoNodeStruct::m_runtime, m_ExecPlan ) );

    boost::write_graphml( myfile, m_ExecPlan, dp );

    myfile.close();
  }

  void PrecedenceRulesGraph::addEdgeToExecutionPlan( const AlgorithmNode* u, const AlgorithmNode* v )
  {

    boost::AlgoVertex source;
    float runtime( 0. );
    if ( u == nullptr ) {
      auto itT = m_exec_plan_map.find( "ENTRY" );
      if ( itT != m_exec_plan_map.end() ) {
        source = itT->second;
      } else {
        source                   = boost::add_vertex( boost::AlgoNodeStruct( "ENTRY", -999, -999, 0 ), m_ExecPlan );
        m_exec_plan_map["ENTRY"] = source;
      }
    } else {
      auto itS = m_exec_plan_map.find( u->getNodeName() );
      if ( itS != m_exec_plan_map.end() ) {
        source = itS->second;
      } else {
        auto alg = dynamic_cast<Algorithm*>( u->getAlgorithmRepresentatives()[0] );
        if ( alg == 0 ) {
          fatal() << "could not convert IAlgorithm to Algorithm!" << endmsg;
        } else {
          try {
            const Gaudi::Details::PropertyBase& p = alg->getProperty( "AvgRuntime" );
            runtime = std::stof( p.toString() );
          } catch(...) {
            if (msgLevel(MSG::DEBUG))
              debug() << "no AvgRuntime for " << alg->name() << endmsg;
            runtime = 1.;
          }
        }
        source = boost::add_vertex( boost::AlgoNodeStruct( u->getNodeName(), u->getAlgoIndex(), u->getRank(), runtime ),
                                    m_ExecPlan );
        m_exec_plan_map[u->getNodeName()] = source;
      }
    }

    boost::AlgoVertex target;
    auto itP = m_exec_plan_map.find( v->getNodeName() );
    if ( itP != m_exec_plan_map.end() ) {
      target = itP->second;
    } else {
      auto alg = dynamic_cast<Algorithm*>( v->getAlgorithmRepresentatives()[0] );
      if ( alg == 0 ) {
        fatal() << "could not convert IAlgorithm to Algorithm!" << endmsg;
      } else {
        try {
          const Gaudi::Details::PropertyBase& p = alg->getProperty( "AvgRuntime" );
          runtime = std::stof( p.toString() );
        } catch(...) {
          if (msgLevel(MSG::DEBUG))
            debug() << "no AvgRuntime for " << alg->name() << endmsg;
          runtime = 1.;
        }
      }
      target = boost::add_vertex( boost::AlgoNodeStruct( v->getNodeName(), v->getAlgoIndex(), v->getRank(), runtime ),
                                  m_ExecPlan );
      m_exec_plan_map[v->getNodeName()] = target;
    }

    if (msgLevel(MSG::DEBUG))
      debug() << "Edge added to execution plan" << endmsg;
    boost::add_edge(source, target, m_ExecPlan);
  }

} // namespace
