#ifndef GAUDIHIVE_HIVEEVENTLOOPMGR_H
#define GAUDIHIVE_HIVEEVENTLOOPMGR_H 1

// Framework include files
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/MinimalEventLoopMgr.h"

// std includes
#include <bitset>

// include tbb
#include "tbb/concurrent_vector.h"
#include "tbb/concurrent_queue.h"

// typedef for the event and algo state
typedef std::bitset<1000> state_type;

// Forward declarations
class IIncidentSvc;
class IDataManagerSvc;
class IDataProviderSvc;

namespace tbb {
  class task_scheduler_init;
}

class HiveEventLoopMgr : public MinimalEventLoopMgr   {
public:

protected:
  /// Reference to the Event Data Service's IDataManagerSvc interface
  SmartIF<IDataManagerSvc>  m_evtDataMgrSvc;
  /// Reference to the Event Data Service's IDataProviderSvc interface
  SmartIF<IDataProviderSvc> m_evtDataSvc;
  /// Reference to the Event Selector
  SmartIF<IEvtSelector>     m_evtSelector;
  /// Event Iterator
  IEvtSelector::Context*      m_evtContext;
  /// Event selector
  std::string       m_evtsel;
  /// Reference to the Histogram Data Service
  SmartIF<IDataManagerSvc>  m_histoDataMgrSvc;
  /// Reference to the Histogram Persistency Service
  SmartIF<IConversionSvc>   m_histoPersSvc;
  /// Name of the Hist Pers type
  std::string       m_histPersName;
  /// Property interface of ApplicationMgr
  SmartIF<IProperty>        m_appMgrProperty;
  /// Flag to avoid to fire the EnvEvent incident twice in a row
  /// (and also not before the first event)
  bool              m_endEventFired;
  /// Flag to disable warning messages when using external input
  bool              m_warnings;
  
  // Variables for the concurrency  
  /// Maximum number of parallel running algorithms
  unsigned int m_max_parallel;
  /// Pointer to tbb task scheduler
  tbb::task_scheduler_init* m_tbb_scheduler_init;  
  /// Get the input and output collections
  void find_dependencies();
  /// The termination requirement
  state_type m_termination_requirement;
  /// All requirements
  std::vector<state_type> m_all_requirements;
  /// Run algos in parallel
  bool run_parallel();  
  /// Register of algorithms started
  tbb::concurrent_vector<bool> m_algos_started;
  /// Register of algorithms successfully finished
  tbb::concurrent_vector<bool> m_algos_passed;
  /// Event state recording which products are there
  state_type* m_event_state;
  /// How many algos are in flight?
  unsigned int m_algos_in_flight;
  /// list of finished algos
  tbb::concurrent_queue<unsigned int> m_done_queue;
  
public:
  /// Standard Constructor
  HiveEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  virtual ~HiveEventLoopMgr();
  /// Call-back for finished algo tasks
  void algo_has_finished(unsigned int algo_id); 
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

};
#endif // GAUDISVC_EVENTLOOPMGR_H
