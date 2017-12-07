// FW includes
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/AppReturnCode.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IAlgResourcePool.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/IScheduler.h"
#include "GaudiKernel/Memory.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include "EventSlot.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "HistogramAgent.h"
#include "PrecedenceSvc.h"
#include "RetCodeGuard.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <map>
#include <sstream>

#include "boost/algorithm/string.hpp"
#include "boost/thread.hpp"
#include "boost/tokenizer.hpp"
#include "tbb/task_scheduler_init.h"

#include "GaudiHive/HLTEventLoopMgr.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY( HLTEventLoopMgr )

namespace
{
  struct DataObjIDSorter {
    bool operator()( const DataObjID* a, const DataObjID* b ) { return a->fullKey() < b->fullKey(); }
  };

  // Sort a DataObjIDColl in a well-defined, reproducible manner.
  // Used for making debugging dumps.
  std::vector<const DataObjID*> sortedDataObjIDColl( const DataObjIDColl& coll )
  {
    std::vector<const DataObjID*> v;
    v.reserve( coll.size() );
    for ( const DataObjID& id : coll ) v.push_back( &id );
    std::sort( v.begin(), v.end(), DataObjIDSorter() );
    return v;
  }
}

StatusCode HLTEventLoopMgr::initialize()
{
  StatusCode sc = Service::initialize();
  if ( !sc.isSuccess() ) {
    error() << "Failed to initialize Service Base class." << endmsg;
    return StatusCode::FAILURE;
  }
  // Setup access to event data services
  m_evtDataMgrSvc = serviceLocator()->service<IDataManagerSvc>( "EventDataSvc" );
  if ( !m_evtDataMgrSvc ) {
    fatal() << "Error retrieving EventDataSvc interface IDataManagerSvc." << endmsg;
    return StatusCode::FAILURE;
  }
  m_whiteboard = serviceLocator()->service<IHiveWhiteBoard>( "EventDataSvc" );
  if ( !m_whiteboard ) {
    fatal() << "Error retrieving EventDataSvc interface IHiveWhiteBoard." << endmsg;
    return StatusCode::FAILURE;
  }
  // Obtain the IProperty of the ApplicationMgr
  IProperty* appMgrProperty = serviceLocator()->service<IProperty>( "ApplicationMgr" );
  if ( !appMgrProperty ) {
    fatal() << "IProperty interface not found in ApplicationMgr." << endmsg;
    return StatusCode::FAILURE;
  }
  // We do not expect a Event Selector necessarily being declared
  setProperty( appMgrProperty->getProperty( "EvtSel" ) ).ignore();

  if ( m_evtsel != "NONE" || m_evtsel.length() == 0 ) {
    m_evtSelector = serviceLocator()->service<IEvtSelector>( "EventSelector" );
  } else {
    m_evtSelector = 0;
    warning() << "Unable to locate service \"EventSelector\" " << endmsg;
    warning() << "No events will be processed from external input." << endmsg;
  }

  // Setup access to histogramming services
  m_histoDataMgrSvc = serviceLocator()->service<IDataManagerSvc>( "HistogramDataSvc" );
  if ( !m_histoDataMgrSvc ) {
    fatal() << "Error retrieving HistogramDataSvc." << endmsg;
    return sc;
  }
  // Setup histogram persistency
  m_histoPersSvc = serviceLocator()->service<IConversionSvc>( "HistogramPersistencySvc" );
  if ( !m_histoPersSvc ) {
    warning() << "Histograms cannot not be saved - though required." << endmsg;
    return sc;
  }

  m_algExecStateSvc = serviceLocator()->service<IAlgExecStateSvc>( "AlgExecStateSvc" );
  if ( !m_algExecStateSvc ) {
    fatal() << "Error retrieving AlgExecStateSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // Get the precedence service
  m_precSvc = serviceLocator()->service<IPrecedenceSvc>( "PrecedenceSvc" );
  if ( !m_precSvc ) {
    fatal() << "Error retrieving PrecedenceSvc" << endmsg;
    return StatusCode::FAILURE;
  }
  const PrecedenceSvc* precSvc = dynamic_cast<const PrecedenceSvc*>( m_precSvc );
  if ( !precSvc ) {
    fatal() << "Unable to dcast PrecedenceSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  m_algExecStateSvc = serviceLocator()->service<IAlgExecStateSvc>( "AlgExecStateSvc" );
  if ( !m_algExecStateSvc ) {
    fatal() << "Error retrieving AlgExecStateSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // Get the list of algorithms
  IAlgResourcePool* algResourcePool = serviceLocator()->service<IAlgResourcePool>( "AlgResourcePool" );
  if ( !algResourcePool ) {
    fatal() << "Error retrieving AlgoResourcePool" << endmsg;
    return StatusCode::FAILURE;
  }
  for ( auto alg : algResourcePool->getFlatAlgList() ) {
    m_algos.push_back( alg );
  }
  const unsigned int algsNumber = m_algos.size();

  /* Dependencies
   1) Look for handles in algo, if none
   2) Assume none are required
  */
  DataObjIDColl globalInp, globalOutp;

  // figure out all outputs
  std::map<DataObjID, IAlgorithm*> producers;
  for ( IAlgorithm* ialgoPtr : m_algos ) {
    Algorithm* algoPtr = dynamic_cast<Algorithm*>( ialgoPtr );
    if ( !algoPtr ) {
      fatal() << "Could not convert IAlgorithm into Algorithm: this will result in a crash." << endmsg;
    }
    for ( auto id : algoPtr->outputDataObjs() ) {
      auto r        = globalOutp.insert( id );
      producers[id] = algoPtr;
      if ( !r.second ) {
        warning() << "multiple algorithms declare " << id << " as output! could be a single instance in multiple paths "
                                                             "though, or control flow may guarantee only one runs...!"
                  << endmsg;
      }
    }
  }

  // Building and printing Data Dependencies
  std::vector<DataObjIDColl> algosDependencies;
  std::map<const IAlgorithm*, unsigned int> algo2Index;
  info() << "Data Dependencies for Algorithms:";
  int n = 0;
  for ( IAlgorithm* ialgoPtr : m_algos ) {
    Algorithm* algoPtr = dynamic_cast<Algorithm*>( ialgoPtr );
    info() << "\n  " << algoPtr->name();

    DataObjIDColl algoDependencies;
    if ( !algoPtr->inputDataObjs().empty() || !algoPtr->outputDataObjs().empty() ) {
      for ( const DataObjID* idp : sortedDataObjIDColl( algoPtr->inputDataObjs() ) ) {
        DataObjID id = *idp;
        info() << "\n    o INPUT  " << id;
        if ( id.key().find( ":" ) != std::string::npos ) {
          info() << " contains alternatives which require resolution...\n";
          auto tokens = boost::tokenizer<boost::char_separator<char>>{id.key(), boost::char_separator<char>{":"}};
          auto itok   = std::find_if( tokens.begin(), tokens.end(), [&]( const std::string& t ) {
            return globalOutp.find( DataObjID{t} ) != globalOutp.end();
          } );
          if ( itok != tokens.end() ) {
            info() << "found matching output for " << *itok << " -- updating scheduler info\n";
            id.updateKey( *itok );
          } else {
            error() << "failed to find alternate in global output list"
                    << " for id: " << id << " in Alg " << algoPtr->name() << endmsg;
          }
        }
        algoDependencies.insert( id );
        globalInp.insert( id );
      }
      for ( const DataObjID* id : sortedDataObjIDColl( algoPtr->outputDataObjs() ) ) {
        info() << "\n    o OUTPUT " << *id;
        if ( id->key().find( ":" ) != std::string::npos ) {
          error() << " in Alg " << algoPtr->name() << " alternatives are NOT allowed for outputs! id: " << *id
                  << endmsg;
        }
      }
    } else {
      info() << "\n      none";
    }
    algosDependencies.emplace_back( algoDependencies );
    algo2Index[ialgoPtr] = n;
    n++;
  }
  info() << endmsg;

  // Check if we have unmet global input dependencies
  DataObjIDColl unmetDep;
  for ( auto o : globalInp ) {
    if ( globalOutp.find( o ) == globalOutp.end() ) {
      unmetDep.insert( o );
    }
  }
  if ( unmetDep.size() > 0 ) {
    std::ostringstream ost;
    for ( const DataObjID* o : sortedDataObjIDColl( unmetDep ) ) {
      ost << "\n   o " << *o << "    required by Algorithm: ";
      for ( size_t i = 0; i < algosDependencies.size(); ++i ) {
        if ( algosDependencies[i].find( *o ) != algosDependencies[i].end() ) {
          ost << "\n       * " << m_algname_vect[i];
        }
      }
    }
    fatal() << "The following unmet INPUT dependencies were found:" << ost.str() << endmsg;
    return StatusCode::FAILURE;
  } else {
    info() << "No unmet INPUT data dependencies were found" << endmsg;
  }

  // Deal with Event Slots
  auto messageSvc = serviceLocator()->service<IMessageSvc>( "MessageSvc" );
  m_eventSlots.assign( m_whiteboard->getNumberOfStores(),
                       EventSlot( algsNumber, precSvc->getRules()->getControlFlowNodeCounter(), messageSvc ) );

  // Clearly inform about the level of concurrency
  info() << "Concurrency level information:" << endmsg;
  info() << " o Number of events slots: " << m_whiteboard->getNumberOfStores() << endmsg;
  info() << " o TBB thread pool size: " << m_threadPoolSize << endmsg;

  // rework the flat algo list to respect data dependencies
  auto start   = m_algos.begin();
  auto end     = m_algos.end();
  auto current = std::partition( start, end, [&algosDependencies, &algo2Index]( const IAlgorithm* algo ) {
    return algosDependencies[algo2Index[algo]].empty();
  } );

  // Repeatedly put in front algos for which input are already fullfilled
  while ( current != end ) {
    current = std::partition(
        current, end, [start, current, &producers, &algosDependencies, &algo2Index]( const IAlgorithm* algo ) {
          return std::none_of( algosDependencies[algo2Index[algo]].begin(), algosDependencies[algo2Index[algo]].end(),
                               [start, current, &producers]( const DataObjID& id ) {
                                 return std::find( start, current, producers[id] ) == current;
                               } );
        } );
  }

  // Fill the containers to convert algo names to index
  debug() << "Order of algo execution :" << endmsg;
  m_algname_vect.resize( algsNumber );
  for ( IAlgorithm* algo : m_algos ) {
    const std::string& name    = algo->name();
    auto index                 = precSvc->getRules()->getAlgorithmNode( name )->getAlgoIndex();
    m_algname_index_map[name]  = index;
    m_algname_vect.at( index ) = name;
    debug() << "  . " << algo->name() << endmsg;
  }

  return sc;
}

StatusCode HLTEventLoopMgr::finalize()
{
  StatusCode sc;
  // Save Histograms Now
  if ( m_histoPersSvc ) {
    HistogramAgent agent;
    sc = m_histoDataMgrSvc->traverseTree( &agent );
    if ( sc.isSuccess() ) {
      const IDataSelector& objects = agent.selectedObjects();
      // skip /stat entry!
      sc = std::accumulate( begin( objects ), end( objects ), sc, [&]( StatusCode s, const auto& i ) {
        IOpaqueAddress* pAddr = nullptr;
        StatusCode iret       = m_histoPersSvc->createRep( i, pAddr );
        if ( iret.isSuccess() ) i->registry()->setAddress( pAddr );
        return s.isFailure() ? s : iret;
      } );
      sc = std::accumulate( begin( objects ), end( objects ), sc, [&]( StatusCode s, const auto& i ) {
        IRegistry* reg  = i->registry();
        StatusCode iret = m_histoPersSvc->fillRepRefs( reg->address(), i );
        return s.isFailure() ? s : iret;
      } );
      if ( sc.isSuccess() ) {
        info() << "Histograms converted successfully according to request." << endmsg;
      } else {
        error() << "Error while saving Histograms." << endmsg;
      }
    } else {
      error() << "Error while traversing Histogram data store" << endmsg;
    }
  }

  StatusCode sc2 = Service::finalize();
  return sc.isFailure() ? sc2.ignore(), sc : sc2;
}

StatusCode HLTEventLoopMgr::executeEvent( void* createdEvts_IntPtr )
{
  // Leave the interface intact and swallow this C trick.
  int& createdEvts = *( (int*)createdEvts_IntPtr );

  std::unique_ptr<EventContext> evtContext = std::make_unique<EventContext>();
  evtContext->set( createdEvts, m_whiteboard->allocateStore( createdEvts ) );
  m_algExecStateSvc->reset( *evtContext.get() );

  StatusCode sc = m_whiteboard->selectStore( evtContext->slot() );
  if ( sc.isFailure() ) {
    fatal() << "Slot " << evtContext->slot() << " could not be selected for the WhiteBoard\n"
            << "Impossible to create event context" << endmsg;
    return StatusCode::FAILURE;
  }

  StatusCode declEvtRootSc = declareEventRootAddress();
  if ( declEvtRootSc.isFailure() ) { // We ran out of events!
    createdEvts = -1;                // Set created event to a negative value: we finished!
    return StatusCode::SUCCESS;
  }

  // Now add event to the scheduler
  verbose() << "Adding event " << evtContext->evt() << ", slot " << evtContext->slot() << " to the scheduler" << endmsg;

  // Event processing slot forced to be the same as the wb slot
  const unsigned int thisSlotNum = evtContext->slot();
  m_eventSlots[thisSlotNum].reset( evtContext.get() );
  // closure to be executed at the end of event
  auto promote2ExecutedClosure = [this]( std::unique_ptr<EventContext> evtContext ) {
    this->promoteToExecuted( std::move( evtContext ) );
  };
  // tbb task
  tbb::task* task = new ( tbb::task::allocate_root() ) HLTExecutionTask(
      m_algos, std::move( evtContext ), serviceLocator(), m_algExecStateSvc, promote2ExecutedClosure );
  tbb::task::enqueue( *task );

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "All algorithms were submitted on event " << evtContext->evt() << " in slot " << thisSlotNum << endmsg;

  createdEvts++;
  return StatusCode::SUCCESS;
}

StatusCode HLTEventLoopMgr::stopRun()
{
  // Set the application return code
  auto appmgr = serviceLocator()->as<IProperty>();
  if ( Gaudi::setAppReturnCode( appmgr, Gaudi::ReturnCode::ScheduledStop ).isFailure() ) {
    error() << "Could not set return code of the application (" << Gaudi::ReturnCode::ScheduledStop << ")" << endmsg;
  }
  return StatusCode::SUCCESS;
}

StatusCode HLTEventLoopMgr::nextEvent( int maxevt )
{
  // Calculate runtime
  using Clock = std::chrono::high_resolution_clock;

  // Reset the application return code.
  auto appmgr = serviceLocator()->as<IProperty>();
  Gaudi::setAppReturnCode( appmgr, Gaudi::ReturnCode::Success, true ).ignore();

  // create m_evtSelContext used internally in executeEvent and more
  // precisely in declareEventRootAddress. Cannot be passed through the interface
  // without breaking other schedulers
  StatusCode sc = m_evtSelector->createContext( m_evtSelContext );
  if ( !sc.isSuccess() ) {
    fatal() << "Can not create the event selector Context." << endmsg;
    return sc;
  }

  // create th tbb thread pool
  tbb::task_scheduler_init tbbSchedInit( m_threadPoolSize.value() + 1 );

  int createdEvts = 0;
  // Loop until the finished events did not reach the maxevt number
  bool loop_ended = false;
  // Run the first event before spilling more than one
  bool newEvtAllowed = false;

  info() << "Starting loop on events" << endmsg;
  auto start_time = Clock::now();
  while ( !loop_ended && ( maxevt < 0 || m_finishedEvt < (unsigned int)maxevt ) ) {
    // if the created events did not reach maxevt, create an event
    if ( !( ( newEvtAllowed || createdEvts == 0 ) && // Launch the first event alone
            // The events are not finished with an unlimited number of events
            createdEvts >= 0 &&
            // The events are not finished with a limited number of events
            ( createdEvts < maxevt || maxevt < 0 ) &&
            // There are still free slots in the whiteboard
            m_whiteboard->freeSlots() > 0 ) ) {

      std::unique_lock<std::mutex> lock{m_createEventMutex};
      using namespace std::chrono_literals;
      m_createEventCond.wait_for( lock, 1ms, [this, newEvtAllowed, createdEvts, maxevt] {
        return ( newEvtAllowed || createdEvts == 0 ) && // Launch the first event alone
               // The events are not finished with an unlimited number of events
               createdEvts >= 0 &&
               // The events are not finished with a limited number of events
               ( createdEvts < maxevt || maxevt < 0 ) &&
               // There are still free slots in the whiteboard
               this->m_whiteboard->freeSlots() > 0;
      } );
      continue;
    }
    if ( 1 == createdEvts ) // reset counter to count from event 1
      start_time = Clock::now();

    // TODO can we adapt the interface of executeEvent for a nicer solution?
    StatusCode sc = StatusCode::RECOVERABLE;
    sc            = executeEvent( &createdEvts );
    if ( !sc.isSuccess() ) {
      return StatusCode::FAILURE;
    } // else we have an success --> exit loop
    newEvtAllowed = true;
  } // end main loop on finished events
  auto end_time = Clock::now();

  delete m_evtSelContext;
  m_evtSelContext = nullptr;

  constexpr double oneOver1024 = 1. / 1024.;
  info() << "---> Loop Finished (skipping 1st evt) - "
         << " WSS " << System::mappedMemory( System::MemoryUnit::kByte ) * oneOver1024 << " total time "
         << std::chrono::duration_cast<std::chrono::nanoseconds>( end_time - start_time ).count() << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode HLTEventLoopMgr::declareEventRootAddress()
{
  IOpaqueAddress* pAddr = nullptr;
  StatusCode sc         = m_evtSelector->next( *m_evtSelContext );
  if ( sc.isSuccess() ) {
    // Create root address and assign address to data service
    sc = m_evtSelector->createAddress( *m_evtSelContext, pAddr );
    if ( !sc.isSuccess() ) {
      sc = m_evtSelector->next( *m_evtSelContext );
      if ( sc.isSuccess() ) {
        sc = m_evtSelector->createAddress( *m_evtSelContext, pAddr );
        if ( !sc.isSuccess() ) warning() << "Error creating IOpaqueAddress." << endmsg;
      }
    }
  }
  if ( !sc.isSuccess() ) {
    info() << "No more events in event selection " << endmsg;
    return StatusCode::FAILURE;
  }
  sc = m_evtDataMgrSvc->setRoot( "/Event", pAddr );
  if ( !sc.isSuccess() ) {
    warning() << "Error declaring event root address." << endmsg;
  }
  return StatusCode::SUCCESS;
}

/**
 * It can be possible that an event fails. In this case this method is called.
 * It dumps the state of the scheduler, drains the actions (without executing
 * them) and events in the queues and returns a failure.
*/
StatusCode HLTEventLoopMgr::eventFailed( EventContext* eventContext )
{
  fatal() << "*** Event " << eventContext->evt() << " on slot " << eventContext->slot() << " failed! ***" << endmsg;
  std::ostringstream ost;
  m_algExecStateSvc->dump( ost, *eventContext );
  info() << "Dumping Alg Exec State for slot " << eventContext->slot() << ":\n" << ost.str() << endmsg;
  return StatusCode::FAILURE;
}

void HLTEventLoopMgr::promoteToExecuted( std::unique_ptr<EventContext> eventContext )
{
  // Check if the execution failed
  if ( m_algExecStateSvc->eventStatus( *eventContext ) != EventStatus::Success )
    eventFailed( eventContext.get() ).ignore();
  int si = eventContext->slot();

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "Event " << eventContext->evt() << " executed in slot " << si << "." << endmsg;

  // Schedule the cleanup of the event
  if ( m_algExecStateSvc->eventStatus( *eventContext ) == EventStatus::Success ) {
    if ( msgLevel( MSG::DEBUG ) )
      debug() << "Event " << eventContext->evt() << " finished (slot " << si << ")." << endmsg;
  } else {
    fatal() << "Failed event detected on " << *eventContext << endmsg;
  }

  debug() << "Clearing slot " << si << " (event " << eventContext->evt() << ") of the whiteboard" << endmsg;

  StatusCode sc = m_whiteboard->clearStore( si );
  if ( !sc.isSuccess() ) {
    warning() << "Clear of Event data store failed" << endmsg;
  }
  m_eventSlots[si].eventContext = nullptr;
  sc                            = m_whiteboard->freeStore( si );
  if ( !sc.isSuccess() ) {
    error() << "Whiteboard slot " << eventContext->slot() << " could not be properly cleared";
  }
  m_finishedEvt++;
  m_createEventCond.notify_all();
}

tbb::task* HLTEventLoopMgr::HLTExecutionTask::execute()
{
  bool eventfailed = false;
  Gaudi::Hive::setCurrentContext( m_evtCtx.get() );

  const SmartIF<IProperty> appmgr( m_serviceLocator );

  for ( IAlgorithm* ialg : m_algorithms ) {
    Algorithm* this_algo = dynamic_cast<Algorithm*>( ialg );
    if ( !this_algo ) {
      throw GaudiException( "Cast to Algorithm failed!", "HLTExecutionTask", StatusCode::FAILURE );
    }

    // select the appropriate store
    this_algo->whiteboard()->selectStore( m_evtCtx->valid() ? m_evtCtx->slot() : 0 ).ignore();

    StatusCode sc( StatusCode::FAILURE );
    try {
      RetCodeGuard rcg( appmgr, Gaudi::ReturnCode::UnhandledException );
      m_aess->algExecState( ialg, *m_evtCtx ).setState( AlgExecState::State::Executing );
      sc = ialg->sysExecute( *m_evtCtx );
      if ( UNLIKELY( !sc.isSuccess() ) ) {
        log() << MSG::WARNING << "Execution of algorithm " << ialg->name() << " failed" << endmsg;
        eventfailed = true;
      }
      rcg.ignore(); // disarm the guard
    } catch ( const GaudiException& Exception ) {
      log() << MSG::FATAL << ".executeEvent(): Exception with tag=" << Exception.tag() << " thrown by " << ialg->name()
            << endmsg << MSG::ERROR << Exception << endmsg;
      eventfailed = true;
    } catch ( const std::exception& Exception ) {
      log() << MSG::FATAL << ".executeEvent(): Standard std::exception thrown by " << ialg->name() << endmsg
            << MSG::ERROR << Exception.what() << endmsg;
      eventfailed = true;
    } catch ( ... ) {
      log() << MSG::FATAL << ".executeEvent(): UNKNOWN Exception thrown by " << ialg->name() << endmsg;
      eventfailed = true;
    }

    // Commit all DataHandles
    this_algo->commitHandles();

    // DP it is important to propagate the failure of an event.
    // We need to stop execution when this happens so that execute run can
    // then receive the FAILURE
    m_aess->algExecState( ialg, *m_evtCtx ).setState( AlgExecState::State::Done, sc );
    m_aess->updateEventStatus( eventfailed, *m_evtCtx );

    // in case the algorithm was a filter and the filter did not pass, stop here
    if ( !this_algo->filterPassed() ) {
      break;
    }
  }
  // update scheduler state
  m_promote2ExecutedClosure( std::move( m_evtCtx ) );
  Gaudi::Hive::setCurrentContextEvt( -1 );

  return nullptr;
}
