// $Id: AlgorithmManager.cpp,v 1.11 2008/10/20 20:58:10 marcocle Exp $

// Include files
#include "AlgorithmManager.h"
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
AlgorithmManager::AlgorithmManager(IInterface* application):
  base_class(application, IAlgorithm::interfaceID())
{
  addRef(); // Initial count set to 1
}

// destructor
AlgorithmManager::~AlgorithmManager() {
}

// addAlgorithm
StatusCode AlgorithmManager::addAlgorithm(IAlgorithm* alg) {
  m_listalg.push_back(alg);
  return StatusCode::SUCCESS;
}

// removeAlgorithm
StatusCode AlgorithmManager::removeAlgorithm(IAlgorithm* alg) {
  ListAlg::iterator it = std::find(m_listalg.begin(), m_listalg.end(), alg);
  if (it != m_listalg.end()) {
    m_listalg.erase(it);
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

// createService
StatusCode AlgorithmManager::createAlgorithm( const std::string& algtype,
                                              const std::string& algname,
                                              IAlgorithm*& algorithm,
                                              bool managed)
{
  // Check is the algorithm is already existing
  if( existsAlgorithm( algname ) ) {
    // return an error because an algorithm with that name already exists
    return StatusCode::FAILURE;
  }
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
    m_listalg.push_back(AlgorithmItem(algorithm, managed));
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

SmartIF<IAlgorithm>& AlgorithmManager::algorithm(const Gaudi::Utils::TypeNameString &typeName, const bool createIf) {
  ListAlg::iterator it = std::find(m_listalg.begin(), m_listalg.end(), typeName.name());
  if (it != m_listalg.end()) { // found
    return it->algorithm;
  }
  if (createIf) {
    IAlgorithm* alg;
    if (createAlgorithm(typeName.type(), typeName.name(), alg, true).isSuccess()) {
      return algorithm(typeName, false);
    }
  }
  return no_algorithm;
}

// existsAlgorithm
bool AlgorithmManager::existsAlgorithm(const std::string& name) const {
  ListAlg::const_iterator it = std::find(m_listalg.begin(), m_listalg.end(), name);
  return it != m_listalg.end();
}

  // Return the list of Algorithms
const std::list<IAlgorithm*>& AlgorithmManager::getAlgorithms() const
{
  m_listOfPtrs.clear();
  for (ListAlg::const_iterator it = m_listalg.begin(); it != m_listalg.end(); ++it) {
    m_listOfPtrs.push_back(const_cast<IAlgorithm*>(it->algorithm.get()));
  }
  return m_listOfPtrs;
}

StatusCode AlgorithmManager::initialize() {
  StatusCode rc;
  ListAlg::iterator it;
  for (it = m_listalg.begin(); it != m_listalg.end(); ++it) {
    if (!it->managed) continue;
    rc = it->algorithm->sysInitialize();
    if ( rc.isFailure() ) return rc;
  }
  return rc;
}

StatusCode AlgorithmManager::start() {
  StatusCode rc;
  ListAlg::iterator it;
  for (it = m_listalg.begin(); it != m_listalg.end(); ++it) {
    if (!it->managed) continue;
    rc = it->algorithm->sysStart();
    if ( rc.isFailure() ) return rc;
  }
  return rc;
}

StatusCode AlgorithmManager::stop() {
  StatusCode rc;
  ListAlg::iterator it;
  for (it = m_listalg.begin(); it != m_listalg.end(); ++it) {
    if (!it->managed) continue;
    rc = it->algorithm->sysStop();
    if ( rc.isFailure() ) return rc;
  }
  return rc;
}

StatusCode AlgorithmManager::finalize() {
  StatusCode rc;
  ListAlg::iterator it = m_listalg.begin();
  while (it != m_listalg.end()){ // finalize and remove from the list the managed algorithms
    if (it->managed) {
      rc = it->algorithm->sysFinalize();
      if( rc.isFailure() ) return rc;
      it = m_listalg.erase(it);
    } else {
      ++it;
    }
  }
  return rc;
}

StatusCode AlgorithmManager::reinitialize() {
  StatusCode rc;
  ListAlg::iterator it;
  for (it = m_listalg.begin(); it != m_listalg.end(); ++it) {
    if (!it->managed) continue;
    rc = it->algorithm->sysReinitialize();
    if( rc.isFailure() ){
      this->error() << "Unable to re-initialize algorithm: " << it->algorithm->name() << endmsg;
      return rc;
    }
  }
  return rc;
}

StatusCode AlgorithmManager::restart() {
  StatusCode rc;
  ListAlg::iterator it;
  for (it = m_listalg.begin(); it != m_listalg.end(); ++it) {
    if (!it->managed) continue;
    rc = it->algorithm->sysRestart();
    if( rc.isFailure() ){
      this->error() << "Unable to re-initialize algorithm: " << it->algorithm->name() << endmsg;
      return rc;
    }
  }
  return rc;
}
