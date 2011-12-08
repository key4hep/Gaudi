// $Id: AlgorithmManager.h,v 1.4 2008/06/02 14:21:35 marcocle Exp $	//
#ifndef GAUDISVC_ALGORITHMMANAGER_H
#define GAUDISVC_ALGORITHMMANAGER_H

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IStateful.h"
#include "GaudiKernel/ComponentManager.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/IAlgorithm.h"
#include <string>
#include <list>
#include <map>

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
class AlgorithmManager : public extends1<ComponentManager, IAlgManager>{
public:

  struct AlgorithmItem {
    AlgorithmItem(IAlgorithm *s, bool managed = false):
      algorithm(s), managed(managed) {}
    SmartIF<IAlgorithm> algorithm;
    bool managed;
    inline bool operator==(const std::string &name) const {
      return algorithm->name() == name;
    }
    inline bool operator==(const IAlgorithm *ptr) const {
      return algorithm.get() == ptr;
    }
  };

  /// typedefs and classes
  typedef std::list<AlgorithmItem> ListAlg;

  /// default creator
  AlgorithmManager( IInterface* iface );
  /// virtual destructor
  virtual ~AlgorithmManager();

  /// implementation of IAlgManager::addAlgorithm
  virtual StatusCode addAlgorithm(IAlgorithm* alg);
  /// implementation of IAlgManager::removeAlgorithm
  virtual StatusCode removeAlgorithm(IAlgorithm* alg);
  /// implementation of IAlgManager::createAlgorithm
  virtual StatusCode createAlgorithm(const std::string& algtype, const std::string& algname,
                                     IAlgorithm*& algorithm, bool managed = false);

  /// implementation of IAlgManager::existsAlgorithm
  virtual bool existsAlgorithm(const std::string& name) const;
  /// implementation of IAlgManager::getAlgorithms
  virtual const std::list<IAlgorithm*>& getAlgorithms() const;

  /// Initialization (from CONFIGURED to INITIALIZED).
  virtual StatusCode initialize();
  /// Start (from INITIALIZED to RUNNING).
  virtual StatusCode start();
  /// Stop (from RUNNING to INITIALIZED).
  virtual StatusCode stop();
  /// Finalize (from INITIALIZED to CONFIGURED).
  virtual StatusCode finalize();

  /// Initialization (from INITIALIZED or RUNNING to INITIALIZED, via CONFIGURED).
  virtual StatusCode reinitialize();
  /// Initialization (from RUNNING to RUNNING, via INITIALIZED).
  virtual StatusCode restart();

  /// Return the name of the manager (implementation of INamedInterface)
  const std::string &name() const {
    static std::string _name = "AlgorithmManager";
    return _name;
  }

  virtual SmartIF<IAlgorithm> &algorithm(const Gaudi::Utils::TypeNameString &typeName, const bool createIf = true);

private:
  ListAlg      m_listalg;     ///< List of algorithms maintained by AlgorithmManager

  /// List of pointers to the know services used to implement getAlgorithms()
  mutable std::list<IAlgorithm*> m_listOfPtrs;

};
#endif  // GAUDISVC_ALGORITHMFACTORY_H

