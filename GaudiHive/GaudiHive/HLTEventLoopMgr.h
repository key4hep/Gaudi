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
class IDataManagerSvc;
class IDataProviderSvc;
class IPrecedenceSvc;

/**@class RecoSchedulerSvc RecoSchedulerSvc.h
 *
 *  Basic event loop and scheduler for fast HLT reconstruction.
 *  It uses one fifo queue per tbb thread and schedules full events on a
 *  given thread by putting all algos in the dedicated queue from the beginning
 *
 */
class HLTEventLoopMgr : public extends<Service, IEventProcessor>
{

  struct HLTExecutionTask : public tbb::task {

    HLTExecutionTask( std::vector<IAlgorithm*>& algorithms, std::unique_ptr<EventContext> ctx, ISvcLocator* svcLocator,
                      IAlgExecStateSvc* aem,
                      std::function<void( std::unique_ptr<EventContext> )> promote2ExecutedClosure )
        : m_algorithms( algorithms )
        , m_evtCtx( std::move( ctx ) )
        , m_aess( aem )
        , m_serviceLocator( svcLocator )
        , m_promote2ExecutedClosure( std::move( promote2ExecutedClosure ) ){};
    tbb::task* execute() override;

    MsgStream log()
    {
      SmartIF<IMessageSvc> messageSvc( m_serviceLocator );
      return MsgStream( messageSvc, "HLTExecutionTask" );
    }

    std::vector<IAlgorithm*>& m_algorithms;
    std::unique_ptr<EventContext> m_evtCtx;
    IAlgExecStateSvc* m_aess;
    SmartIF<ISvcLocator> m_serviceLocator;
    std::function<void( std::unique_ptr<EventContext> )> m_promote2ExecutedClosure;
  };

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
  StatusCode eventFailed( EventContext* eventContext );
  /// Algorithm promotion
  void promoteToExecuted( std::unique_ptr<EventContext> eventContext );

  /// Convert a name to an integer
  inline unsigned int algname2index( const std::string& algoname ) { return m_algname_index_map[algoname]; }
  /// Convert an integer to a name
  inline const std::string& index2algname( unsigned int index ) { return m_algname_vect[index]; }

private:
  Gaudi::Property<std::string> m_histPersName{this, "HistogramPersistency", "", ""};
  Gaudi::Property<std::string> m_evtsel{this, "EvtSel", "", ""};
  Gaudi::Property<int> m_threadPoolSize{this, "ThreadPoolSize", -1, "Size of the threadpool initialised by TBB"};
  Gaudi::Property<std::string> m_whiteboardSvcName{this, "WhiteboardSvc", "EventDataSvc", "The whiteboard name"};

  /// Reference to the Event Data Service's IDataManagerSvc interface
  IDataManagerSvc* m_evtDataMgrSvc;
  /// Reference to the Event Selector
  IEvtSelector* m_evtSelector;
  /// Reference to the Histogram Data Service
  IDataManagerSvc* m_histoDataMgrSvc;
  /// Reference to the Histogram Persistency Service
  IConversionSvc* m_histoPersSvc;
  /// Reference to the Whiteboard
  IHiveWhiteBoard* m_whiteboard;
  /// Reference to the AlgExecStateSvc
  IAlgExecStateSvc* m_algExecStateSvc;
  /// A shortcut to the Precedence Service
  IPrecedenceSvc* m_precSvc;

  /// atomic count of the number of finished events
  std::atomic<unsigned int> m_finishedEvt{0};
  /// condition variable to wake up main thread when we need to create a new event
  std::condition_variable m_createEventCond;
  /// mutex assoiciated with m_createEventCond condition variable
  std::mutex m_createEventMutex;

  /// event selector context
  IEvtSelector::Context* m_evtSelContext{nullptr};

  /// Vector to bookkeep the information necessary to the index2name conversion
  std::vector<std::string> m_algname_vect;
  /// Map to bookkeep the information necessary to the name2index conversion
  std::unordered_map<std::string, unsigned int> m_algname_index_map;
  /// Vector of events slots
  std::vector<EventSlot> m_eventSlots;
  /// Vector of algorithms to run for every event
  std::vector<IAlgorithm*> m_algos;
};
