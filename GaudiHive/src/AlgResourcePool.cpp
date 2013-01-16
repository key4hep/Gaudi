// $Id: HiveAlgorithmManager.cpp,v 1.11 2008/10/20 20:58:10 marcocle Exp $

// Include files
#include "AlgResourcePool.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SvcFactory.h"
#include <functional>

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(AlgResourcePool)

// constructor
AlgResourcePool::AlgResourcePool( const std::string& name, ISvcLocator* svc ) :
    base_class(name,svc)
{
  declareProperty("CreateLazily", m_lazyCreation = false );
}

// destructor
AlgResourcePool::~AlgResourcePool() {
}

// initialize the pool with the list of algos known to the IAlgManager  
StatusCode AlgResourcePool::initialize(){
  SmartIF<IAlgManager> algMan(serviceLocator());
  const std::list<IAlgorithm*>& algos = algMan->getAlgorithms();
  std::hash<std::string> hash_function;
  for (auto algo : algos){
    size_t algo_id = hash_function(algo->name());
    tbb::concurrent_queue<IAlgorithm*>* queue = new tbb::concurrent_queue<IAlgorithm*>();  
    m_algqueue_map[algo_id] = queue;
    queue->push(algo);    
    m_n_of_allowed_instances[algo_id] = algo->cardinality();
    m_n_of_created_instances[algo_id] = 1;
    m_resource_requirements[algo_id] = state_type();
    // potentially create clones; if not lazy creation we have to do it now
    if (!m_lazyCreation) {
      for (unsigned int i =1, end =algo->cardinality();i<end; ++i){
        IAlgorithm* new_algo;
        algMan->createAlgorithm(algo->type(),algo->name(), new_algo, false, false);
        queue->push(new_algo);
        m_n_of_created_instances[algo_id]+=1;
      } 
    }
  }
  // now compute the resource needs 


  return StatusCode::SUCCESS;
}

StatusCode AlgResourcePool::acquireAlgorithm(const std::string& name, IAlgorithm*& algo){
  std::hash<std::string> hash_function;
  size_t algo_id = hash_function(name);
  StatusCode sc = m_algqueue_map[algo_id]->try_pop(algo); //TODO: check for existence
  //  if (m_lazyCreation ) {
  // TODO: fill the lazyCreation part
  //}

  return sc;
  // check whether requirements are fulfilled. 
  //needed_resources = m_resource_requirements[algo_id];

  // declare resources as used
  //  m_resource_mutex.lock();
  //used_resources|= m_resource_requirements[algo_id];
  //m_resource_mutex.unlock();
  
}

StatusCode AlgResourcePool::releaseAlgorithm(const std::string& name, IAlgorithm*& algo){
  std::hash<std::string> hash_function;
  size_t algo_id = hash_function(name);
  m_algqueue_map[algo_id]->push(algo);
  // finally release resources used by the algorithm
  //m_resource_mutex.lock();
  //used_resources|= m_resource_requirements[algo_id];
  //m_resource_mutex.unlock();
  return StatusCode::SUCCESS;
 }



