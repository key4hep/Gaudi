// Include Files

// Framework
#include "AlgResourcePool.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/ThreadGaudi.h"

// C++
#include <functional>
#include <queue>

// DP TODO: Manage smartifs and not pointers to algos

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(AlgResourcePool)

// constructor
AlgResourcePool::AlgResourcePool( const std::string& name, ISvcLocator* svc ) :
  base_class(name,svc), m_available_resources(1)
{
  declareProperty("CreateLazily", m_lazyCreation = false );
  declareProperty("TopAlg", m_topAlgNames );
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
 
  // Try to recover the topAlgList from the ApplicationManager for backward-compatibility
  if (m_topAlgNames.value().empty()){
    info() << "TopAlg list empty. Recovering the one of Application Manager" << endmsg;
    const Gaudi::Utils::TypeNameString appMgrName("ApplicationMgr/ApplicationMgr");
    SmartIF<IProperty> appMgrProps (serviceLocator()->service(appMgrName));
    m_topAlgNames.assign(appMgrProps->getProperty("TopAlg"));
  } 
    
  sc = m_decodeTopAlgs();
  if (sc.isFailure())
    warning() << "Algorithms could not be properly decoded." << endmsg;
  
  // let's assume all resources are there
  m_available_resources.set(true);
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::start(){
  
  StatusCode startSc = Service::start();
  if ( ! startSc.isSuccess() ) return startSc;
  
  // sys-Start the algos
  for (auto& ialgo : m_algList){
    startSc = ialgo->sysStart();
    if (startSc.isFailure()){
      error() << "Unable to start Algorithm: " << ialgo->name() << endmsg;
      return startSc;
      }
    }   
  return StatusCode::SUCCESS;    
}

//---------------------------------------------------------------------------  
  
StatusCode AlgResourcePool::acquireAlgorithm(const std::string& name, IAlgorithm*& algo){
  
  
  
  std::hash<std::string> hash_function;
  size_t algo_id = hash_function(name);  
  
  if (m_algqueue_map.find(algo_id) == m_algqueue_map.end()){
    error() << "Algorithm " << name << " requested, but not recognised" 
            << endmsg;
    algo=nullptr;
    return StatusCode::FAILURE;
  }
  
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

StatusCode AlgResourcePool::m_decodeTopAlgs()    {
  
  SmartIF<IAlgManager> algMan ( serviceLocator() );
  if (!algMan.isValid()){
    error() << "Algorithm manager could not be properly fetched." << endmsg;
    return StatusCode::FAILURE;  
  }
  
  // Useful lambda not to repeat ourselves
  auto createAlg = [&algMan,this] (const std::string& item_type,
                                     const std::string& item_name,  
                                     IAlgorithm*& algo){
        StatusCode createAlgSc = algMan->createAlgorithm(item_type, 
                                                         item_name, 
                                                         algo, 
                                                         true, 
                                                         false);
        if (createAlgSc.isFailure())
          this->warning() << "Algorithm " << item_type << "/" << item_name 
                          << " could not be created." << endmsg;    
    };
  // End of lambda
  
  StatusCode sc = StatusCode::SUCCESS;

  std::hash<std::string> hash_function;

  // book keeping for resources
  unsigned int resource_counter(0);  
    
  const std::vector<std::string>& algNames = m_topAlgNames.value( );
  
  for (auto& algName : algNames) {
    
    Gaudi::Utils::TypeNameString item(algName);
    
    debug() << "Decoding " << item << endmsg;
    
    // Got the type and name. Now creating the algorithm, avoiding duplicate creation.
    std::string item_name = item.name();// + getGaudiThreadIDfromName(name());
// 
    size_t algo_id = hash_function(item_name);
    tbb::concurrent_queue<IAlgorithm*>* queue = new tbb::concurrent_queue<IAlgorithm*>(); 
    m_algqueue_map[algo_id] = queue;
    
    // DP TODO Do it properly with SmartIFs, also in the queues
    IAlgorithm* algo(nullptr);    
    SmartIF<IAlgorithm> algoSmartIF (algMan->algorithm(item_name,false));
    
    if (!algoSmartIF.isValid()){
      createAlg(item.type(),item_name,algo);      
    } else {    
      algo = algoSmartIF.get();
    }
      queue->push(algo);
      m_algList.push_back(algo);
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
        createAlg(item.type(),item_name,algo);      
        queue->push(algo);
        m_n_of_created_instances[algo_id]+=1;
      } 
    }    
    
  }
  // DP TODO: check if init/start is really necessary (gaudi state machine state check in algman...)
  algMan->initialize();
  algMan->start();    
  
  return sc;
}

//---------------------------------------------------------------------------
