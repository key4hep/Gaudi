#include "ControlFlowGraph.h"

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
  void ControlFlowGraph::initialize( const std::unordered_map<std::string, unsigned int>& algname_index_map )
  {
    m_headNode->initialize( algname_index_map );
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

  }  // namespace
} // namespace
