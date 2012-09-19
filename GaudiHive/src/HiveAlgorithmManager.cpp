// $Id: HiveAlgorithmManager.cpp,v 1.11 2008/10/20 20:58:10 marcocle Exp $

// Include files
#include "GaudiHive/HiveAlgorithmManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/TypeNameString.h"
#include <iostream>
#ifndef _WIN32
#include <errno.h>
#endif

/// needed when no algorithm is found or could be returned
static SmartIF<IAlgorithm> no_algorithm;

using ROOT::Reflex::PluginService;

// constructor
HiveAlgorithmManager::HiveAlgorithmManager(IInterface* application):
  base_class(application, IAlgorithm::interfaceID())
{
  m_alg_conc_queues.reserve(ALG_NUM);
  addRef(); // Initial count set to 1
}

// destructor
HiveAlgorithmManager::~HiveAlgorithmManager() {
}

// addAlgorithm
StatusCode HiveAlgorithmManager::addAlgorithm(IAlgorithm* alg) {
  // Get Queue index
  const int index = m_name_type_collection.getIndex(alg->name());

  // It does not make sense in this case
  if (index < 0 )
	  return StatusCode::FAILURE;

  // Update queues
  m_alg_conc_queues[index]->push(alg);

  return StatusCode::SUCCESS;
}

StatusCode HiveAlgorithmManager::m_addAlgorithm(IAlgorithm* alg,
												const std::string& name,
												const std::string& type ){


	int index = m_name_type_collection.getIndex(alg->name());

	if (index<0){ // Queue not there
		m_alg_conc_queues.push_back(new AlgConcQueue());
		m_name_type_collection.add(name,type);
		index = m_name_type_collection.size()-1;
	}

	m_alg_conc_queues[index]->push(alg);
	return StatusCode::SUCCESS;
}


// removeAlgorithm
StatusCode HiveAlgorithmManager::removeAlgorithm(IAlgorithm* alg) {

  // Get Queue index
  const int index = m_name_type_collection.getIndex(alg->name());
  return m_removeAlgorithm(index);
}
// removeAlgorithm
StatusCode HiveAlgorithmManager::m_removeAlgorithm(const int index) {

  // If not there
  if (index < 0 )
  	  return StatusCode::FAILURE;

  // If there, drain the associated queue
  AlgConcQueue* queue = m_alg_conc_queues[index];
  IAlgorithm* algo_popped;
  while(queue->try_pop(algo_popped));
  m_name_type_collection.reset(index);
  return StatusCode::SUCCESS;
}

StatusCode HiveAlgorithmManager::createAlgorithm(const std::string& algname,
                                                   IAlgorithm*& algorithm){

	// Try to deduce the type
	std::string algtype;
	if (! m_name_type_collection.getType(algname,algtype).isSuccess() ){
		algorithm=NULL;
		std::cout << "Could not find type for " << algname << std::endl;
		return StatusCode::FAILURE;
		}
	// Now pull it out of the plugin service as usual
	return createAlgorithm(algtype,algname,algorithm,true);

}

// createAlgorithm
StatusCode HiveAlgorithmManager::createAlgorithm( const std::string& algtype,
                                              const std::string& algname,
                                              IAlgorithm*& algorithm,
                                              bool managed)
{
  // Remove the check: it does not make sense anymore!
//	  // Check is the algorithm is already existing
//  if( existsAlgorithm( algname ) ) {
//    // return an error because an algorithm with that name already exists
//    return StatusCode::FAILURE;
//  }


  algorithm = PluginService::Create<IAlgorithm*>(algtype, algname, serviceLocator().get());
  if ( !algorithm ) {
    algorithm = PluginService::CreateWithId<IAlgorithm*>(algtype, algname, serviceLocator().get());
  }
  if ( algorithm ) {
    // Check the compatibility of the version of the interface obtained
    if( !isValidInterface(algorithm) ) {
      fatal() << "Incompatible interface IAlgorithm version for " << algtype << endmsg;
      return StatusCode::FAILURE;
    }
    StatusCode rc;
    m_addAlgorithm(algorithm,algname,algtype);

    // this is needed to keep the reference count correct, since isValidInterface(algorithm)
    // implies an increment of the counter by 1
    algorithm->release();
    if ( managed ) {
      // Bring the created algorithm to the same state of the ApplicationMgr
      if (FSMState() >= Gaudi::StateMachine::INITIALIZED) {
        rc = algorithm->sysInitialize();
        if (rc.isSuccess() && FSMState() >= Gaudi::StateMachine::RUNNING) {
          rc = algorithm->sysStart();
        }
      }
      if ( !rc.isSuccess() )  {
        this->error() << "Failed to initialize algorithm: [" << algname << "]" << endmsg;
      }
    }
    return rc;
  }
  this->error() << "Algorithm of type " << algtype
                << " is unknown (No factory available)." << endmsg;
#ifndef _WIN32
  errno = 0xAFFEDEAD; // code used by Gaudi for library load errors: forces getLastErrorString do use dlerror (on Linux)
#endif
  std::string err = System::getLastErrorString();
  if (! err.empty()) {
    this->error() << err << endmsg;
  }
  this->error() << "More information may be available by setting the global jobOpt \"ReflexPluginDebugLevel\" to 1" << endmsg;

  return StatusCode::FAILURE;
}

