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
  base_class(name,svc), m_available_resources(0), m_nodeCounter(0)
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
  m_available_resources.set();
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
    if (requirements.is_subset_of(m_available_resources)) {
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

StatusCode AlgResourcePool::m_flattenSequencer(Algorithm* algo, ListAlg& alglist, concurrency::DecisionNode* motherNode, unsigned int recursionDepth){
      
  std::vector<Algorithm*>* subAlgorithms = algo->subAlgorithms();
  if (subAlgorithms->empty() and not (algo->type() == "GaudiSequencer")){
    debug() << std::string(recursionDepth, ' ') << algo->name() << " is not a sequencer. Appending it" << endmsg;
    alglist.emplace_back(algo);
    motherNode->addDaughterNode(new concurrency::AlgorithmNode(m_nodeCounter,algo->name(),false,false));
    ++m_nodeCounter;
    return StatusCode::SUCCESS;
  }

  // Recursively unroll
  ++recursionDepth;
  debug() << std::string(recursionDepth, ' ') << algo->name() << " is a sequencer. Flattening it." << endmsg;  
  bool modeOR =false;
  bool allPass =false;
  bool isLazy = false;
  if ("GaudiSequencer" == algo->type()) {
    modeOR  = (algo->getProperty("ModeOR").toString() == "True")? true : false;
    allPass = (algo->getProperty("IgnoreFilterPassed").toString() == "True")? true : false;
    isLazy = (algo->getProperty("ShortCircuit").toString() == "True")? true : false;
    if (allPass) isLazy = false; // standard GaudiSequencer behaviour on all pass is to execute everything
  }
  concurrency::DecisionNode* node = new concurrency::DecisionNode(m_nodeCounter,algo->name(),modeOR,allPass,isLazy);
  ++m_nodeCounter;
  motherNode->addDaughterNode(node);

  for (Algorithm* subalgo : *subAlgorithms ){
    StatusCode sc (m_flattenSequencer(subalgo,alglist,node,recursionDepth));
    if (sc.isFailure()){
      error() << "Algorithm " << subalgo->name() << " could not be flattened" << endmsg;
      return sc;
    }
  }  
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::m_decodeTopAlgs()    {
  
  SmartIF<IAlgManager> algMan ( serviceLocator() );
  if (!algMan.isValid()){
    error() << "Algorithm manager could not be properly fetched." << endmsg;
    return StatusCode::FAILURE;  
  }
  
  // Useful lambda not to repeat ourselves --------------------------
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
  // End of lambda --------------------------------------------------
  
  StatusCode sc = StatusCode::SUCCESS;
  
  // Fill the top alg list ----
  const std::vector<std::string>& topAlgNames = m_topAlgNames.value();    
  for (auto& name : topAlgNames) {
    IAlgorithm* algo(nullptr);
    
    Gaudi::Utils::TypeNameString item(name);       
    const std::string& item_name = item.name();
    const std::string& item_type = item.type();
     
    SmartIF<IAlgorithm> algoSmartIF (algMan->algorithm(item_name,false));
    
    if (!algoSmartIF.isValid()){
      createAlg(item_type,item_name,algo); 
      algoSmartIF = algo;
    }    
    // Init and start
    algoSmartIF->sysInitialize();    
    m_topAlgList.push_back(algoSmartIF);    
  }
  // Top Alg list filled ----

  // prepare the head node for the control flow
  m_cfNode = new concurrency::DecisionNode(m_nodeCounter, "EVENT LOOP", true,true,false);
  ++m_nodeCounter;

  // Now we unroll it ----
  for (auto& algoSmartIF : m_topAlgList){    
    Algorithm* algorithm = dynamic_cast<Algorithm*> (algoSmartIF.get());
    if (!algorithm) fatal() << "Conversion from IAlgorithm to Algorithm failed" << endmsg;
    sc = m_flattenSequencer(algorithm, m_flatUniqueAlgList, m_cfNode);       
  }    
  if (outputLevel() <= MSG::DEBUG){
    debug() << "List of algorithms is: " << endmsg;
    for (auto& algo : m_flatUniqueAlgList)
      debug() << "  o " << algo->type() << "/" << algo->name() <<  endmsg;
  }  
  // DP: TODO Make algos unique: Do we need this?  
  // Unrolled --- 

  // Now let's manage the clones
  unsigned int resource_counter(0);
  std::hash<std::string> hash_function;   
  for (auto& ialgoSmartIF : m_flatUniqueAlgList) {      
    
    const std::string& item_name = ialgoSmartIF->name();
    Algorithm* algo = dynamic_cast<Algorithm*> ( ialgoSmartIF.get() );
    if (!algo) fatal() << "Conversion from IAlgorithm to Algorithm failed" << endmsg;
    const std::string& item_type = algo->type();      
    
    size_t algo_id = hash_function(item_name);    
    tbb::concurrent_queue<IAlgorithm*>* queue = new tbb::concurrent_queue<IAlgorithm*>(); 
    m_algqueue_map[algo_id] = queue;
    
    // DP TODO Do it properly with SmartIFs, also in the queues
    IAlgorithm* ialgo(ialgoSmartIF.get());    

    queue->push(ialgo);
    m_algList.push_back(ialgo);
    m_n_of_allowed_instances[algo_id] = ialgo->cardinality();
    m_n_of_created_instances[algo_id] = 1;
  
    state_type requirements(0);     
        
    for (auto resource_name : ialgo->neededResources()){
      auto ret = m_resource_indices.insert(std::pair<std::string, unsigned int>(resource_name,resource_counter));
      // insert successful means == wasn't known before. So increment counter
      if (ret.second==true) {
         ++resource_counter;
         requirements.resize(resource_counter);
      }
      // in any case the return value holds the proper product index
      requirements[ret.first->second] = true;
    }
    
    m_resource_requirements[algo_id] = requirements;
  
    // potentially create clones; if not lazy creation we have to do it now
    if (!m_lazyCreation) {
      for (unsigned int i =1, end =ialgo->cardinality();i<end; ++i){
        debug() << "type/name to create clone of: " << item_type << "/" << item_name << endmsg;
        IAlgorithm* ialgoClone(nullptr);
        createAlg(item_type,item_name,ialgoClone);      
        queue->push(ialgoClone);
        m_n_of_created_instances[algo_id]+=1;
      } 
    }        
  }

  // Now resize all the requirement bitsets to the same size
  for (auto& kv :  m_resource_requirements) {
    kv.second.resize(resource_counter);
  }
  // Set all resources to be available
  m_available_resources.resize(resource_counter,true);

  // DP TODO: check if init/start is really necessary (gaudi state machine state check in algman...)
  algMan->initialize();
  algMan->start();    
  
  return sc;
}

//---------------------------------------------------------------------------

std::list<IAlgorithm*> AlgResourcePool::getFlatAlgList(){
  m_flatUniqueAlgPtrList.clear();
  for (auto algoSmartIF :m_flatUniqueAlgList )
    m_flatUniqueAlgPtrList.push_back(const_cast<IAlgorithm*>(algoSmartIF.get()));
  return m_flatUniqueAlgPtrList;
}

//---------------------------------------------------------------------------

std::list<IAlgorithm*> AlgResourcePool::getTopAlgList(){
  m_topAlgPtrList.clear();
  for (auto algoSmartIF :m_topAlgList )
    m_topAlgPtrList.push_back(const_cast<IAlgorithm*>(algoSmartIF.get()));
  return m_topAlgPtrList;  
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::beginRun(){
  auto algBeginRun = [&] (SmartIF<IAlgorithm>& algoSmartIF) -> StatusCode {
    StatusCode sc = algoSmartIF->sysBeginRun();
    if (!sc.isSuccess()) {
      warning() << "beginRun() of algorithm " << algoSmartIF->name() << " failed" << endmsg;
    return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  };  
  // Call the beginRun() method of all top algorithms
  for (auto& algoSmartIF : m_flatUniqueAlgList ) {
    if (algBeginRun(algoSmartIF).isFailure())
      return StatusCode::FAILURE;
  }
  for (auto& algoSmartIF : m_topAlgList ) {
    if (algBeginRun(algoSmartIF).isFailure())
    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::endRun() {
  
  auto algEndRun = [&] (SmartIF<IAlgorithm>& algoSmartIF) -> StatusCode {
    StatusCode sc = algoSmartIF->sysEndRun();
    if (!sc.isSuccess()) {
      warning() << "endRun() of algorithm " << algoSmartIF->name() << " failed" << endmsg;
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  };
  // Call the beginRun() method of all top algorithms
  for (auto& algoSmartIF : m_flatUniqueAlgList ) {
    if (algEndRun(algoSmartIF).isFailure())
    return StatusCode::FAILURE;
  }
  for (auto& algoSmartIF : m_topAlgList ) {
    if (algEndRun(algoSmartIF).isFailure())
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;  
} 

//---------------------------------------------------------------------------

