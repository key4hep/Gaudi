// $Id: HiveAlgorithmManager.cpp,v 1.11 2008/10/20 20:58:10 marcocle Exp $

// Include Files

// Framework
#include "AlgResourcePool.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SvcFactory.h"

// C++
#include <functional>
#include <queue>

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(AlgResourcePool)

// constructor
AlgResourcePool::AlgResourcePool( const std::string& name, ISvcLocator* svc ) :
  base_class(name,svc), m_available_resources(1)
{
  declareProperty("CreateLazily", m_lazyCreation = false );
}

//---------------------------------------------------------------------------

// destructor
AlgResourcePool::~AlgResourcePool() {
  
  for (auto& algoId_algoQueue : m_algqueue_map){
    auto* queue = algoId_algoQueue.second;
    delete queue;
  }
}

//---------------------------------------------------------------------------

// initialize the pool with the list of algos known to the IAlgManager  
StatusCode AlgResourcePool::initialize(){
  
  StatusCode sc(Service::initialize());
  if (!sc.isSuccess())
    warning () << "Base class could not be started" << endmsg;  
  
  SmartIF<IAlgManager> algMan(serviceLocator());
  const std::list<IAlgorithm*>& algos = algMan->getAlgorithms();
  std::hash<std::string> hash_function;

  // book keeping for resources
  unsigned int resource_counter(0);

  for (auto algo : algos){
    size_t algo_id = hash_function(algo->name());
    tbb::concurrent_queue<IAlgorithm*>* queue = new tbb::concurrent_queue<IAlgorithm*>();  
    m_algqueue_map[algo_id] = queue;
    queue->push(algo);    
    m_n_of_allowed_instances[algo_id] = algo->cardinality();
    m_n_of_created_instances[algo_id] = 1;
    state_type requirements(0); 
    for (auto resource_name : algo->neededResources()){
      auto ret = m_resource_indices.insert(std::pair<std::string, unsigned int>(resource_name,resource_counter));
      // insert successful means == wasn't known before. So increment counter
      if (ret.second==true) ++resource_counter;
      // in any case the return value holds the proper product index
      requirements[ret.first->second] = true;
    }
    m_resource_requirements[algo_id] = requirements;
    // potentially create clones; if not lazy creation we have to do it now
    if (!m_lazyCreation) {
      for (unsigned int i =1, end =algo->cardinality();i<end; ++i){
        IAlgorithm* new_algo(0);
        algMan->createAlgorithm(algo->type(),algo->name(), new_algo, true, false);
        // BH TODO: this explicit handling of algorithm state is 
        // neeeded as long as the MinimalEventLoopManager does management of itself
        // In the future we'd need to leave the entire state machine business up to
        // the AlgorithmManager 
        StatusCode sc = new_algo->sysInitialize();
        if( !sc.isSuccess() ) {
          error() << "Unable to initialize Algorithm: " << new_algo->name() << endmsg;
        }
        queue->push(new_algo);
        m_n_of_created_instances[algo_id]+=1;
      } 
    }
  }
  // let's assume all resources are there
  m_available_resources.set(true);
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::start(){
  return Service::start();
}

//---------------------------------------------------------------------------
  
StatusCode AlgResourcePool::stop(){
  StatusCode stopSc;
  // Loop on all algos and stop them  
  SmartIF<IAlgManager> algMan(serviceLocator());
  const std::list<IAlgorithm*>& algos = algMan->getAlgorithms();
  for (IAlgorithm* ialgoPtr: algos){
    stopSc = ialgoPtr->sysStop();
    if (stopSc.isFailure()){
      error() << "Unable to stop Algorithm: " << ialgoPtr->name() << endmsg;
      return stopSc;
    }
  } 
  return Service::stop();
}

//---------------------------------------------------------------------------  

StatusCode AlgResourcePool::finalize(){
  
  StatusCode sc;
  StatusCode scRet;
  
  // Loop on all algos and finalize them
  SmartIF<IAlgManager> algMan(serviceLocator());
  const std::list<IAlgorithm*>& algos = algMan->getAlgorithms();
  for (IAlgorithm* ialgoPtr: algos){
    sc = ialgoPtr->sysFinalize();
    if( !sc.isSuccess() ) {
      scRet = StatusCode::FAILURE;
      warning() << "Finalization of algorithm " << ialgoPtr->name() << " failed" << endmsg;
    }
  }
  
  
  // Clear the queues which contain now invalid pointers!
  for (auto& algoId_AlgoQueue : m_algqueue_map )
    algoId_AlgoQueue.second->clear();
    
  // Remove the algorithm from the manager
  for (IAlgorithm* ialgoPtr: algos){
    if (algMan->removeAlgorithm(ialgoPtr).isFailure()) {
      scRet = StatusCode::FAILURE;
      warning() << "Problems removing Algorithm " << ialgoPtr->name() << endmsg;
    }
  }
  
  // DP output streams do not need to be released. Indeed they are treated 
  // as managed algos.
  
  return scRet;
}

//---------------------------------------------------------------------------  
  
StatusCode AlgResourcePool::acquireAlgorithm(const std::string& name, IAlgorithm*& algo){
  std::hash<std::string> hash_function;
  size_t algo_id = hash_function(name);  
  StatusCode sc = m_algqueue_map[algo_id]->try_pop(algo); //TODO: check for existence
  //  if (m_lazyCreation ) {
  // TODO: fill the lazyCreation part
  //}
  if (sc.isSuccess()){
    algo->resetExecuted();
    state_type requirements = m_resource_requirements[algo_id];
    m_resource_mutex.lock();
    state_type dependencies_missing = (m_available_resources & requirements) ^ requirements;
    if (dependencies_missing == 0) {
      m_available_resources^=requirements;
    } else{ 
      sc = StatusCode::FAILURE;
      m_algqueue_map[algo_id]->push(algo);
    }
    m_resource_mutex.unlock();
  }
  return sc;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::releaseAlgorithm(const std::string& name, IAlgorithm*& algo){
  std::hash<std::string> hash_function;
  size_t algo_id = hash_function(name);
  m_algqueue_map[algo_id]->push(algo);
  // finally release resources used by the algorithm
  m_resource_mutex.lock();
  m_available_resources|= m_resource_requirements[algo_id];
  m_resource_mutex.unlock();
  return StatusCode::SUCCESS;
 }

//--------------------------------------------------------------------------- 
 
StatusCode AlgResourcePool::acquireResource(const std::string& name){
  m_resource_mutex.lock();
  m_available_resources[m_resource_indices[name]] = false;
  m_resource_mutex.unlock();
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::releaseResource(const std::string& name){
  m_resource_mutex.lock();
  m_available_resources[m_resource_indices[name]] = true;
  m_resource_mutex.unlock();
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------