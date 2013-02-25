#ifndef GAUDIHIVE_ALGRESOURCEPOOL_H
#define GAUDIHIVE_ALGRESOURCEPOOL_H

// Include files
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/Service.h" 
#include "GaudiKernel/IAlgorithm.h"

// std includes
#include <string>
#include <list>
#include <vector>
#include <bitset>
#include <atomic>
#include <mutex>

// External libs
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

private:
  typedef tbb::concurrent_queue<IAlgorithm*> concurrentQueueIAlgPtr;
  typedef std::list<SmartIF<IAlgorithm> > ListAlg;
  typedef std::bitset<1000> state_type;
  
  std::mutex m_resource_mutex;
  bool m_lazyCreation;
  state_type m_available_resources;
  std::map<size_t,concurrentQueueIAlgPtr*> m_algqueue_map;
  std::map<size_t,state_type> m_resource_requirements;
  std::map<size_t,size_t> m_n_of_allowed_instances;
  std::map<size_t,unsigned int> m_n_of_created_instances;
  std::map<std::string,unsigned int> m_resource_indices;
  StatusCode m_decodeTopAlgs();
  /// The names of the algorithms to be passed to the algorithm manager
  StringArrayProperty m_topAlgNames;
  /// The list of algorithms
  ListAlg m_algList;
 

};

#endif  // GAUDIHIVE_ALGRESOURCEPOOL_H
