#include "ControlFlowManager.h"

namespace concurrency {

  //---------------------------------------------------------------------------
  std::string ControlFlowNode::stateToString(const int& stateId) const {
    if (0 == stateId ) return "FALSE";
    else if (1 == stateId ) return "TRUE";
    else return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  DecisionNode::~DecisionNode(){
    for (auto node : m_daughters) {
      delete node;
    }
  }

  //---------------------------------------------------------------------------
  void DecisionNode::initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map){
    for (auto daughter : m_daughters){ daughter->initialize(algname_index_map); }
  }

  //---------------------------------------------------------------------------
  void DecisionNode::printState(std::stringstream& output,
                                AlgsExecutionStates& states,
                                const std::vector<int>& node_decisions,
                                const unsigned int& recursionLevel) const {
    output << std::string(recursionLevel, ' ') << m_nodeName << " (" << m_nodeIndex << ")" << ", w/ decision: "
           << stateToString(node_decisions[m_nodeIndex]) << "(" << node_decisions[m_nodeIndex] <<")" << std::endl;
    for (auto daughter : m_daughters ) {
      daughter->printState(output,states,node_decisions,recursionLevel+2);
    }
  }

  //---------------------------------------------------------------------------
  int DecisionNode::updateState(AlgsExecutionStates& states,
                                std::vector<int>& node_decisions) const {
    // check whether we already had a result earlier
    //    if (-1 != node_decisions[m_nodeIndex] ) { return node_decisions[m_nodeIndex]; }
    int decision = ((m_allPass && m_isLazy) ? 1 : -1);
    bool hasUndecidedChild = false;
    for (auto daughter : m_daughters){
      if (m_isLazy && (-1 !=decision || hasUndecidedChild ) ) {
        node_decisions[m_nodeIndex] = decision;
        return decision;} // if lazy return once result is known already or we can't fully evaluate right now because one daugther decision is missing still
      auto res = daughter->updateState(states, node_decisions);
      if ( -1 == res) {hasUndecidedChild = true;}
      else if ( false == m_modeOR && res == 0 ){decision = 0;} // "and"-mode (once first result false, the overall decision is false)
      else if ( true  == m_modeOR && res == 1){decision = 1;} // "or"-mode  (once first result true, the overall decision is true)
    }
    // what to do with yet undefined answers depends on whether AND or OR mode applies
    if (!hasUndecidedChild &&  -1 == decision ) {
      // OR mode: all results known, and none true -> reject
      if ( true == m_modeOR){ decision = 0; }
      // AND mode: all results known, and no false -> accept
      else { decision = 1; }
    }
    // in all other cases I stay with previous decisions
    node_decisions[m_nodeIndex] = decision;
    if (m_allPass) decision = 1;
    return decision;
  }

