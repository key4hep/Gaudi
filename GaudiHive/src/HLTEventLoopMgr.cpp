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
#include "RetCodeGuard.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <map>
#include <sstream>

#include "boost/algorithm/string.hpp"
#include "boost/optional.hpp"
#include "boost/thread.hpp"
#include "boost/tokenizer.hpp"
#include "tbb/task_scheduler_init.h"

#include "HLTEventLoopMgr.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_COMPONENT( HLTEventLoopMgr )

namespace
{
  struct DataObjIDDotRepr {
    const DataObjID&     parent;
    friend std::ostream& operator<<( std::ostream& os, const DataObjIDDotRepr& repr )
    {
      return os << '\"' << repr.parent.fullKey() << '\"';
    }
  };

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

struct HLTEventLoopMgr::HLTExecutionTask final : tbb::task {

  std::vector<Algorithm*>&      m_algorithms;
  std::unique_ptr<EventContext> m_evtCtx;
  IAlgExecStateSvc*             m_aess;
  SmartIF<ISvcLocator>          m_serviceLocator;
  const HLTEventLoopMgr*        m_parent;

  HLTExecutionTask( std::vector<Algorithm*>& algorithms, std::unique_ptr<EventContext> ctx, ISvcLocator* svcLocator,
                    IAlgExecStateSvc* aem, const HLTEventLoopMgr* parent )
      : m_algorithms( algorithms )
      , m_evtCtx( std::move( ctx ) )
      , m_aess( aem )
      , m_serviceLocator( svcLocator )
      , m_parent( parent )
  {
  }

  MsgStream log()
  {
    SmartIF<IMessageSvc> messageSvc( m_serviceLocator );
    return MsgStream( messageSvc, "HLTExecutionTask" );
  }

  tbb::task* execute() override
  {
    bool eventfailed = false;
    Gaudi::Hive::setCurrentContext( m_evtCtx.get() );

    const SmartIF<IProperty> appmgr( m_serviceLocator );

    for ( Algorithm* ialg : m_algorithms ) {

      // select the appropriate store
      ialg->whiteboard()->selectStore( m_evtCtx->valid() ? m_evtCtx->slot() : 0 ).ignore();

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
        log() << MSG::FATAL << ".executeEvent(): Exception with tag=" << Exception.tag() << " thrown by "
              << ialg->name() << endmsg << MSG::ERROR << Exception << endmsg;
        eventfailed = true;
      } catch ( const std::exception& Exception ) {
        log() << MSG::FATAL << ".executeEvent(): Standard std::exception thrown by " << ialg->name() << endmsg
              << MSG::ERROR << Exception.what() << endmsg;
        eventfailed = true;
      } catch ( ... ) {
        log() << MSG::FATAL << ".executeEvent(): UNKNOWN Exception thrown by " << ialg->name() << endmsg;
        eventfailed = true;
      }

      // DP it is important to propagate the failure of an event.
      // We need to stop execution when this happens so that execute run can
      // then receive the FAILURE
      m_aess->algExecState( ialg, *m_evtCtx ).setState( AlgExecState::State::Done, sc );
      m_aess->updateEventStatus( eventfailed, *m_evtCtx );

      // in case the algorithm was a filter and the filter did not pass, stop here
      if ( !ialg->filterPassed() ) break;
    }
    // update scheduler state
    m_parent->promoteToExecuted( std::move( m_evtCtx ) );
    Gaudi::Hive::setCurrentContextEvt( -1 );