SmartIF<IAlgorithm>& HiveAlgorithmManager::algorithm(const Gaudi::Utils::TypeNameString &typeName, const bool createIf) {
  std::cout << "[HiveAlgorithmManager::algorithm]\n";
  // Get Queue index
  const int index = m_name_type_collection.getIndex(typeName.name());

  // If there and queue not empty, return first
  IAlgorithm* algo;
  if (index>0){
	  AlgConcQueue* queue = m_alg_conc_queues[index];
	  if (!queue->empty()){
		  queue->try_pop(algo);
		  // FIXME HACK!----------------------------------------
		  SmartIF<IAlgorithm> stupidif(algo);
		  queue->push(algo);
		  return stupidif;
	  }
  }
  // if not let's create one
  if (createIf) {
    if (createAlgorithm(typeName.type(), typeName.name(), algo, true).isSuccess()) {
      return algorithm(typeName, false);
    }
  }
  return no_algorithm;
}

// existsAlgorithm
bool HiveAlgorithmManager::existsAlgorithm(const std::string& name) const {
  const int index = m_name_type_collection.getIndex( name );
  return index > 0;
}

  // Return the list of Algorithms
const std::list<IAlgorithm*>& HiveAlgorithmManager::getAlgorithms() const {
  // All the gymnastic is done not to loop over queues: very inefficient

  m_listOfPtrs.clear();

  IAlgorithm* tmp_algo;
  // Loop on queues
  for (AlgConcQueue* algo_queue : m_alg_conc_queues){
	  std::list<IAlgorithm*> tmp_algo_list;
	  // drain queue and save the algorithms
	  while(algo_queue->try_pop(tmp_algo)){
		  tmp_algo_list.push_back(tmp_algo);
		  m_listOfPtrs.push_back(tmp_algo);
	  	  }
  	  // now put the algos back in the concurrent queue
  	  for (IAlgorithm* algo : tmp_algo_list) algo_queue->push(algo);
  }
  return m_listOfPtrs;
}

StatusCode HiveAlgorithmManager::initialize() {
  StatusCode rc;
  for (auto algorithm : getAlgorithms()){
	  rc = algorithm->sysInitialize();
	  if ( rc.isFailure() ) return rc;
  	  }
  return rc;
}

StatusCode HiveAlgorithmManager::start() {
  StatusCode rc;
  for (auto algorithm : getAlgorithms()){
	  rc = algorithm->sysStart();
	  if ( rc.isFailure() ) return rc;
  	  }
  return rc;
}

StatusCode HiveAlgorithmManager::stop() {
  StatusCode rc;
  for (auto algorithm : getAlgorithms()){
	rc = algorithm->sysStop();
	if ( rc.isFailure() ) return rc;
    }
  return rc;
}


StatusCode HiveAlgorithmManager::finalize() {
	// A little involuted, could be simpler
	StatusCode rc;
	for (auto algorithm : getAlgorithms()){
		rc = algorithm->sysFinalize();
		if( rc.isFailure() ) return rc;
		const int index = m_name_type_collection.getIndex(algorithm->name());
		m_removeAlgorithm(index);
	}
	return rc;
}

StatusCode HiveAlgorithmManager::reinitialize() {
	std::cout << "[HiveAlgorithmManager::reinitialize]\n";
	StatusCode rc;
	for (auto algorithm : getAlgorithms()){
		rc = algorithm->sysReinitialize();
		if( rc.isFailure() ){
		      this->error() << "Unable to re-initialize algorithm: " << algorithm->name() << endmsg;
		      return rc;
		    }
	}
	return rc;
}

StatusCode HiveAlgorithmManager::restart() {
	std::cout << "[HiveAlgorithmManager::restart]\n";
	StatusCode rc;
	for (auto algorithm : getAlgorithms()){
		rc = algorithm->sysRestart();
		if( rc.isFailure() ){
		      this->error() << "Unable to re-start algorithm: " << algorithm->name() << endmsg;
		      return rc;
		    }
	}
	return rc;
}

#include "GaudiKernel/Algorithm.h"
void HiveAlgorithmManager::dump() const{
	always() << "Dumping the content of the HiveAlgoManager"<< endmsg;
	always() << "Total Number of Queues: " << m_alg_conc_queues.size() <<endmsg;

	IAlgorithm* tmp_algo;
	unsigned int qcounter=0;
	for (AlgConcQueue* algo_queue : m_alg_conc_queues){
		std::list<IAlgorithm*> tmp_algo_list;
		const std::string& qname = m_name_type_collection.getName(qcounter);
		const std::string& qtype = m_name_type_collection.getType(qcounter);

		// drain queue and save the algorithms
		while(algo_queue->try_pop(tmp_algo)) tmp_algo_list.push_back(tmp_algo);

		always() << "o Queue " << qcounter << " [" << qname << "/" << qtype
				 << "] size " << tmp_algo_list.size() << endmsg;

		// now put the algos back in the concurrent queue
		for (IAlgorithm* algo : tmp_algo_list){
			always() << "   - Algo " << algo->name() << " " << algo << endmsg;
//			Algorithm* tmpalgo = dynamic_cast<Algorithm*>(algo);
//			std::cout << tmpalgo->getProperty("Inputs").toString() << std::endl;
//			std::cout << tmpalgo->getProperty("Outputs").toString() << std::endl;
			algo_queue->push(algo);
		}
		qcounter++;
	}
	always() << "Dumped" << endmsg;

}