  //---------------------------------------------------------------------------
  void DecisionNode::updateDecision(AlgsExecutionStates& states,
                                    std::vector<int>& node_decisions) const {
    int decision = ((m_allPass && m_isLazy) ? 1 : -1);
    bool hasUndecidedChild = false;
    //std::cout << "UPDATING DAUGHTERS of DECISION NODE: " << m_nodeName << std::endl;

    for (auto daughter : m_daughters){
      // if lazy return once result is known already or we can't fully evaluate
      // right now because one daughter decision is missing still
      //std::cout << "----UPDATING DAUGHTER: " << daughter->getNodeName() << std::endl;
      if (m_isLazy && (-1 != decision || hasUndecidedChild)) {
        node_decisions[m_nodeIndex] = decision;
        break;
        //std::cout << "LEAVING (UPDATING) DECISION NODE: " << m_nodeName << std::endl;
        //return;
      }

      // modified
      int& res = node_decisions[daughter->getNodeIndex()];
      if (-1 == res) {
        hasUndecidedChild = true;
        //daughter->promoteToControlReadyState(states, node_decisions);

        if (typeid(*daughter) != typeid(concurrency::DecisionNode)) {
          AlgorithmNode* algod = (AlgorithmNode*) daughter;
          auto& algoState = states[algod->getAlgoIndex()];
          if (State::INITIAL == algoState) {
            //std::cout << "----> UPDATING DAUGHTER STATE to CONTROLREADY: " << daughter->getNodeName() << std::endl;
            states.updateState(algod->getAlgoIndex(), State::CONTROLREADY);
            if (algod->dataDependenciesSatisfied(states))
              algod->promoteToDataReadyState(states);
          } else if (State::CONTROLREADY == algoState && algod->dataDependenciesSatisfied(states)) {
            algod->promoteToDataReadyState(states);
          }

        } else {
          daughter->updateDecision(states, node_decisions);
        }

      // "and"-mode (once first result false, the overall decision is false)
      } else if (false == m_modeOR && res == 0) {
        decision = 0;
      // "or"-mode  (once first result true, the overall decision is true)
      } else if (true  == m_modeOR && res == 1) {
        decision = 1;
      }
    }

    // what to do with yet undefined answers depends on whether AND or OR mode applies
    if (!hasUndecidedChild &&  -1 == decision ) {
      // OR mode: all results known, and none true -> reject
      if ( true == m_modeOR) {
        decision = 0;
      // AND mode: all results known, and no false -> accept
      } else {
        decision = 1;
      }
    }

    // in all other cases I stay with previous decisions
    node_decisions[m_nodeIndex] = decision;

    // propagate decision upwards through the decision graph
    if (-1 != decision)
      for (auto p : m_parents)
          p->updateDecision(states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void DecisionNode::promoteToControlReadyState(AlgsExecutionStates& states,
                                                std::vector<int>& node_decisions) const {
    //std::cout << "REACHED DECISNODE " << m_nodeName << std::endl;
    if (-1 != node_decisions[m_nodeIndex]) {
      return;
    }

    for (auto daughter : m_daughters ) {
      auto res = node_decisions[daughter->getNodeIndex()];
      if (-1 == res) {
        daughter->promoteToControlReadyState(states, node_decisions);
        if (m_isLazy) return;
      } else if (m_isLazy) {
        if ((false == m_modeOR && res == 0) || (true == m_modeOR && res == 1)) return;
      }
    }
  }

  //---------------------------------------------------------------------------
  AlgorithmNode::~AlgorithmNode() {

    for (auto node : m_outputs) {
      delete node;
    }
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map){
    m_algoIndex = algname_index_map.at(m_algoName);
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::promoteToControlReadyState(AlgsExecutionStates& states,
                                                 std::vector<int>& node_decisions) const {

    //std::cout << "REACHED ALGONODE " << m_algoName << std::endl;
    if (State::INITIAL == states[m_algoIndex]) {
      states.updateState(m_algoIndex, State::CONTROLREADY);
      if (dataDependenciesSatisfied(states))
        promoteToDataReadyState(states);
    }
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::promoteToDataReadyState(AlgsExecutionStates& states) const {

    states.updateState(m_algoIndex, State::DATAREADY);

  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::promoteToControlReadyState(AlgsExecutionStates& states) const {

    std::cout << "REACHED ALGONODE (DATACONSUMER)" << m_algoName << std::endl;
    if (State::INITIAL == states[m_algoIndex] && dataDependenciesSatisfied(states))
      states.updateState(m_algoIndex, State::CONTROLREADY);
  }

  //---------------------------------------------------------------------------
  bool AlgorithmNode::dataDependenciesSatisfied(const AlgsExecutionStates& states) const {

    bool result = true;
    /*
    for (auto algoNode : m_suppliers)
      if (State::EVTACCEPTED != states[algoNode->getAlgoIndex()]) {
        result = false;
        break;
      }
    */
    for (auto dataNode : m_inputs) {

      result = false;
      for (auto algoNode : dataNode->getProducers())
        if (State::EVTACCEPTED == states[algoNode->getAlgoIndex()]) {
          result = true;
          break;
        }

      if (!result) break;
    }

    return result;
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::printState(std::stringstream& output,
                                 AlgsExecutionStates& states,
                                 const std::vector<int>& node_decisions,
                                 const unsigned int& recursionLevel) const {
    output << std::string(recursionLevel, ' ') << m_nodeName << " (" << m_nodeIndex << ")" << ", w/ decision: "
           << stateToString(node_decisions[m_nodeIndex]) << "(" << node_decisions[m_nodeIndex] << ")"
           << ", in state: " << states[m_algoIndex] << std::endl;
  }

  //---------------------------------------------------------------------------
  int AlgorithmNode::updateState(AlgsExecutionStates& states,
                                 std::vector<int>& node_decisions) const {
    // check whether we already had a result earlier
    //    if (-1 != node_decisions[m_nodeIndex] ) { return node_decisions[m_nodeIndex]; }
    // since we reached this point in the control flow, this algorithm is supposed to run
    // if it hasn't already
    const State& state = states[m_algoIndex];
    unsigned int decision = -1;
    if (State::INITIAL == state) {states.updateState(m_algoIndex, State::CONTROLREADY);}
    // now derive the proper result to pass back
    if (true == m_allPass) {
      decision = 1;
    } else if (State::EVTACCEPTED == state) {
      decision = !m_inverted;
    } else if (State::EVTREJECTED == state) {
      decision = m_inverted;
    } else {
      decision =  -1; // result not known yet
    }
    node_decisions[m_nodeIndex] = decision;
    return decision;
  }

  //---------------------------------------------------------------------------
  void AlgorithmNode::updateDecision(AlgsExecutionStates& states,
                                     std::vector<int>& node_decisions) const {

    const State& state = states[m_algoIndex];
    int decision = -1;

    // now derive the proper result to pass back
    if (true == m_allPass) {
      decision = 1;
    } else if (State::EVTACCEPTED == state) {
      decision = !m_inverted;
    } else if (State::EVTREJECTED == state) {
      decision = m_inverted;
    } else {
      decision =  -1; // result not known yet
    }

    node_decisions[m_nodeIndex] = decision;

    if (-1 != decision) {
      for (auto output : m_outputs)
        for (auto consumer : output->getConsumers())
          if (State::CONTROLREADY == states[consumer->getAlgoIndex()] && consumer->dataDependenciesSatisfied(states))
            consumer->promoteToDataReadyState(states);

      for (auto p : m_parents)
        p->updateDecision(states, node_decisions);
    }
  }




  //---------------------------------------------------------------------------
  StatusCode ControlFlowGraph::initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map){

    m_headNode->initialize(algname_index_map);
    //StatusCode sc = buildDataDependenciesRealm();
    StatusCode sc = buildAugmentedDataDependenciesRealm();

    if (!sc.isSuccess())
      error() << "Could not build the data dependency realm." << endmsg;
    //for (auto n : getDataIndependentNodes())
    //  info() << "Independent node: " << n->getNodeName() << endmsg;

    return sc;
  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::registerIODataObjects(const Algorithm* algo) {

    const std::string& algoName = algo->name();

    const DataObjectDescriptorCollection& inputDOCollection = algo->inputDataObjects();
    m_algoNameToAlgoInputsMap[algoName] = &inputDOCollection;

    debug() << "Inputs of " << algoName << ": ";
    for (auto tag : inputDOCollection) {
      if (inputDOCollection[tag].isValid())
        debug() << inputDOCollection[tag].dataProductName() << " | ";
    }
    debug() << endmsg;

    const DataObjectDescriptorCollection& outputDOCollection = algo->outputDataObjects();
    m_algoNameToAlgoOutputsMap[algoName] = &outputDOCollection;

    debug() << "Outputs of " << algoName << ": ";
    for (auto tag : outputDOCollection) {
      if (outputDOCollection[tag].isValid())
        debug() << outputDOCollection[tag].dataProductName() << " | ";
    }
    debug() << endmsg;
  }

  //---------------------------------------------------------------------------
  StatusCode ControlFlowGraph::buildDataDependenciesRealm() {

    StatusCode global_sc(StatusCode::SUCCESS);

    for (auto algo : m_algoNameToAlgoNodeMap) {

      auto targetNode = m_algoNameToAlgoNodeMap[algo.first];

      // Find producers for all the inputs of the target node
      const DataObjectDescriptorCollection& targetInCollection = *m_algoNameToAlgoInputsMap[algo.first];
      for (auto inputTag : targetInCollection) {
        const std::string& input2Match = targetInCollection[inputTag].dataProductName();
        for (auto producer : m_algoNameToAlgoOutputsMap) {
          const DataObjectDescriptorCollection& outputs = *m_algoNameToAlgoOutputsMap[producer.first];
          for (auto outputTag : outputs) {
            if (outputs[outputTag].isValid() && outputs[outputTag].dataProductName() == input2Match) {
              const std::vector<AlgorithmNode*>& known_producers = targetNode->getSupplierNodes();
              auto valid_producer = m_algoNameToAlgoNodeMap[producer.first];
              const std::vector<AlgorithmNode*>& known_consumers = valid_producer->getConsumerNodes();
              if (std::find(known_producers.begin(),known_producers.end(),valid_producer) == known_producers.end())
                targetNode->addSupplierNode(valid_producer);
              if (std::find(known_consumers.begin(),known_consumers.end(),targetNode) == known_consumers.end())
                valid_producer->addConsumerNode(targetNode);
            }
          }
        }
      }

      // Find consumers for all the outputs of the target node
      const DataObjectDescriptorCollection& targetOutCollection = *m_algoNameToAlgoOutputsMap[algo.first];
      for (auto outputTag : targetOutCollection) {
        const std::string& output2Match = targetOutCollection[outputTag].dataProductName();
        for (auto consumer : m_algoNameToAlgoInputsMap) {
          const DataObjectDescriptorCollection& inputs = *m_algoNameToAlgoInputsMap[consumer.first];
          for (auto inputTag : inputs) {
            if (inputs[inputTag].isValid() && inputs[inputTag].dataProductName() == output2Match) {
              const std::vector<AlgorithmNode*>& known_consumers = targetNode->getConsumerNodes();
              auto valid_consumer = m_algoNameToAlgoNodeMap[consumer.first];
              const std::vector<AlgorithmNode*>& known_producers = valid_consumer->getSupplierNodes();
              if (std::find(known_producers.begin(),known_producers.end(),targetNode) == known_producers.end())
                valid_consumer->addSupplierNode(targetNode);
              if (std::find(known_consumers.begin(),known_consumers.end(),valid_consumer) == known_consumers.end())
                targetNode->addConsumerNode(valid_consumer);
            }
          }
        }
      }

    }
    return global_sc;
  }

  //---------------------------------------------------------------------------
  StatusCode ControlFlowGraph::buildAugmentedDataDependenciesRealm() {

    StatusCode global_sc(StatusCode::SUCCESS);

    // Create the DataObjects (DO) realm (represented by DataNodes in the graph), connected to DO producers (AlgorithmNodes)
    for (auto algo : m_algoNameToAlgoNodeMap) {

      StatusCode sc;
      const DataObjectDescriptorCollection& outCollection = *m_algoNameToAlgoOutputsMap[algo.first];
      for (auto outputTag : outCollection) {
        if (outCollection[outputTag].isValid()) {
          const std::string& output = outCollection[outputTag].dataProductName();
          sc = addDataNode(output);
          if (!sc.isSuccess()) {
            error() << "Extra producer (" << algo.first << ") for DataObject @ " << output
                    << " has been detected: this is not allowed." << endmsg;
            global_sc = StatusCode::FAILURE;
          }
          auto dataNode = getDataNode(output);
          dataNode->addProducerNode(algo.second);
          algo.second->addOutputDataNode(dataNode);
        }
      }
    }

    // Connect previously created DO realm to DO consumers (AlgorithmNodes)
    for (auto algo : m_algoNameToAlgoNodeMap) {
      const DataObjectDescriptorCollection& inCollection = *m_algoNameToAlgoInputsMap[algo.first];
      for (auto inputTag : inCollection) {
        if (inCollection[inputTag].isValid()) {
          DataNode* dataNode = nullptr;
          auto primaryPath = inCollection[inputTag].dataProductName();
          auto itP = m_dataPathToDataNodeMap.find(primaryPath);
          if (itP != m_dataPathToDataNodeMap.end()) {
            dataNode = getDataNode(primaryPath);
            if (!inCollection[inputTag].alternativeDataProductNames().empty())
              warning() << "Dropping all alternative data dependencies in the graph, but '" << primaryPath
                        << "', for algorithm " << algo.first << endmsg;
          } else {
            for (auto alterPath : inCollection[inputTag].alternativeDataProductNames()) {
              auto itAP = m_dataPathToDataNodeMap.find(alterPath);
              if (itAP != m_dataPathToDataNodeMap.end()) {
                dataNode = getDataNode(alterPath);
                warning() << "Dropping all alternative data dependencies in the graph, but '" << alterPath
                          << "', for algorithm " << algo.first << endmsg;
                break;
              }
            }
          }
          if (dataNode) {
            dataNode->addConsumerNode(algo.second);
            algo.second->addInputDataNode(dataNode);
          }

        }
      }
    }

    return global_sc;
  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::addAlgorithmNode(Algorithm* algo, const std::string& parentName, bool inverted, bool allPass) {

    const std::string& algoName = algo->name();

    auto itP = m_decisionNameToDecisionHubMap.find(parentName);
    concurrency::DecisionNode* parentNode = itP->second;

    auto itA = m_algoNameToAlgoNodeMap.find(algoName);
    concurrency::AlgorithmNode* algoNode;
    if ( itA != m_algoNameToAlgoNodeMap.end()) {
      algoNode = itA->second;
    } else {
      algoNode = new concurrency::AlgorithmNode(*this,m_nodeCounter,algoName,inverted,allPass);
      ++m_nodeCounter;
      m_algoNameToAlgoNodeMap[algoName] = algoNode;
      debug() << "AlgoNode " << algoName << " added @ " << algoNode << endmsg;
    }

    parentNode->addDaughterNode(algoNode);
    algoNode->addParentNode(parentNode);

    registerIODataObjects(algo);
  }

  //---------------------------------------------------------------------------
  AlgorithmNode* ControlFlowGraph::getAlgorithmNode(const std::string& algoName) const {

    return m_algoNameToAlgoNodeMap.at(algoName);
  }

  //---------------------------------------------------------------------------
  StatusCode ControlFlowGraph::addDataNode(const std::string& dataPath) {

    StatusCode sc;

    auto itD = m_dataPathToDataNodeMap.find(dataPath);
    concurrency::DataNode* dataNode;
    if ( itD != m_dataPathToDataNodeMap.end()) {
      dataNode = itD->second;
      //sc = StatusCode::FAILURE;
      sc = StatusCode::SUCCESS;
    } else {
      dataNode = new concurrency::DataNode(*this,dataPath);
      m_dataPathToDataNodeMap[dataPath] = dataNode;
      debug() << "  DataNode for " << dataPath << " added @ " << dataNode << endmsg;
      sc = StatusCode::SUCCESS;
    }

    return sc;
  }

  //---------------------------------------------------------------------------
  DataNode* ControlFlowGraph::getDataNode(const std::string& dataPath) const {

    return m_dataPathToDataNodeMap.at(dataPath);
  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::addDecisionHubNode(Algorithm* aggregateAlgo, const std::string& parentName, bool modeOR, bool allPass, bool isLazy) {

    const std::string& aggregateName = aggregateAlgo->name();

    auto itP = m_decisionNameToDecisionHubMap.find(parentName);
    concurrency::DecisionNode* parentNode = itP->second;

    auto itA = m_decisionNameToDecisionHubMap.find(aggregateName);
    concurrency::DecisionNode* aggregateNode;
    if ( itA != m_decisionNameToDecisionHubMap.end()) {
      aggregateNode = itA->second;
    } else {
      aggregateNode = new concurrency::DecisionNode(*this,m_nodeCounter,aggregateName,modeOR,allPass,isLazy);
      ++m_nodeCounter;
      m_decisionNameToDecisionHubMap[aggregateName] = aggregateNode;
      debug() << "AggregateNode " << aggregateName << " added @ " << aggregateNode << endmsg;
    }

    parentNode->addDaughterNode(aggregateNode);
    aggregateNode->addParentNode(parentNode);

  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::addHeadNode(const std::string& headName, bool modeOR, bool allPass, bool isLazy) {

    auto itH = m_decisionNameToDecisionHubMap.find(headName);
    if ( itH != m_decisionNameToDecisionHubMap.end()) {
      m_headNode = itH->second;
    } else {
      m_headNode = new concurrency::DecisionNode(*this,m_nodeCounter,headName,modeOR,allPass,isLazy);
      ++m_nodeCounter;
      m_decisionNameToDecisionHubMap[headName] = m_headNode;
    }

  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::updateEventState(AlgsExecutionStates& algo_states,
                                          std::vector<int>& node_decisions) const {
    m_headNode->updateState(algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ControlFlowGraph::updateDecision(const std::string& algo_name,
                                        AlgsExecutionStates& algo_states,
                                        std::vector<int>& node_decisions) const {
    debug() << "Setting decision of algorithm " << algo_name << " and propagating it upwards.." << endmsg;
    getAlgorithmNode(algo_name)->updateDecision(algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  std::vector<AlgorithmNode*> ControlFlowGraph::getDataIndependentNodes() const {

    std::vector<AlgorithmNode*> result;

    for (auto node : m_algoNameToAlgoInputsMap) {
      const DataObjectDescriptorCollection& collection = *(node.second);
      for (auto tag : collection)
        if (collection[tag].isValid()) {
          result.push_back(getAlgorithmNode(node.first));
          break;
        }
    }

    return result;
  }





  //---------------------------------------------------------------------------
  StatusCode ControlFlowManager::initialize(ControlFlowGraph* cf_graph,
                                            const std::unordered_map<std::string,unsigned int>& algname_index_map){
    m_CFGraph = cf_graph;
    StatusCode sc = cf_graph->initialize(algname_index_map);
    if (!sc.isSuccess())
      error() << "Could not initialize the flow graph." << endmsg;

    return sc;
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::updateEventState(AlgsExecutionStates& algo_states,
                                            std::vector<int>& node_decisions) const {
    m_CFGraph->updateEventState(algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::updateDecision(const std::string& algo_name,
                                          AlgsExecutionStates& algo_states,
                                          std::vector<int>& node_decisions) const {
    m_CFGraph->updateDecision(algo_name, algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::promoteToControlReadyState(AlgsExecutionStates& algo_states,
                                                      std::vector<int>& node_decisions) const {
    m_CFGraph->m_headNode->promoteToControlReadyState(algo_states, node_decisions);
  }

  //---------------------------------------------------------------------------
  void ControlFlowManager::promoteDataConsumersToCR(const std::string& algo_name, AlgsExecutionStates& states) const {

    const std::vector<AlgorithmNode*>& consumers = m_CFGraph->getAlgorithmNode(algo_name)->getConsumerNodes();
    //debug() << "About to promote " << consumers.size() << " data consumer(s) for " << algo_name << endmsg;
    for (auto node : consumers) {
      //debug() << "  ... about to promote consumer " << node->getNodeName() << endmsg;
      node->promoteToControlReadyState(states);
    }

  }

  //---------------------------------------------------------------------------
  bool ControlFlowManager::algoDataDependenciesSatisfied(const std::string& algo_name, const AlgsExecutionStates& states) const {
    return m_CFGraph->getAlgorithmNode(algo_name)->dataDependenciesSatisfied(states);
  }

  //---------------------------------------------------------------------------
  bool ControlFlowManager::rootDecisionResolved(const std::vector<int>& node_decisions) const {
    return (-1 != node_decisions[m_CFGraph->m_headNode->getNodeIndex()]) ? true : false;
  }

} // namespace
