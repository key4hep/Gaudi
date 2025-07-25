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
#ifndef GAUDISVC_ALGORITHMMANAGER_H
#define GAUDISVC_ALGORITHMMANAGER_H

// Include files
#include <GaudiKernel/ComponentManager.h>
#include <GaudiKernel/IAlgManager.h>
#include <GaudiKernel/IAlgorithm.h>
#include <GaudiKernel/IStateful.h>
#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/SmartIF.h>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declarations
class ISvcLocator;
class IMessageSvc;

/** @class AlgorithmManager AlgorithmManager.h

    The AlgorithmManager class is in charge of the creation of concrete
    instances of Algorithms.
    The ApplicationMgr delegates the creation and bookkeeping of
    algorithms to the algorithm factory. In order to be able to create
    algorithms from which it does not know the concrete type it requires
    that the algorithm has been declared in one of 3 possible ways: an abstract
    static creator function, a dynamic link library or an abstract factory reference.

    @author  Pere Mato
*/
class AlgorithmManager : public extends<ComponentManager, IAlgManager> {
public:
  struct AlgorithmItem final {
    AlgorithmItem( IAlgorithm* s, bool managed = false ) : algorithm( s ), managed( managed ) {}
    IAlgorithm* algorithm;
    bool        managed;
    bool        operator==( const IAlgorithm* ptr ) const { return algorithm == ptr; }
  };

  /// typedefs and classes
  typedef std::map<std::string, std::string, std::less<>> AlgTypeAliasesMap;

  /// default creator
  AlgorithmManager( IInterface* iface );

  /// implementation of IAlgManager::addAlgorithm
  StatusCode addAlgorithm( IAlgorithm* alg ) override;
  /// implementation of IAlgManager::removeAlgorithm
  StatusCode removeAlgorithm( IAlgorithm* alg ) override;
  /// implementation of IAlgManager::createAlgorithm
  StatusCode createAlgorithm( const std::string& algtype, const std::string& algname, IAlgorithm*& algorithm,
                              bool managed = false, bool checkIfExists = true ) override;

  /// implementation of IAlgManager::existsAlgorithm
  bool existsAlgorithm( std::string_view name ) const override;
  /// implementation of IAlgManager::getAlgorithms
  std::vector<IAlgorithm*> getAlgorithms() const override;

  /// Initialization (from CONFIGURED to INITIALIZED).
  StatusCode initialize() override;
  /// Start (from INITIALIZED to RUNNING).
  StatusCode start() override;
  /// Stop (from RUNNING to INITIALIZED).
  StatusCode stop() override;
  /// Finalize (from INITIALIZED to CONFIGURED).
  StatusCode finalize() override;

  /// Initialization (from INITIALIZED or RUNNING to INITIALIZED, via CONFIGURED).
  StatusCode reinitialize() override;
  /// Initialization (from RUNNING to RUNNING, via INITIALIZED).
  StatusCode restart() override;

  /// Return the name of the manager (implementation of INamedInterface)
  const std::string& name() const override {
    static const std::string _name = "AlgorithmManager";
    return _name;
  }

  SmartIF<IAlgorithm>& algorithm( const Gaudi::Utils::TypeNameString& typeName, const bool createIf = true ) override;

  AlgTypeAliasesMap&       typeAliases() { return m_algTypeAliases; }
  const AlgTypeAliasesMap& typeAliases() const { return m_algTypeAliases; }

  /// Function to call to update the outputLevel of the components (after a change in MessageSvc).
  void outputLevelUpdate() override;

private:
  std::vector<AlgorithmItem>                                m_algs;    ///< algorithms maintained by AlgorithmManager
  std::unordered_multimap<std::string, SmartIF<IAlgorithm>> m_algsMap; ///< algorithms maintained by
                                                                       ///< AlgorithmManager

  AlgTypeAliasesMap m_algTypeAliases;
};
#endif // GAUDISVC_ALGORITHMFACTORY_H
