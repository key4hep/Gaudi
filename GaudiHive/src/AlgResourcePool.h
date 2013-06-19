#ifndef GAUDIHIVE_ALGRESOURCEPOOL_H
#define GAUDIHIVE_ALGRESOURCEPOOL_H

// Include files
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/Service.h" 
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h"

// TODO: include here is only a workaround
#include "ControlFlowManager.h"

// std includes
#include <string>
#include <list>
#include <vector>
#include <bitset>
#include <atomic>
#include <mutex>

// External libs
#include "boost/dynamic_bitset.hpp"
#include "tbb/concurrent_queue.h"

/** @class AlgResourcePool AlgResourcePool.h GaudiHive/AlgResourcePool.h

    The AlgResourcePool is a concrete implementation of the IAlgResourcePool interface.
    It either creates all instances upfront or lazily.
    Internal bookkeeping is done via hashes of the algo names.

    @author Benedikt Hegner
*/
class AlgResourcePool: public extends1<Service, IAlgResourcePool>{
public:
  // Standard constructor
  AlgResourcePool(  const std::string& name, ISvcLocator* svc );
  // Standard destructor
  ~AlgResourcePool();

  virtual StatusCode start();
  virtual StatusCode initialize();
  /// Acquire a certain algorithm using its name 
  virtual StatusCode acquireAlgorithm(const std::string& name, IAlgorithm*& algo);
  /// Release a certain algorithm 
  virtual StatusCode releaseAlgorithm(const std::string& name, IAlgorithm*& algo);
  /// Acquire a certain resource
  virtual StatusCode acquireResource(const std::string& name);
  /// Release a certrain resource 
  virtual StatusCode releaseResource(const std::string& name);
  
  virtual std::list<IAlgorithm*> getFlatAlgList();  
  virtual std::list<IAlgorithm*> getTopAlgList();
  
  virtual StatusCode beginRun();  
  
  virtual StatusCode endRun();  

  virtual concurrency::ControlFlowNode* getControlFlow() const {return m_cfNode;} 
  virtual unsigned int getControlFlowNodeCounter() const {return m_nodeCounter;}
  
private:
  typedef tbb::concurrent_queue<IAlgorithm*> concurrentQueueIAlgPtr;
  typedef std::list<SmartIF<IAlgorithm> > ListAlg;
  typedef boost::dynamic_bitset<> state_type;
  
  std::mutex m_resource_mutex;
  bool m_lazyCreation;
  state_type m_available_resources;
  std::map<size_t,concurrentQueueIAlgPtr*> m_algqueue_map;
  std::map<size_t,state_type> m_resource_requirements;
  std::map<size_t,size_t> m_n_of_allowed_instances;
  std::map<size_t,unsigned int> m_n_of_created_instances;
  std::map<std::string,unsigned int> m_resource_indices;
  
  /// Decode the top alg list
  StatusCode decodeTopAlgs();
  
  /// Recursively flatten an algList
  StatusCode flattenSequencer(Algorithm* sequencer, ListAlg& alglist, concurrency::DecisionNode* motherNode, unsigned int recursionDepth=0);
   
  /// The names of the algorithms to be passed to the algorithm manager
  StringArrayProperty m_topAlgNames;
  
  /// The list of all algorithms created withing the Pool which are not top
  ListAlg m_algList;  
  
  /// The list of top algorithms
  ListAlg m_topAlgList;
  
  /// The flat list of algorithms w/o clones
  ListAlg m_flatUniqueAlgList;
  
  /// The flat list of algorithms w/o clones which is returned
  std::list<IAlgorithm*> m_flatUniqueAlgPtrList;

  /// The top list of algorithms
  std::list<IAlgorithm*> m_topAlgPtrList;

  /// OMG this has so to be removed
  bool m_doHacks;

  /// OMG yet another hack
  concurrency::DecisionNode* m_cfNode;  
  unsigned int m_nodeCounter;
};

#endif  // GAUDIHIVE_ALGRESOURCEPOOL_H
