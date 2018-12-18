#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "tbb/task.h"

// Forward declarations
class IAlgExecStateSvc;
class IEvtSelector;
class IHiveWhiteBoard;
struct IDataManagerSvc;
class IDataProviderSvc;

/** Basic event loop and scheduler for fast HLT reconstruction.
 *  It uses one fifo queue per tbb thread and schedules full events on a
 *  given thread by putting all algos in the dedicated queue from the beginning.
 */
class HLTEventLoopMgr : public extends<Service, IEventProcessor>
{

public:
  /// Standard Constructor
  using extends::extends;

  /// implementation of IService::initialize
  StatusCode initialize() override;
  /// implementation of IService::reinitialize
  StatusCode reinitialize() override { return StatusCode::FAILURE; }
  /// implementation of IService::finalize
  StatusCode finalize() override;

  /// implementation of IEventProcessor::nextEvent
  StatusCode nextEvent( int maxevt ) override;
  /// implementation of IEventProcessor::executeEvent(void* par)
  StatusCode executeEvent( void* par ) override;
  /// implementation of IEventProcessor::executeRun()
  StatusCode executeRun( int maxevt ) override { return nextEvent( maxevt ); }
  /// implementation of IEventProcessor::stopRun()
  StatusCode stopRun() override;

private:
  /// Declare the root address of the event
  StatusCode declareEventRootAddress();
  /// Method to check if an event failed and take appropriate actions
  StatusCode eventFailed( EventContext* eventContext ) const;
  /// Algorithm promotion
  struct HLTExecutionTask;
  friend HLTExecutionTask; // must be able to call `promoteToExecuted`...
  void promoteToExecuted( std::unique_ptr<EventContext> eventContext ) const;

private:
  Gaudi::Property<std::string> m_histPersName{this, "HistogramPersistency", "", ""};
  Gaudi::Property<std::string> m_evtsel{this, "EvtSel", "", ""};
  Gaudi::Property<int> m_threadPoolSize{this, "ThreadPoolSize", -1, "Size of the threadpool initialised by TBB"};
  Gaudi::Property<std::string> m_whiteboardSvcName{this, "WhiteboardSvc", "EventDataSvc", "The whiteboard name"};
  Gaudi::Property<std::string> m_dotfile{
      this, "DotFile", {}, "Name of file to dump dependency graph; if empty, do not dump"};
  Gaudi::Property<std::vector<std::string>> m_topAlgs{this, "TopAlg", {}};

  /// Reference to the Event Data Service's IDataManagerSvc interface
  IDataManagerSvc* m_evtDataMgrSvc = nullptr;
  /// Reference to the Event Selector
  IEvtSelector* m_evtSelector = nullptr;
  /// Reference to the Histogram Data Service
  IDataManagerSvc* m_histoDataMgrSvc = nullptr;
  /// Reference to the Histogram Persistency Service
  IConversionSvc* m_histoPersSvc = nullptr;
  /// Reference to the Whiteboard
  IHiveWhiteBoard* m_whiteboard = nullptr;
  /// Reference to the AlgExecStateSvc
  IAlgExecStateSvc* m_algExecStateSvc = nullptr;

  /// atomic count of the number of finished events
  mutable std::atomic<unsigned int> m_finishedEvt{0};
  /// condition variable to wake up main thread when we need to create a new event
  mutable std::condition_variable m_createEventCond;
  /// mutex assoiciated with m_createEventCond condition variable
  std::mutex m_createEventMutex;

  /// event selector context
  IEvtSelector::Context* m_evtSelContext{nullptr};

  /// Vector of algorithms to run for every event
  std::vector<Gaudi::Algorithm*> m_algos;
};
