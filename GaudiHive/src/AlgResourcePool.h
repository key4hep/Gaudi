// $Id: IAlgManager.h,v 1.7 2008/06/02 14:20:38 marcocle Exp $
#ifndef GAUDIHIVE_ALGRESOURCEPOOL_H
#define GAUDIHIVE_ALGRESOURCEPOOL_H

// Include files
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/Service.h" 
#include "GaudiKernel/IAlgorithm.h"
// std includes
#include <string>
#include <bitset>
#include <atomic>
#include <mutex>

#include "tbb/concurrent_queue.h"
typedef std::bitset<1000> state_type;

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

  virtual StatusCode initialize();
  /// Acquire a certain algorithm using its name 
  virtual StatusCode acquireAlgorithm(const std::string& name, IAlgorithm*& algo);
  /// Release a certain algorithm 
  virtual StatusCode releaseAlgorithm(const std::string& name, IAlgorithm*& algo);

private:
  std::mutex m_resource_mutex;
  state_type used_resources;
  std::map<size_t,tbb::concurrent_queue<IAlgorithm*>*> m_algqueue_map;
  std::map<size_t,state_type> m_resource_requirements;
  std::map<size_t,size_t> m_n_of_allowed_instances;
  std::map<size_t,std::atomic<unsigned int>> m_n_of_created_instances;

};

// Instantiation of a static factory class used by clients to create                                                                                                      
// instances of this service                                                                                                                                              
//DECLARE_SERVICE_FACTORY(AlgResourcePool)

#endif  // GAUDIHIVE_ALGRESOURCEPOOL_H
