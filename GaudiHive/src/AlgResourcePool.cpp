// Include Files

// Framework
#include "AlgResourcePool.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/ThreadGaudi.h"
#include "GaudiAlg/GaudiSequencer.h"

// C++
#include <functional>
#include <queue>

// DP TODO: Manage smartifs and not pointers to algos

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(AlgResourcePool)

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define DEBUG_MSG ON_DEBUG debug()

//---------------------------------------------------------------------------

// destructor
AlgResourcePool::~AlgResourcePool() {

  for (auto& algoId_algoQueue : m_algqueue_map){
    auto* queue = algoId_algoQueue.second;
    delete queue;
  }

  delete m_PRGraph;
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

  // Prepare empty graph of precedence rules
  const std::string& name = "PrecedenceRulesGraph";
  SmartIF<ISvcLocator> svc = serviceLocator();
  m_PRGraph = new concurrency::PrecedenceRulesGraph(name, svc);

  sc = decodeTopAlgs();
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

StatusCode AlgResourcePool::acquireAlgorithm(const std::string& name, IAlgorithm*& algo, bool blocking){

  std::hash<std::string> hash_function;
  size_t algo_id = hash_function(name);
  auto itQueueIAlgPtr = m_algqueue_map.find(algo_id);

  if (itQueueIAlgPtr == m_algqueue_map.end()) {
    error() << "Algorithm " << name << " requested, but not recognised"
            << endmsg;
    algo = nullptr;
    return StatusCode::FAILURE;
  }

  StatusCode sc;
  if (blocking) {
    itQueueIAlgPtr->second->pop(algo);
    sc = StatusCode::SUCCESS;
  } else {
    sc = itQueueIAlgPtr->second->try_pop(algo);
  }

  // Note that reentrant algos are not consumed so we put them
  // back immediately in the queue at the end of this function.
  // Now we may still be called again in between and get this
  // error. In such a case, the Scheduler will retry later.
  // This is of course not optimal, but should only happen very
  // seldom and thud won't affect the global efficiency
  if(sc.isFailure())
    DEBUG_MSG << "No instance of algorithm " << name << " could be retrieved in non-blocking mode" << endmsg;

  // if (m_lazyCreation ) {
  //    TODO: fill the lazyCreation part
  // }
  if (sc.isSuccess()){
    state_type requirements = m_resource_requirements[algo_id];
    m_resource_mutex.lock();
    if (requirements.is_subset_of(m_available_resources)) {
      m_available_resources^=requirements;
    } else {
      sc = StatusCode::FAILURE;
      error() << "Failure to allocate resources of algorithm " << name << endmsg;
      // in case of not reentrant, push it back. Reentrant ones are pushed back
      // in all cases further down
      if (0 != algo->cardinality()) {
        itQueueIAlgPtr->second->push(algo);
      }
    }
    m_resource_mutex.unlock();
    if (0 == algo->cardinality()) {
      // push back reentrant algos immediately as it can be reused
      itQueueIAlgPtr->second->push(algo);
    }
  }
  return sc;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::releaseAlgorithm(const std::string& name, IAlgorithm*& algo){

  std::hash<std::string> hash_function;
  size_t algo_id = hash_function(name);

  // release resources used by the algorithm
  m_resource_mutex.lock();
  m_available_resources|= m_resource_requirements[algo_id];
  m_resource_mutex.unlock();

  // release algorithm itself if not reentrant
  if (0 != algo->cardinality()) {
    m_algqueue_map[algo_id]->push(algo);
  }
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

StatusCode AlgResourcePool::flattenSequencer(Algorithm* algo, ListAlg& alglist, const std::string& parentName, unsigned int recursionDepth){

  StatusCode sc = StatusCode::SUCCESS;

  bool isGaudiSequencer(false);
  bool isAthSequencer(false);

  if (algo->hasProperty("Members")) {
    if (algo->hasProperty("ShortCircuit"))
      isGaudiSequencer = true;
    else if (algo->hasProperty("StopOverride"))
      isAthSequencer = true;
  }

  std::vector<Algorithm*>* subAlgorithms = algo->subAlgorithms();
  if ( //we only want to add basic algorithms -> have no subAlgs
       // and exclude the case of empty sequencers
       (subAlgorithms->empty() && !(isGaudiSequencer || isAthSequencer)) ) {

    DEBUG_MSG << std::string(recursionDepth, ' ') << algo->name()
              << " is not a sequencer. Appending it" << endmsg;

    alglist.emplace_back(algo);
    m_PRGraph->addAlgorithmNode(algo, parentName, false, false).ignore();
    return sc;
  }

  // Recursively unroll
  ++recursionDepth;
  DEBUG_MSG << std::string(recursionDepth, ' ') << algo->name() << " is a sequencer. Flattening it." << endmsg;
  bool modeOR = false;
  bool allPass = false;
  bool isLazy = false;
  bool isSequential = false;

  if ( isGaudiSequencer ) {
    modeOR  = (algo->getProperty("ModeOR").toString() == "True")? true : false;
    allPass = (algo->getProperty("IgnoreFilterPassed").toString() == "True")? true : false;
    isLazy = (algo->getProperty("ShortCircuit").toString() == "True")? true : false;
    if (allPass) isLazy = false; // standard GaudiSequencer behavior on all pass is to execute everything
    isSequential = (algo->hasProperty("Sequential") &&
                   (algo->getProperty("Sequential").toString() == "True") );
  } else if (isAthSequencer ) {
    modeOR  = (algo->getProperty("ModeOR").toString() == "True")? true : false;
    allPass = (algo->getProperty("IgnoreFilterPassed").toString() == "True")? true : false;
    isLazy = (algo->getProperty("StopOverride").toString() == "True")? false : true;
    isSequential = (algo->hasProperty("Sequential") &&
                   (algo->getProperty("Sequential").toString() == "True") );
  }
  sc = m_PRGraph->addDecisionHubNode(algo, parentName, !isSequential, isLazy, modeOR, allPass);
  if (sc.isFailure()) {
    error() << "Failed to add DecisionHub " << algo->name() << " to graph of precedence rules" << endmsg;
    return sc;
  }

  for (Algorithm* subalgo : *subAlgorithms ) {
    sc = flattenSequencer(subalgo,alglist,algo->name(),recursionDepth);
    if (sc.isFailure()) {
      error() << "Algorithm " << subalgo->name() << " could not be flattened" << endmsg;
      return sc;
    }
  }
  return sc;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::decodeTopAlgs()    {

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

  // start forming the graph of precedence rules by adding the head decision hub
  m_PRGraph->addHeadNode("RootDecisionHub",true,false,true,true);

  // Now we unroll it ----
  for (auto& algoSmartIF : m_topAlgList) {
    Algorithm* algorithm = dynamic_cast<Algorithm*> (algoSmartIF.get());
    if (!algorithm) fatal() << "Conversion from IAlgorithm to Algorithm failed" << endmsg;
    sc = flattenSequencer(algorithm, m_flatUniqueAlgList, "RootDecisionHub");
  }
  // stupid O(N^2) unique-ification..
  for ( auto i = begin(m_flatUniqueAlgList); i!=end(m_flatUniqueAlgList); ++i ) {
    auto n = next(i);
    while ( n!=end(m_flatUniqueAlgList) ) {
        if (*n==*i) n = m_flatUniqueAlgList.erase(n);
        else ++n;
    }
  }
  if (msgLevel(MSG::DEBUG)){
    debug() << "List of algorithms is: " << endmsg;
    for (auto& algo : m_flatUniqueAlgList)
      debug() << "  o " << algo->type() << "/" << algo->name() << " @ " << algo <<  endmsg;
  }

  // Unrolled ---

  // Now let's manage the clones
  unsigned int resource_counter(0);
  std::hash<std::string> hash_function;
  for (auto& ialgoSmartIF : m_flatUniqueAlgList) {

    const std::string& item_name = ialgoSmartIF->name();

    verbose() << "Treating resource management and clones of " << item_name << endmsg;

    Algorithm* algo = dynamic_cast<Algorithm*> ( ialgoSmartIF.get() );
    if (!algo) fatal() << "Conversion from IAlgorithm to Algorithm failed" << endmsg;
    const std::string& item_type = algo->type();

    size_t algo_id = hash_function(item_name);
    concurrentQueueIAlgPtr* queue = new concurrentQueueIAlgPtr();
    m_algqueue_map[algo_id] = queue;

    // DP TODO Do it properly with SmartIFs, also in the queues
    IAlgorithm* ialgo(ialgoSmartIF.get());

    queue->push(ialgo);
    m_algList.push_back(ialgo);
    if (ialgo->isClonable()) {
      m_n_of_allowed_instances[algo_id] = ialgo->cardinality();
    } else {
      if (! m_overrideUnClonable) {
        debug() << "Algorithm " << ialgo->name() << " is not clonable" << endmsg;
        m_n_of_allowed_instances[algo_id] = 1;
      } else {
        warning() << "Overriding UnClonability of Algorithm " << ialgo->name() 
                  << endmsg;
        m_n_of_allowed_instances[algo_id] = ialgo->cardinality();
      }
    }
    m_n_of_created_instances[algo_id] = 1;

    state_type requirements(0);

    for (auto& resource_name : ialgo->neededResources()){
      auto ret = m_resource_indices.emplace(resource_name,resource_counter);
      // insert successful means == wasn't known before. So increment counter
      if (ret.second) ++resource_counter;
      // Resize for every algo according to the found resources
      requirements.resize(resource_counter);
      // in any case the return value holds the proper product index
      requirements[ret.first->second] = true;

    }

    m_resource_requirements[algo_id] = requirements;

    // potentially create clones; if not lazy creation we have to do it now
    if (!m_lazyCreation) {
      for (unsigned int i =1, end =m_n_of_allowed_instances[algo_id];i<end; ++i){
        debug() << "type/name to create clone of: " << item_type << "/"
                << item_name << endmsg;
        IAlgorithm* ialgoClone(nullptr);
        createAlg(item_type,item_name,ialgoClone);
        ialgoClone->setIndex(i);
        if (ialgoClone->sysInitialize().isFailure()) {
          error() << "unable to initialize Algorithm clone "
                  << ialgoClone->name() << endmsg;
          sc = StatusCode::FAILURE;
          // FIXME: should we delete this failed clone?
        } else {
          queue->push(ialgoClone);
          m_n_of_created_instances[algo_id]+=1;
        }
      }
    }

    m_PRGraph->attachAlgorithmsToNodes<concurrentQueueIAlgPtr>(item_name,*queue);

  }

  // Now resize all the requirement bitsets to the same size
  for (auto& kv :  m_resource_requirements) {
    kv.second.resize(resource_counter);
  }

  // Set all resources to be available
  m_available_resources.resize(resource_counter);
  m_available_resources.set();

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
  // Call the beginRun() method of all algorithms
  for (auto& algoSmartIF : m_flatUniqueAlgList ) {
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

StatusCode AlgResourcePool::stop(){

  StatusCode stopSc = Service::stop();
  if ( ! stopSc.isSuccess() ) return stopSc;

  // sys-Stop the algos
  for (auto& ialgo : m_algList){
    stopSc = ialgo->sysStop();
    if (stopSc.isFailure()){
      error() << "Unable to stop Algorithm: " << ialgo->name() << endmsg;
      return stopSc;
    }
    }
  return StatusCode::SUCCESS;
}