    return nullptr;
  }
};

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
    m_evtSelector = nullptr;
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
    Algorithm* algoPtr = dynamic_cast<Algorithm*>( alg );
    if ( !algoPtr ) {
      fatal() << "Could not convert IAlgorithm into Algorithm: this will result in a crash." << endmsg;
    }
    m_algos.push_back( algoPtr );
  }
  /* Dependencies
   1) Look for handles in algo, if none
   2) Assume none are required
  */
  DataObjIDColl globalInp, globalOutp;

  boost::optional<std::ofstream> dot{boost::in_place_init_if, !m_dotfile.empty(), m_dotfile.value()};

  if ( dot ) {
    *dot << "digraph G {\n";
    for ( auto* a : m_algos ) {
      bool is_consumer = a->outputDataObjs().empty();
      *dot << '\"' << a->name() << "\" [shape=box" << ( is_consumer ? ",style=filled" : "" ) << "];\n";
    }
  }

  // figure out all outputs
  std::map<DataObjID, Algorithm*> producers;
  for ( Algorithm* algoPtr : m_algos ) {
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
  algosDependencies.reserve( m_algos.size() ); // reserve so that pointers into the vector do not get invalidated...
  std::map<const Algorithm*, DataObjIDColl*> algo2Deps;
  info() << "Data Dependencies for Algorithms:";

  for ( Algorithm* algoPtr : m_algos ) {
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
        if ( dot ) *dot << DataObjIDDotRepr{id} << " -> \"" << algoPtr->name() << "\";\n";
        globalInp.insert( id );
      }
      for ( const DataObjID* id : sortedDataObjIDColl( algoPtr->outputDataObjs() ) ) {
        info() << "\n    o OUTPUT " << *id;
        if ( id->key().find( ":" ) != std::string::npos ) {
          error() << " in Alg " << algoPtr->name() << " alternatives are NOT allowed for outputs! id: " << *id
                  << endmsg;
        }
        if ( dot ) *dot << '\"' << algoPtr->name() << "\" -> " << DataObjIDDotRepr{*id} << ";\n";
      }
    } else {
      info() << "\n      none";
    }
    algosDependencies.emplace_back( algoDependencies );
    algo2Deps[algoPtr] = &algosDependencies.back();
  }
  if ( dot ) {
    for ( const auto& t : globalOutp ) {
      if ( globalInp.find( t ) == globalInp.end() ) *dot << DataObjIDDotRepr{t} << " [style=filled];\n";
    }
    *dot << "}\n";
  }
  info() << endmsg;

  // Check if we have unmet global input dependencies
  DataObjIDColl unmetDep;
  for ( auto o : globalInp ) {
    if ( globalOutp.find( o ) == globalOutp.end() ) unmetDep.insert( o );
  }
  if ( unmetDep.size() > 0 ) {
    std::ostringstream ost;
    for ( const DataObjID* o : sortedDataObjIDColl( unmetDep ) ) {
      ost << "\n   o " << *o << "    required by Algorithm: ";
      for ( const auto& i : algo2Deps ) {
        if ( i.second->find( *o ) != i.second->end() ) {
          ost << "\n       * " << i.first->name();
        }
      }
    }
    fatal() << "The following unmet INPUT dependencies were found:" << ost.str() << endmsg;
    return StatusCode::FAILURE;
  } else {
    info() << "No unmet INPUT data dependencies were found" << endmsg;
  }

  // Clearly inform about the level of concurrency
  info() << "Concurrency level information:" << endmsg;
  info() << " o Number of events slots: " << m_whiteboard->getNumberOfStores() << endmsg;
  info() << " o TBB thread pool size: " << m_threadPoolSize << endmsg;

  // rework the flat algo list to respect data dependencies
  auto start = m_algos.begin();
  auto end   = m_algos.end();
  auto current =
      std::partition( start, end, [&algo2Deps]( const Algorithm* algo ) { return algo2Deps[algo]->empty(); } );

  // Repeatedly put in front algos for which input are already fullfilled
  while ( current != end ) {
    current = std::partition( current, end, [start, current, &producers, &algo2Deps]( const Algorithm* algo ) {
      return std::none_of( algo2Deps[algo]->begin(), algo2Deps[algo]->end(),
                           [start, current, &producers]( const DataObjID& id ) {
                             return std::find( start, current, producers[id] ) == current;
                           } );
    } );
  }

  // Fill the containers to convert algo names to index
  debug() << "Order of algo execution :" << endmsg;
  for ( const Algorithm* algo : m_algos ) debug() << "  . " << algo->name() << endmsg;

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
        StatusCode      iret  = m_histoPersSvc->createRep( i, pAddr );
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
  int& createdEvts = *reinterpret_cast<int*>( createdEvts_IntPtr );

  auto evtContext = std::make_unique<EventContext>();
  evtContext->set( createdEvts, m_whiteboard->allocateStore( createdEvts ) );
  m_algExecStateSvc->reset( *evtContext );

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
  if ( msgLevel( MSG::DEBUG ) )
    debug() << "Event " << evtContext->evt() << " submitting in slot " << evtContext->slot() << endmsg;

  // Event processing slot forced to be the same as the wb slot
  tbb::task* task = new ( tbb::task::allocate_root() )
      HLTExecutionTask( m_algos, std::move( evtContext ), serviceLocator(), m_algExecStateSvc, this );
  tbb::task::enqueue( *task );

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
  // Run the first event before spilling more than one
  bool newEvtAllowed = false;

  info() << "Starting loop on events" << endmsg;
  auto start_time = Clock::now();
  while ( maxevt < 0 || m_finishedEvt < (unsigned int)maxevt ) {
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
    if ( !sc.isSuccess() ) return StatusCode::FAILURE; // else we have an success --> exit loop
    newEvtAllowed = true;
  } // end main loop on finished events
  auto end_time = Clock::now();

  delete m_evtSelContext;
  m_evtSelContext = nullptr;

  constexpr double oneOver1024 = 1. / 1024.;
  info() << "---> Loop Finished (skipping 1st evt) - "
         << " WSS " << System::mappedMemory( System::MemoryUnit::kByte ) * oneOver1024 << " total time "
         << std::chrono::duration_cast<std::chrono::milliseconds>( end_time - start_time ).count() << " ms" << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode HLTEventLoopMgr::declareEventRootAddress()
{
  IOpaqueAddress* pAddr = nullptr;
  StatusCode      sc    = m_evtSelector->next( *m_evtSelContext );
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
StatusCode HLTEventLoopMgr::eventFailed( EventContext* eventContext ) const
{
  fatal() << "*** Event " << eventContext->evt() << " on slot " << eventContext->slot() << " failed! ***" << endmsg;
  std::ostringstream ost;
  m_algExecStateSvc->dump( ost, *eventContext );
  info() << "Dumping Alg Exec State for slot " << eventContext->slot() << ":\n" << ost.str() << endmsg;
  return StatusCode::FAILURE;
}

void HLTEventLoopMgr::promoteToExecuted( std::unique_ptr<EventContext> eventContext ) const
{
  // Check if the execution failed
  if ( m_algExecStateSvc->eventStatus( *eventContext ) != EventStatus::Success )
    eventFailed( eventContext.get() ).ignore();
  int si = eventContext->slot();

  // if ( msgLevel( MSG::DEBUG ) )
  //  debug() << "Event " << eventContext->evt() << " executed in slot " << si << "." << endmsg;

  // Schedule the cleanup of the event
  if ( m_algExecStateSvc->eventStatus( *eventContext ) == EventStatus::Success ) {
    if ( msgLevel( MSG::DEBUG ) )
      debug() << "Event " << eventContext->evt() << " finished (slot " << si << ")." << endmsg;
  } else {
    fatal() << "Failed event detected on " << *eventContext << endmsg;
  }

  debug() << "Clearing slot " << si << " (event " << eventContext->evt() << ") of the whiteboard" << endmsg;

  StatusCode sc = m_whiteboard->clearStore( si );
  if ( !sc.isSuccess() ) warning() << "Clear of Event data store failed" << endmsg;
  sc = m_whiteboard->freeStore( si );
  if ( !sc.isSuccess() ) error() << "Whiteboard slot " << eventContext->slot() << " could not be properly cleared";
  ++m_finishedEvt;
  m_createEventCond.notify_all();
}
