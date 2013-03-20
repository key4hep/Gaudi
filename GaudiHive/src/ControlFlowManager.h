#ifndef GAUDIHIVE_CONTROLFLOWMANAGER_H
#define GAUDIHIVE_CONTROLFLOWMANAGER_H

// std includes
#include <vector>
#include <unordered_map>

// fwk includes
#include "AlgsExecutionStates.h"


// TODO: add non-lazy behaviour!

namespace concurrency {

  typedef AlgsExecutionStates::State State;

  class ControlFlowNode {
  public:
    /// Constructor
    ControlFlowNode(){};
    /// Destructor
    virtual ~ControlFlowNode(){}; 
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map){}; 
    /// Method to set algos to CONTROLREADY, if possible
    virtual int updateState(std::vector<State>& states) const = 0;
  };


  class DecisionNode : public ControlFlowNode {
  public:
    /// Constructor
    DecisionNode(bool modeOR, bool allPass, bool isLazy) : 
      ControlFlowNode(),
      m_modeOR(modeOR), m_allPass(allPass), m_isLazy(isLazy),m_daughters() 
      {};
    /// Destructor
    virtual ~DecisionNode();
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    /// Method to set algos to CONTROLREADY, if possible
    virtual int updateState(std::vector<State>& states) const;
    /// Add a daughter node
    void addDaughterNode(ControlFlowNode* node){m_daughters.push_back(node);}
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
    AlgorithmNode(std::string algoName, bool inverted, bool allPass) : 
      ControlFlowNode(),
      m_algoName(algoName),m_inverted(inverted),m_allPass(allPass) 
      {};
    /// Initialize
    virtual void initialize(const std::unordered_map<std::string,unsigned int>& algname_index_map);
    /// Method to set algos to CONTROLREADY, if possible
    virtual int updateState(std::vector<State>& states) const;
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
  void updateEventState(std::vector<State>& algo_states) const;
  /// Initialize the control flow manager
  /// It greps the topalg list and the index map for the algo names 
  void initialize(ControlFlowNode* headNode, const std::unordered_map<std::string,unsigned int>& algname_index_map); 

private:
  /// the head node of the control flow graph; may want to have multiple ones once supporting trigger paths
  ControlFlowNode* m_headNode;    
};


} // namespace concurrency


#endif
