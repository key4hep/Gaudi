#ifndef GAUDIHIVE_ALGRESOURCEPOOL_H
#define GAUDIHIVE_ALGRESOURCEPOOL_H

// Include files
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h"

// TODO: include here is only a workaround
#include "ExecutionFlowGraph.h"

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
class AlgResourcePool: public extends<Service,
                                      IAlgResourcePool> {
public:
  // Standard constructor
  using extends::extends;
  // Standard destructor
  ~AlgResourcePool() override;

  virtual StatusCode start();
  virtual StatusCode initialize();
  /// Acquire a certain algorithm using its name
  virtual StatusCode acquireAlgorithm(const std::string& name, IAlgorithm*& algo, bool blocking = false);
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

  virtual StatusCode stop();

  virtual concurrency::ExecutionFlowGraph* getExecutionFlowGraph() const {return m_EFGraph;}

private:
  typedef tbb::concurrent_bounded_queue<IAlgorithm*> concurrentQueueIAlgPtr;
  typedef std::list<SmartIF<IAlgorithm> > ListAlg;
  typedef boost::dynamic_bitset<> state_type;

  std::mutex m_resource_mutex;

  state_type m_available_resources{0};
  std::map<size_t,concurrentQueueIAlgPtr*> m_algqueue_map;
  std::map<size_t,state_type> m_resource_requirements;
  std::map<size_t,size_t> m_n_of_allowed_instances;
  std::map<size_t,unsigned int> m_n_of_created_instances;
  std::map<std::string,unsigned int> m_resource_indices;

  /// Decode the top alg list
  StatusCode decodeTopAlgs();

  /// Recursively flatten an algList
  StatusCode flattenSequencer(Algorithm* sequencer, ListAlg& alglist, const std::string& parentName, unsigned int recursionDepth=0);

  BooleanProperty  m_lazyCreation {this, "CreateLazily",  false, ""};
  StringArrayProperty  m_topAlgNames {this, "TopAlg",  {},  "names of the algorithms to be passed to the algorithm manager"};

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

  /// OMG yet another hack
  concurrency::ExecutionFlowGraph* m_EFGraph = nullptr;
};

#endif  // GAUDIHIVE_ALGRESOURCEPOOL_H
