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
    base_class(name,svc){
}

// destructor
AlgResourcePool::~AlgResourcePool() {
}

// initialize the pool with the list of algos known to the IAlgManager  
StatusCode AlgResourcePool::initialize(){
  SmartIF<IAlgManager> algMan(serviceLocator());
  const std::list<IAlgorithm*>& algos = algMan->getAlgorithms();
  for (auto algo : algos){
    std::hash<std::string> hash_function;
    size_t algo_id = hash_function(algo->name());
    tbb::concurrent_queue<IAlgorithm*>* queue = new tbb::concurrent_queue<IAlgorithm*>();  
    m_algqueue_map[algo_id] = queue;
    queue->push(algo);    
  }
  return StatusCode::SUCCESS;
}

StatusCode AlgResourcePool::acquireAlgorithm(const std::string& name, IAlgorithm*& algo){
  std::hash<std::string> hash_function;
  size_t algo_id = hash_function(name);
  StatusCode sc = m_algqueue_map[algo_id]->try_pop(algo); //TODO: check for existence
  return sc;
  //TODO: create things on the fly
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



