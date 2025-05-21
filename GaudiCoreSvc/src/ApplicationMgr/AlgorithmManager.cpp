/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// Include files
#include "AlgorithmManager.h"
#include <Gaudi/Algorithm.h>
#include <GaudiKernel/IAlgExecStateSvc.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/System.h>
#include <GaudiKernel/TypeNameString.h>
#ifndef _WIN32
#  include <errno.h>
#endif

/// needed when no algorithm is found or could be returned
static SmartIF<IAlgorithm> no_algorithm;

// constructor
AlgorithmManager::AlgorithmManager( IInterface* application ) : base_class( application, IAlgorithm::interfaceID() ) {
  addRef(); // Initial count set to 1
}

// addAlgorithm
StatusCode AlgorithmManager::addAlgorithm( IAlgorithm* alg ) {
  m_algs.push_back( alg );
  m_algsMap.emplace( alg->name(), alg );
  return StatusCode::SUCCESS;
}

// removeAlgorithm
StatusCode AlgorithmManager::removeAlgorithm( IAlgorithm* alg ) {
  auto it = std::find( m_algs.begin(), m_algs.end(), alg );
  if ( it == m_algs.end() ) { return StatusCode::FAILURE; }

  auto range = m_algsMap.equal_range( alg->name() );
  auto itm   = std::find_if( range.first, range.second, [&]( auto const& p ) { return p.second == alg; } );
  if ( itm == range.second ) { return StatusCode::FAILURE; }

  m_algs.erase( it );
  m_algsMap.erase( itm );
  return StatusCode::SUCCESS;
}

// createService
StatusCode AlgorithmManager::createAlgorithm( std::string algtype, std::string algname, IAlgorithm*& algorithm,
                                              bool managed, bool checkIfExists ) {
  // Check is the algorithm is already existing
  if ( checkIfExists ) {
    if ( existsAlgorithm( algname ) ) {
      // return an error because an algorithm with that name already exists
      return StatusCode::FAILURE;
    }
  }
  std::string actualalgtype( algtype );
  // a '\' in front of the type name prevents alias replacement
  if ( ( actualalgtype.size() > 8 ) && ( actualalgtype.compare( 0, 8, "unalias:" ) == 0 ) ) {
    actualalgtype = actualalgtype.substr( 8 );
  } else {
    auto typeAlias = m_algTypeAliases.find( algtype );
    if ( typeAlias != m_algTypeAliases.end() ) { actualalgtype = typeAlias->second; }
  }
  algorithm = Gaudi::Algorithm::Factory::create( actualalgtype, algname, serviceLocator().get() ).release();
  if ( !algorithm ) {
    this->error() << "Algorithm of type " << actualalgtype << " is unknown (No factory available)." << endmsg;
#ifndef _WIN32
    errno =
        0xAFFEDEAD; // code used by Gaudi for library load errors: forces getLastErrorString do use dlerror (on Linux)
#endif
    std::string err = System::getLastErrorString();
    if ( !err.empty() ) this->error() << err << endmsg;
    this->error() << "More information may be available by setting the global jobOpt \"PluginDebugLevel\" to 1"
                  << endmsg;
    return StatusCode::FAILURE;
  }
  // Check the compatibility of the version of the interface obtained
  if ( !isValidInterface( algorithm ) ) {
    fatal() << "Incompatible interface IAlgorithm version for " << actualalgtype << endmsg;
    return StatusCode::FAILURE;
  }
  m_algs.emplace_back( algorithm, managed );
  m_algsMap.emplace( algorithm->name(), algorithm );
  // let the algorithm know its type
  algorithm->setType( std::move( actualalgtype ) );
  // this is needed to keep the reference count correct, since isValidInterface(algorithm)
  // implies an increment of the counter by 1
  algorithm->release();
  StatusCode rc;
  if ( managed ) {
    // Bring the created algorithm to the target state of the ApplicationMgr
    if ( targetFSMState() >= Gaudi::StateMachine::INITIALIZED ) {
      rc = algorithm->sysInitialize();
      if ( rc.isSuccess() && targetFSMState() >= Gaudi::StateMachine::RUNNING ) { rc = algorithm->sysStart(); }
    }
    if ( !rc.isSuccess() ) { this->error() << "Failed to initialize algorithm: [" << algname << "]" << endmsg; }
  }
  return rc;
}

