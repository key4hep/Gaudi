// $Id: HiveAlgorithmManager.h,v 1.4 2008/06/02 14:21:35 marcocle Exp $	//
#ifndef GAUDISVC_HiveAlgorithmManager_H
#define GAUDISVC_HiveAlgorithmManager_H

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IStateful.h"
#include "GaudiKernel/ComponentManager.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/StatusCode.h"

#include <string>
#include <list>
#include <map>


// concurrency
#include "tbb/concurrent_queue.h"


// Forward declarations
class ISvcLocator;
class IMessageSvc;

/** @class HiveAlgorithmManager HiveAlgorithmManager.h

    Adapted the original implementation in order to accomodate the presence
    of several instances of the same algorithm. This is needed in order to
    process multiple events at the time.
    The internal algorithm list is replaced by a vector of concurrent queues.
    The class now also permits to bootstrap via the plugin system algorithms
    at runtime w/o knowing their type with the method
    createAlgorithm(name,algo pointer).
    The algorithms are taken over via the acquireAlgorithm and releaseAlgorithm.
    TODO:
    1) Make the mechanism to retrieve algorithms more efficient than string
    matching. Maybe hashing?
    2) Unify names, types and queues in the same container class / collection?
*/

constexpr unsigned int ALG_NUM = 1000;
constexpr unsigned int MAX_ALGOS_PER_TYPE = 100;

// Number of algorithms
class HiveAlgorithmManager : public extends1<ComponentManager, IAlgManager>{
public:

  /// typedefs and classes
  typedef tbb::concurrent_queue<IAlgorithm*> AlgConcQueue;
  typedef std::vector<AlgConcQueue*> AlgConcQueues;
  typedef std::vector<std::string> Strings;

  class NameTypeCollection{
  public:
	  NameTypeCollection(){
		  m_name_strings.reserve(ALG_NUM);
		  m_type_strings.reserve(ALG_NUM);
	  }
	  StatusCode add(const std::string& name,const std::string& type){
		  m_name_strings.push_back(name);
		  m_type_strings.push_back(type);
		  return StatusCode::SUCCESS;
	  }
	  StatusCode getType(const std::string& name, std::string& type) const {
          const int index = getIndex(name);
          if (index<0) return StatusCode::FAILURE;
          type = m_type_strings[index];
          return StatusCode::SUCCESS;
	  }

	  int getIndex(const std::string& name) const {
		  const auto iter = std::find(m_name_strings.begin(), m_name_strings.end(), name);
		  const unsigned int index = std::distance(m_name_strings.begin(), iter);
		  const int mone = -1;
		  return index ==  m_name_strings.size()? mone : index;
	  }

	  const std::string& getName(const unsigned int i) const{
		  return m_name_strings[i];
	  }
	  const std::string& getType(const unsigned int i) const{
		  return m_type_strings[i];
	  }

	  void reset(const int index){
		  m_name_strings[index]=="";
		  m_type_strings[index]=="";
	  }
	  size_t size()const {
		  return m_name_strings.size();
	  }
  private:
	  // Collection of algorithm types names
	  Strings m_name_strings;
	  // Collection of algorithm types names
	  Strings m_type_strings;
    };


  /// default creator
  HiveAlgorithmManager( IInterface* iface );
  /// virtual destructor
  virtual ~HiveAlgorithmManager();

  /// implementation of IAlgManager::addAlgorithm
  virtual StatusCode addAlgorithm(IAlgorithm* alg);
  /// implementation of IAlgManager::removeAlgorithm
  virtual StatusCode removeAlgorithm(IAlgorithm* alg);
  /// implementation of IAlgManager::createAlgorithm
  virtual StatusCode createAlgorithm(const std::string& algtype, const std::string& algname,
                                     IAlgorithm*& algorithm, bool managed = true);

  /// Bootstrap an algorithm from its name
  StatusCode createAlgorithm(const std::string& algname,
                               IAlgorithm*& algorithm);

  /// Bootstrap an algorithm from its index
  StatusCode createAlgorithm(const unsigned int index,
                             IAlgorithm*& ialgorithm){
      StatusCode sc = createAlgorithm(m_name_type_collection.getType(index),
                                      m_name_type_collection.getName(index),
                                      ialgorithm);
      return sc;
  }

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
    static std::string _name = "HiveAlgorithmManager";
    return _name;
  }

  virtual SmartIF<IAlgorithm> &algorithm(const Gaudi::Utils::TypeNameString &typeName, const bool createIf = true);

  // For the concurrency (minimal public interface)
  StatusCode acquireAlgorithm(const std::string& name,IAlgorithm*& algo, bool createIfAbsent=false){
	  const int index = m_name_type_collection.getIndex(name);
	  if ( index < 0 )
		  fatal() << "Error in acquiring " << name
		          << ": algorithm does not exist in HiveAlgorithmManager!" << endmsg;
	  return acquireAlgorithm(index, algo, createIfAbsent);
  }

  StatusCode acquireAlgorithm(const unsigned int index,IAlgorithm*& algo, bool createIfAbsent=false){
      StatusCode sc = m_alg_conc_queues[index]->try_pop(algo);
      if (createIfAbsent)
          while(!sc.isSuccess()){
              createAlgorithm(index,algo);
               sc = m_alg_conc_queues[index]->try_pop(algo);
          }
      return sc;
  }

  void releaseAlgorithm(const std::string& name,IAlgorithm*& algo){
	  const int index = m_name_type_collection.getIndex(name);
	  if ( index < 0 )
		  fatal() << "Error in releasing " << name
		          << ": algorithm does not exist in HiveAlgorithmManager!" << endmsg;
	  releaseAlgorithm(index, algo);
  }

  void releaseAlgorithm(const unsigned int index,IAlgorithm*& algo){

	  m_alg_conc_queues[index]->push(algo);
  }

  // Dump the queues for DEBUGGING
  void dump() const;

private:
  AlgConcQueues      m_alg_conc_queues;     ///< Algorithms queues of the HiveAlgorithmManager

  /// List of pointers to the know services used to implement getAlgorithms()
  mutable std::list<IAlgorithm*> m_listOfPtrs;
  NameTypeCollection m_name_type_collection;

  // Added for the concurrency

  // Remove algorithms in a queue
  StatusCode m_removeAlgorithm(const int index);

  StatusCode m_addAlgorithm(IAlgorithm* alg,
                            const std::string& name,
                            const std::string& type );

};
#endif  // GAUDISVC_ALGORITHMFACTORY_H

