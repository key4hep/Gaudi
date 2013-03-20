#ifndef GAUDIHIVE_CONTROLFLOWMANAGER_H
#define GAUDIHIVE_CONTROLFLOWMANAGER_H

// std includes
#include <vector>
#include <unordered_map>

// fwk includes
#include "AlgsExecutionStates.h"


// TODO: simplify structure put into updateState

namespace concurrency {

  typedef AlgsExecutionStates::State State;

  class ControlFlowNode {
  public:
    /// Constructor
    ControlFlowNode(unsigned int& index, const std::string& name) : m_nodeIndex(index), m_nodeName(name) {};
    /// Destructor
    virtual ~ControlFlowNode(){};
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map) = 0;
    /// Method to set algos to CONTROLREADY, if possible
    virtual int updateState(std::vector<State>& states, std::vector<int>& node_results) const = 0;
    /// Print a string representing the control flow state
    virtual void printState(std::stringstream& output, const std::vector<int>& node_results, const unsigned int& recursionLevel) const = 0;
  protected:
    /// Translation between state id and name
    std::string stateToString(const int& stateId) const;
    unsigned int m_nodeIndex;
    std::string m_nodeName;
  };


  class DecisionNode : public ControlFlowNode {
  public:
    /// Constructor
    DecisionNode(unsigned int& index, const std::string& name, bool modeOR, bool allPass, bool isLazy) : 
      ControlFlowNode(index, name),
      m_modeOR(modeOR), m_allPass(allPass), m_isLazy(isLazy),m_daughters() 
      {};
    /// Destructor
    virtual ~DecisionNode();
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    /// Method to set algos to CONTROLREADY, if possible
    virtual int updateState(std::vector<State>& states, std::vector<int>& node_results) const;  
    /// Add a daughter node
    void addDaughterNode(ControlFlowNode* node){m_daughters.push_back(node);}
    /// Print a string representing the control flow state
    virtual void printState(std::stringstream& output, const std::vector<int>& node_results, const unsigned int& recursionLevel) const;
  private:
    /// Whether acting as "and" (false) or "or" node (true) 
    bool m_modeOR;
    /// Whether always passing regardless of daugther results
    bool m_allPass;
    /// Whether to evaluate lazily - i.e. whether to stop once result known
    bool  m_isLazy;
    /// All the direct daughter nodes in the tree
    std::vector<ControlFlowNode*> m_daughters;
  };


  class AlgorithmNode : public ControlFlowNode {
  public:
    AlgorithmNode(unsigned int& index, const std::string& algoName, bool inverted, bool allPass) : 
      ControlFlowNode(index, algoName),
      m_algoName(algoName),m_inverted(inverted),m_allPass(allPass) 
      {};
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    /// Method to set algos to CONTROLREADY, if possible
    virtual int updateState(std::vector<State>& states, std::vector<int>& node_results) const;
    /// Print a string representing the control flow state
    virtual void printState(std::stringstream& output, const std::vector<int>& node_results, const unsigned int& recursionLevel) const;
  private:
    /// The index of the algorithm 
    unsigned int m_algoIndex;
    /// The name of the algorithm
    std::string m_algoName;
    /// Whether the selection result is negated or not
    bool m_inverted;
    /// Whether the selection result is relevant or always "pass"
    bool m_allPass;
  };


/**@class ControlFlowManager ControlFlowManager.h GaudiHive/src/ControlFlowManager.h
 *
 *  For now one instance can be shared across events
 * 
 *  @author  Benedikt Hegner
 *  @version 1.0
 */
class ControlFlowManager{
public:
  /// Constructor
  ControlFlowManager() : m_headNode(0){};
  /// Destructor
  virtual ~ControlFlowManager(){ if (m_headNode != 0) delete m_headNode;};
  /// A little bit silly, but who cares. ;-)
  bool needsAlgorithmToRun(const unsigned int iAlgo) const;
  /// Update the state of algorithms to controlready, where possible 
  void updateEventState(std::vector<State>& algo_states, std::vector<int>& node_results) const;
  /// Initialize the control flow manager
  /// It greps the topalg list and the index map for the algo names 
  void initialize(ControlFlowNode* headNode, const std::unordered_map<std::string,unsigned int>& algname_index_map); 
  /// Print the state of the control flow for a given event
  void printEventState(std::stringstream& ss, const std::vector<int>& node_results, const unsigned int& recursionLevel) const {m_headNode->printState(ss,node_results,recursionLevel);}

private:
  /// the head node of the control flow graph; may want to have multiple ones once supporting trigger paths
  ControlFlowNode* m_headNode;    
};


} // namespace concurrency


#endif