SmartIF<IAlgorithm>& AlgorithmManager::algorithm( const Gaudi::Utils::TypeNameString& typeName, const bool createIf ) {
  auto it = m_algsMap.find( typeName.name() );
  if ( it != m_algsMap.end() ) { // found
    return it->second;
  }
  if ( createIf ) {
    IAlgorithm* alg;
    if ( createAlgorithm( typeName.type(), typeName.name(), alg, true ).isSuccess() ) {
      return algorithm( typeName, false );
    }
  }
  return no_algorithm;
}

// existsAlgorithm
bool AlgorithmManager::existsAlgorithm( std::string_view name ) const {
  return m_algsMap.find( std::string( name ) ) != m_algsMap.end();
}

// Return the list of Algorithms
const std::vector<IAlgorithm*>& AlgorithmManager::getAlgorithms() const {
  m_listOfPtrs.clear();
  m_listOfPtrs.reserve( m_algs.size() );
  std::transform( std::begin( m_algs ), std::end( m_algs ), std::back_inserter( m_listOfPtrs ),
                  []( const AlgorithmItem& alg ) { return alg.algorithm; } );
  return m_listOfPtrs;
}

StatusCode AlgorithmManager::initialize() {
  StatusCode rc;
  for ( auto& it : m_algs ) {
    if ( !it.managed || it.algorithm->FSMState() >= Gaudi::StateMachine::INITIALIZED ) continue;
    rc = it.algorithm->sysInitialize();
    if ( rc.isFailure() ) return rc;
  }
  return rc;
}

StatusCode AlgorithmManager::start() {
  StatusCode rc;
  for ( auto& it : m_algs ) {
    if ( !it.managed || it.algorithm->FSMState() >= Gaudi::StateMachine::RUNNING ) continue;
    rc = it.algorithm->sysStart();
    if ( rc.isFailure() ) return rc;
  }
  return rc;
}

StatusCode AlgorithmManager::stop() {
  StatusCode rc;
  for ( auto& it : m_algs ) {
    if ( !it.managed ) continue;
    rc = it.algorithm->sysStop();
    if ( rc.isFailure() ) return rc;
  }
  return rc;
}

StatusCode AlgorithmManager::finalize() {
  StatusCode rc;
  auto       it = m_algs.begin();
  while ( it != m_algs.end() ) { // finalize and remove from the list the managed algorithms
    if ( it->managed ) {
      auto range = m_algsMap.equal_range( it->algorithm->name() );
      auto itm = std::find_if( range.first, range.second, [&]( auto const& p ) { return p.second == it->algorithm; } );
      if ( itm == range.second ) { return StatusCode::FAILURE; }

      rc = it->algorithm->sysFinalize();
      if ( rc.isFailure() ) return rc;

      it = m_algs.erase( it );
      m_algsMap.erase( itm );
    } else {
      ++it;
    }
  }
  return rc;
}

StatusCode AlgorithmManager::reinitialize() {
  StatusCode rc;
  for ( auto& it : m_algs ) {
    if ( !it.managed ) continue;
    rc = it.algorithm->sysReinitialize();
    if ( rc.isFailure() ) {
      this->error() << "Unable to re-initialize algorithm: " << it.algorithm->name() << endmsg;
      return rc;
    }
  }
  return rc;
}

StatusCode AlgorithmManager::restart() {
  SmartIF<IAlgExecStateSvc> m_aess;
  m_aess = serviceLocator()->service( "AlgExecStateSvc" );
  if ( !m_aess.isValid() ) {
    fatal() << "Error retrieving AlgExecStateSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  StatusCode rc;

  for ( auto& it : m_algs ) {
    if ( !it.managed ) continue;
    rc = it.algorithm->sysRestart();
    m_aess->resetErrorCount( it.algorithm );
    if ( rc.isFailure() ) {
      this->error() << "Unable to re-initialize algorithm: " << it.algorithm->name() << endmsg;
      return rc;
    }
  }
  return rc;
}

void AlgorithmManager::outputLevelUpdate() {
  resetMessaging();
  for ( auto& algItem : m_algs ) {
    const auto alg = dynamic_cast<Gaudi::Algorithm*>( algItem.algorithm );
    if ( alg ) alg->resetMessaging();
  }
}
