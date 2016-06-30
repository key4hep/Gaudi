#ifndef GAUDIHIVE_HIVEEVENTLOOPMGR_H
#define GAUDIHIVE_HIVEEVENTLOOPMGR_H 1

// Framework include files
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/MinimalEventLoopMgr.h"

// std includes
#include <atomic>

//include boost
#include <boost/dynamic_bitset.hpp>

// include tbb
#include "tbb/concurrent_vector.h"
#include "tbb/concurrent_queue.h"

// typedef for the event and algo state
typedef boost::dynamic_bitset<> state_type;

// Forward declarations
class IIncidentSvc;
class IDataManagerSvc;
class IDataProviderSvc;

namespace tbb {
  class task_scheduler_init;
}

class HiveEventLoopMgr : public MinimalEventLoopMgr   {
protected:
  // keep room for a class hashing strings instead of strings
  typedef std::vector<std::vector<std::string>> algosDependenciesCollection;

  // Properties
  StringProperty  m_histPersName {this, "HistogramPersistency",  "",  "Name of the Hist Pers type"};
  StringProperty  m_evtsel {this, "EvtSel",  "",  "Event selector"};
  BooleanProperty  m_warnings {this, "Warnings",  true,  "Set this property to false to suppress warning messages"};
  UnsignedIntegerProperty  m_max_parallel {this, "MaxAlgosParallel",  1,  "Maximum number of parallel running algorithms"};
  UnsignedIntegerProperty  m_evts_parallel {this, "MaxEventsParallel",  1,  "Number of events in parallel"};
  UnsignedIntegerProperty  m_num_threads {this, "NumThreads",  1,  "Total numbers of threads"};
  BooleanProperty  m_DumpQueues {this, "DumpQueues",  false,  "Dump the algorithm queues"};
  BooleanProperty  m_CloneAlgorithms {this, "CloneAlgorithms",  false,  "Clone algorithms to run them simultaneously"};
  PropertyWithValue<algosDependenciesCollection>  m_AlgosDependencies {this, "AlgosDependencies",  {}, ""};

  /// Reference to the Event Data Service's IDataManagerSvc interface
  SmartIF<IDataManagerSvc>  m_evtDataMgrSvc;
  /// Reference to the Event Data Service's IDataProviderSvc interface
  SmartIF<IDataProviderSvc> m_evtDataSvc;
  /// Reference to the Event Selector
  SmartIF<IEvtSelector>     m_evtSelector;
  /// Event Iterator
  IEvtSelector::Context*      m_evtContext = nullptr;
  /// Reference to the Histogram Data Service
  SmartIF<IDataManagerSvc>  m_histoDataMgrSvc;
  /// Reference to the Histogram Persistency Service
  SmartIF<IConversionSvc>   m_histoPersSvc;
  /// Reference to the Histogram Persistency Service
  SmartIF<IHiveWhiteBoard>  m_whiteboard;
  /// Reference to the Algorithm resource pool
  SmartIF<IAlgResourcePool>  m_algResourcePool;
  /// Property interface of ApplicationMgr
  SmartIF<IProperty>        m_appMgrProperty;
  /// Flag to avoid to fire the EnvEvent incident twice in a row
  /// (and also not before the first event)
  bool              m_endEventFired = false;

  /// Pointer to tbb task scheduler
  tbb::task_scheduler_init* m_tbb_scheduler_init;
  /// Get the input and output collections
  void find_dependencies();
  /// The termination requirement
  state_type m_termination_requirement;
  /// All requirements
  std::vector<state_type> m_all_requirements;
  /// Register of input products
  std::map<DataObjID,unsigned int> m_product_indices;
  /// Total number of algos in flight across all events
  std::atomic_uint m_total_algos_in_flight{0};
  /// Total number of algos
  unsigned int  m_numberOfAlgos = 0;

  // Number of products to deal with
  unsigned int m_nProducts = 0;

public:
  /// Standard Constructor
  HiveEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  virtual ~HiveEventLoopMgr();
  /// Create event address using event selector
  StatusCode getEventRoot(IOpaqueAddress*& refpAddr);

  /// implementation of IService::initialize
  virtual StatusCode initialize();
  /// implementation of IService::reinitialize
  virtual StatusCode reinitialize();
  /// implementation of IService::stop
  virtual StatusCode stop();
  /// implementation of IService::finalize
  virtual StatusCode finalize();
  /// implementation of IService::nextEvent
  virtual StatusCode nextEvent(int maxevt);
  /// implementation of IEventProcessor::executeEvent(void* par)
  virtual StatusCode executeEvent(void* par);
  /// implementation of IEventProcessor::executeRun()
  virtual StatusCode executeRun(int maxevt);

  /// Decrement the number of algos in flight and put algo back in manager - maybe private
  void taskFinished(IAlgorithm*& algo);

};
#endif // GAUDIHIVE_HIVEEVENTLOOPMGR_H
