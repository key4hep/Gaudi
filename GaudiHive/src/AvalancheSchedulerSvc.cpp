#include "AvalancheSchedulerSvc.h"
#include "AlgoExecutionTask.h"
#include "IOBoundAlgTask.h"

// Framework includes
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Algorithm.h" // will be IAlgorithm if context getter promoted to interface
#include "GaudiKernel/ConcurrencyFlags.h"
#include "GaudiKernel/DataHandleHolderVisitor.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/ThreadLocalContext.h"

// C++
#include <algorithm>
#include <map>
#include <queue>
#include <sstream>
#include <unordered_set>

// External libs
#include "boost/algorithm/string.hpp"
#include "boost/thread.hpp"
#include "boost/tokenizer.hpp"
// DP waiting for the TBB service
#include "tbb/task_scheduler_init.h"

std::mutex AvalancheSchedulerSvc::m_ssMut;
std::list<AvalancheSchedulerSvc::SchedulerState> AvalancheSchedulerSvc::m_sState;

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY( AvalancheSchedulerSvc )

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

//===========================================================================
// Infrastructure methods

/**
 * Here, among some "bureaucracy" operations, the scheduler is activated,
 * executing the activate() function in a new thread.
 * In addition the algorithms list is acquired from the algResourcePool.
**/
StatusCode AvalancheSchedulerSvc::initialize()
{

  // Initialise mother class (read properties, ...)
  StatusCode sc( Service::initialize() );
  if ( !sc.isSuccess() ) warning() << "Base class could not be initialized" << endmsg;

  // Get hold of the TBBSvc. This should initialize the thread pool
  m_threadPoolSvc = serviceLocator()->service( "ThreadPoolSvc" );
  if ( !m_threadPoolSvc.isValid() ) {
    fatal() << "Error retrieving ThreadPoolSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // Activate the scheduler in another thread.
  info() << "Activating scheduler in a separate thread" << endmsg;
  m_thread = std::thread( std::bind( &AvalancheSchedulerSvc::activate, this ) );

  while ( m_isActive != ACTIVE ) {
    if ( m_isActive == FAILURE ) {
      fatal() << "Terminating initialization" << endmsg;
      return StatusCode::FAILURE;
    } else {
      info() << "Waiting for AvalancheSchedulerSvc to activate" << endmsg;
      sleep( 1 );
    }
  }

  if ( m_enableCondSvc ) {
    // Get hold of the CondSvc
    m_condSvc = serviceLocator()->service( "CondSvc" );
    if ( !m_condSvc.isValid() ) {
      warning() << "No CondSvc found, or not enabled. "
                << "Will not manage CondAlgorithms" << endmsg;
      m_enableCondSvc = false;
    }
  }

  // Get the algo resource pool
  m_algResourcePool = serviceLocator()->service( "AlgResourcePool" );
  if ( !m_algResourcePool.isValid() ) {
    fatal() << "Error retrieving AlgoResourcePool" << endmsg;
    return StatusCode::FAILURE;
  }

  // Get the precedence service
  m_precSvc = serviceLocator()->service( "PrecedenceSvc" );
  if ( !m_precSvc.isValid() ) {
    fatal() << "Error retrieving PrecedenceSvc" << endmsg;
    return StatusCode::FAILURE;
  }
  const PrecedenceSvc* precSvc = dynamic_cast<const PrecedenceSvc*>( m_precSvc.get() );
  if ( !precSvc ) {
    fatal() << "Unable to dcast PrecedenceSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  m_algExecStateSvc = serviceLocator()->service( "AlgExecStateSvc" );
  if ( !m_algExecStateSvc.isValid() ) {
    fatal() << "Error retrieving AlgExecStateSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // Get Whiteboard
  m_whiteboard = serviceLocator()->service( m_whiteboardSvcName );
  if ( !m_whiteboard.isValid() ) {
    fatal() << "Error retrieving EventDataSvc interface IHiveWhiteBoard." << endmsg;
    return StatusCode::FAILURE;
  }

  // Get dedicated scheduler for I/O-bound algorithms
  if ( m_useIOBoundAlgScheduler ) {
    m_IOBoundAlgScheduler = serviceLocator()->service( m_IOBoundAlgSchedulerSvcName );
    if ( !m_IOBoundAlgScheduler.isValid() )
      fatal() << "Error retrieving IOBoundSchedulerAlgSvc interface IAccelerator." << endmsg;
  }

  // Set the MaxEventsInFlight parameters from the number of WB stores
  m_maxEventsInFlight = m_whiteboard->getNumberOfStores();

  // Set the number of free slots
  m_freeSlots = m_maxEventsInFlight;

  // set global concurrency flags
  Gaudi::Concurrency::ConcurrencyFlags::setNumConcEvents( m_maxEventsInFlight );

  // Get the list of algorithms
  const std::list<IAlgorithm*>& algos = m_algResourcePool->getFlatAlgList();
  const unsigned int algsNumber       = algos.size();
  info() << "Found " << algsNumber << " algorithms" << endmsg;

  /* Dependencies
   1) Look for handles in algo, if none
   2) Assume none are required
  */

  DataObjIDColl globalInp, globalOutp;

  // figure out all outputs
  for ( IAlgorithm* ialgoPtr : algos ) {
    Algorithm* algoPtr = dynamic_cast<Algorithm*>( ialgoPtr );
    if ( !algoPtr ) {
      fatal() << "Could not convert IAlgorithm into Algorithm: this will result in a crash." << endmsg;
    }
    for ( auto id : algoPtr->outputDataObjs() ) {
      auto r = globalOutp.insert( id );
      if ( !r.second ) {
        warning() << "multiple algorithms declare " << id << " as output! could be a single instance in multiple paths "
                                                             "though, or control flow may guarantee only one runs...!"
                  << endmsg;
      }
    }
  }

  std::ostringstream ostdd;
  ostdd << "Data Dependencies for Algorithms:";

  std::vector<DataObjIDColl> m_algosDependencies;
  for ( IAlgorithm* ialgoPtr : algos ) {
    Algorithm* algoPtr = dynamic_cast<Algorithm*>( ialgoPtr );
    if ( nullptr == algoPtr ) {
      fatal() << "Could not convert IAlgorithm into Algorithm for " << ialgoPtr->name()
              << ": this will result in a crash." << endmsg;
      return StatusCode::FAILURE;
    }

    ostdd << "\n  " << algoPtr->name();

    DataObjIDColl algoDependencies;
    if ( !algoPtr->inputDataObjs().empty() || !algoPtr->outputDataObjs().empty() ) {
      for ( const DataObjID* idp : sortedDataObjIDColl( algoPtr->inputDataObjs() ) ) {
        DataObjID id = *idp;
        ostdd << "\n    o INPUT  " << id;
        if ( id.key().find( ":" ) != std::string::npos ) {
          ostdd << " contains alternatives which require resolution...\n";
          auto tokens = boost::tokenizer<boost::char_separator<char>>{id.key(), boost::char_separator<char>{":"}};
          auto itok   = std::find_if( tokens.begin(), tokens.end(), [&]( const std::string& t ) {
            return globalOutp.find( DataObjID{t} ) != globalOutp.end();
          } );
          if ( itok != tokens.end() ) {
            ostdd << "found matching output for " << *itok << " -- updating scheduler info\n";
            id.updateKey( *itok );
          } else {
            error() << "failed to find alternate in global output list"
                    << " for id: " << id << " in Alg " << algoPtr->name() << endmsg;
            m_showDataDeps = true;
          }
        }
        algoDependencies.insert( id );
        globalInp.insert( id );
      }
      for ( const DataObjID* id : sortedDataObjIDColl( algoPtr->outputDataObjs() ) ) {
        ostdd << "\n    o OUTPUT " << *id;
        if ( id->key().find( ":" ) != std::string::npos ) {
          error() << " in Alg " << algoPtr->name() << " alternatives are NOT allowed for outputs! id: " << *id
                  << endmsg;
          m_showDataDeps = true;
        }
      }
    } else {
      ostdd << "\n      none";
    }
    m_algosDependencies.emplace_back( algoDependencies );
  }

  if ( m_showDataDeps ) {
    info() << ostdd.str() << endmsg;
  }

  // Fill the containers to convert algo names to index
  m_algname_vect.resize( algsNumber );
  IAlgorithm* dataLoaderAlg( nullptr );
  for ( IAlgorithm* algo : algos ) {
    const std::string& name    = algo->name();
    auto index                 = precSvc->getRules()->getAlgorithmNode( name )->getAlgoIndex();
    m_algname_index_map[name]  = index;
    m_algname_vect.at( index ) = name;
    if ( algo->name() == m_useDataLoader ) {
      dataLoaderAlg = algo;
    }
  }

  // Check if we have unmet global input dependencies
  if ( m_checkDeps ) {
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
        for ( size_t i = 0; i < m_algosDependencies.size(); ++i ) {
          if ( m_algosDependencies[i].find( *o ) != m_algosDependencies[i].end() ) {
            ost << "\n       * " << m_algname_vect[i];
          }
        }
      }

      if ( m_useDataLoader != "" ) {
        // Find the DataLoader Alg
        if ( dataLoaderAlg == nullptr ) {
          fatal() << "No DataLoader Algorithm \"" << m_useDataLoader.value()
                  << "\" found, and unmet INPUT dependencies "
                  << "detected:\n"
                  << ost.str() << endmsg;
          return StatusCode::FAILURE;
        }

        info() << "Will attribute the following unmet INPUT dependencies to \"" << dataLoaderAlg->type() << "/"
               << dataLoaderAlg->name() << "\" Algorithm" << ost.str() << endmsg;

        // Set the property Load of DataLoader Alg
        Algorithm* dataAlg = dynamic_cast<Algorithm*>( dataLoaderAlg );
        if ( !dataAlg ) {
          fatal() << "Unable to dcast DataLoader \"" << m_useDataLoader.value() << "\" IAlg to Algorithm" << endmsg;
          return StatusCode::FAILURE;
        }

        for ( auto& id : unmetDep ) {
          debug() << "adding OUTPUT dep \"" << id << "\" to " << dataLoaderAlg->type() << "/" << dataLoaderAlg->name()
                  << endmsg;
          dataAlg->addDependency( id, Gaudi::DataHandle::Writer );
        }

      } else {
        fatal() << "Auto DataLoading not requested, "
                << "and the following unmet INPUT dependencies were found:" << ost.str() << endmsg;
        return StatusCode::FAILURE;
      }

    } else {
      info() << "No unmet INPUT data dependencies were found" << endmsg;
    }
  }

  // Shortcut for the message service
  SmartIF<IMessageSvc> messageSvc( serviceLocator() );
  if ( !messageSvc.isValid() ) error() << "Error retrieving MessageSvc interface IMessageSvc." << endmsg;

  m_eventSlots.assign( m_maxEventsInFlight,
                       EventSlot( algsNumber, precSvc->getRules()->getControlFlowNodeCounter(), messageSvc ) );
  std::for_each( m_eventSlots.begin(), m_eventSlots.end(), []( EventSlot& slot ) { slot.complete = true; } );

  if ( m_threadPoolSize > 1 ) {
    m_maxAlgosInFlight = (size_t)m_threadPoolSize;
  }

  // Clearly inform about the level of concurrency
  info() << "Concurrency level information:" << endmsg;
  info() << " o Number of events in flight: " << m_maxEventsInFlight << endmsg;
  info() << " o TBB thread pool size: " << m_threadPoolSize << endmsg;

  if ( m_showControlFlow ) m_precSvc->dumpControlFlow();

  if ( m_showDataFlow ) m_precSvc->dumpDataFlow();

  // Simulate execution flow
  if ( m_simulateExecution ) m_precSvc->simulate( m_eventSlots[0] );

  return sc;
}
//---------------------------------------------------------------------------

/**
 * Here the scheduler is deactivated and the thread joined.
**/
StatusCode AvalancheSchedulerSvc::finalize()
{

  StatusCode sc( Service::finalize() );
  if ( !sc.isSuccess() ) warning() << "Base class could not be finalized" << endmsg;

  sc = deactivate();
  if ( !sc.isSuccess() ) warning() << "Scheduler could not be deactivated" << endmsg;

  info() << "Joining Scheduler thread" << endmsg;
  m_thread.join();

  // Final error check after thread pool termination
  if ( m_isActive == FAILURE ) {
    error() << "problems in scheduler thread" << endmsg;
    return StatusCode::FAILURE;
  }

  return sc;
}
//---------------------------------------------------------------------------
/**
 * Activate the scheduler. From this moment on the queue of actions is
 * checked. The checking will stop when the m_isActive flag is false and the
 * queue is not empty. This will guarantee that all actions are executed and
 * a stall is not created.
 * The TBB pool must be initialised in the thread from where the tasks are
 * launched (http://threadingbuildingblocks.org/docs/doxygen/a00342.html)
 * The scheduler is initialised here since this method runs in a separate
 * thread and spawns the tasks (through the execution of the lambdas)
 **/
void AvalancheSchedulerSvc::activate()
{

  if ( msgLevel( MSG::DEBUG ) ) debug() << "AvalancheSchedulerSvc::activate()" << endmsg;

  if ( m_threadPoolSvc->initPool( m_threadPoolSize ).isFailure() ) {
    error() << "problems initializing ThreadPoolSvc" << endmsg;
    m_isActive = FAILURE;
    return;
  }

  // Wait for actions pushed into the queue by finishing tasks.
  action thisAction;
  StatusCode sc( StatusCode::SUCCESS );

  m_isActive = ACTIVE;

  // Continue to wait if the scheduler is running or there is something to do
  info() << "Start checking the actionsQueue" << endmsg;
  while ( m_isActive == ACTIVE or m_actionsQueue.size() != 0 ) {
    m_actionsQueue.pop( thisAction );
    sc = thisAction();
    if ( sc != StatusCode::SUCCESS )
      verbose() << "Action did not succeed (which is not bad per se)." << endmsg;
    else
      verbose() << "Action succeeded." << endmsg;
  }

  info() << "Terminating thread-pool resources" << endmsg;
  if ( m_threadPoolSvc->terminatePool().isFailure() ) {
    error() << "Problems terminating thread pool" << endmsg;
    m_isActive = FAILURE;
  }
}

//---------------------------------------------------------------------------

/**
 * Deactivates the scheduler. Two actions are pushed into the queue:
 *  1) Drain the scheduler until all events are finished.
 *  2) Flip the status flag m_isActive to false
 * This second action is the last one to be executed by the scheduler.
 */
StatusCode AvalancheSchedulerSvc::deactivate()
{

  if ( m_isActive == ACTIVE ) {
    // Drain the scheduler
    m_actionsQueue.push( std::bind( &AvalancheSchedulerSvc::m_drain, this ) );
    // This would be the last action
    m_actionsQueue.push( [this]() -> StatusCode {
      m_isActive = INACTIVE;
      return StatusCode::SUCCESS;
    } );
  }

  return StatusCode::SUCCESS;
}

//===========================================================================

//===========================================================================
// Utils and shortcuts

inline const std::string& AvalancheSchedulerSvc::index2algname( unsigned int index ) { return m_algname_vect[index]; }

//---------------------------------------------------------------------------

inline unsigned int AvalancheSchedulerSvc::algname2index( const std::string& algoname )
{
  unsigned int index = m_algname_index_map[algoname];
  return index;
}

//===========================================================================
// EventSlot management
/**
 * Add event to the scheduler. There are two cases possible:
 *  1) No slot is free. A StatusCode::FAILURE is returned.
 *  2) At least one slot is free. An action which resets the slot and kicks
 * off its update is queued.
 */
StatusCode AvalancheSchedulerSvc::pushNewEvent( EventContext* eventContext )
{

  if ( m_first ) {
    m_first = false;
  }

  if ( !eventContext ) {
    fatal() << "Event context is nullptr" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( m_freeSlots.load() == 0 ) {
    if ( msgLevel( MSG::DEBUG ) ) debug() << "A free processing slot could not be found." << endmsg;
    return StatusCode::FAILURE;
  }

  // no problem as push new event is only called from one thread (event loop manager)
  m_freeSlots--;

  auto action = [this, eventContext]() -> StatusCode {
    // Event processing slot forced to be the same as the wb slot
    const unsigned int thisSlotNum = eventContext->slot();
    EventSlot& thisSlot            = m_eventSlots[thisSlotNum];
    if ( !thisSlot.complete ) {
      fatal() << "The slot " << thisSlotNum << " is supposed to be a finished event but it's not" << endmsg;
      return StatusCode::FAILURE;
    }

    debug() << "Executing event " << eventContext->evt() << " on slot " << thisSlotNum << endmsg;
    thisSlot.reset( eventContext );

    // Result status code:
    StatusCode result = StatusCode::SUCCESS;

    // promote to CR and DR the initial set of algorithms
    Cause cs = {Cause::source::Root, "RootDecisionHub"};
    if ( m_precSvc->iterate( thisSlot, cs ).isFailure() ) {
      error() << "Failed to call IPrecedenceSvc::iterate for slot " << thisSlotNum << endmsg;
      result = StatusCode::FAILURE;
    }

    if ( this->updateStates( thisSlotNum ).isFailure() ) {
      error() << "Failed to call AvalancheSchedulerSvc::updateStates for slot " << thisSlotNum << endmsg;
      result = StatusCode::FAILURE;
    }

    return result;
  }; // end of lambda

  // Kick off the scheduling!
  if ( msgLevel( MSG::VERBOSE ) ) {
    verbose() << "Pushing the action to update the scheduler for slot " << eventContext->slot() << endmsg;
    verbose() << "Free slots available " << m_freeSlots.load() << endmsg;
  }
  m_actionsQueue.push( action );

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
StatusCode AvalancheSchedulerSvc::pushNewEvents( std::vector<EventContext*>& eventContexts )
{
  StatusCode sc;
  for ( auto context : eventContexts ) {
    sc = pushNewEvent( context );
    if ( sc != StatusCode::SUCCESS ) return sc;
  }
  return sc;
}

//---------------------------------------------------------------------------
unsigned int AvalancheSchedulerSvc::freeSlots() { return std::max( m_freeSlots.load(), 0 ); }

//---------------------------------------------------------------------------
/**
 * Update the states for all slots until nothing is left to do.
*/
StatusCode AvalancheSchedulerSvc::m_drain()
{

  unsigned int slotNum = 0;
  for ( auto& thisSlot : m_eventSlots ) {
    if ( not thisSlot.algsStates.allAlgsExecuted() and not thisSlot.complete ) {
      updateStates( slotNum );
    }
    slotNum++;
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------
/**
* Get a finished event or block until one becomes available.
*/
StatusCode AvalancheSchedulerSvc::popFinishedEvent( EventContext*& eventContext )
{
  // debug() << "popFinishedEvent: queue size: " << m_finishedEvents.size() << endmsg;
  if ( m_freeSlots.load() == (int)m_maxEventsInFlight or m_isActive == INACTIVE ) {
    // debug() << "freeslots: " << m_freeSlots << "/" << m_maxEventsInFlight
    //      << " active: " << m_isActive << endmsg;
    return StatusCode::FAILURE;
  } else {
    // debug() << "freeslots: " << m_freeSlots << "/" << m_maxEventsInFlight
    //      << " active: " << m_isActive << endmsg;
    m_finishedEvents.pop( eventContext );
    m_freeSlots++;
    if ( msgLevel( MSG::DEBUG ) )
      debug() << "Popped slot " << eventContext->slot() << "(event " << eventContext->evt() << ")" << endmsg;
    return StatusCode::SUCCESS;
  }
}

//---------------------------------------------------------------------------
/**
* Try to get a finished event, if not available just return a failure
*/
StatusCode AvalancheSchedulerSvc::tryPopFinishedEvent( EventContext*& eventContext )
{
  if ( m_finishedEvents.try_pop( eventContext ) ) {
    if ( msgLevel( MSG::DEBUG ) )
      debug() << "Try Pop successful slot " << eventContext->slot() << "(event " << eventContext->evt() << ")"
              << endmsg;
    m_freeSlots++;
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

//---------------------------------------------------------------------------
/**
 * It can be possible that an event fails. In this case this method is called.
 * It dumps the state of the scheduler, drains the actions (without executing
 * them) and events in the queues and returns a failure.
*/
StatusCode AvalancheSchedulerSvc::eventFailed( EventContext* eventContext )
{

  // Set the number of slots available to an error code
  m_freeSlots.store( 0 );

  fatal() << "*** Event " << eventContext->evt() << " on slot " << eventContext->slot() << " failed! ***" << endmsg;

  std::ostringstream ost;
  m_algExecStateSvc->dump( ost, *eventContext );

  info() << "Dumping Alg Exec State for slot " << eventContext->slot() << ":\n" << ost.str() << endmsg;

  dumpSchedulerState( -1 );

  // Empty queue and deactivate the service
  action thisAction;
  while ( m_actionsQueue.try_pop( thisAction ) ) {
  };
  deactivate();

  // Push into the finished events queue the failed context
  EventContext* thisEvtContext;
  while ( m_finishedEvents.try_pop( thisEvtContext ) ) {
    m_finishedEvents.push( thisEvtContext );
  };
  m_finishedEvents.push( eventContext );

  return StatusCode::FAILURE;
}

//===========================================================================

//===========================================================================
// States Management

/**
 * Update the state of the algorithms.
 * The oldest events are checked before the newest, in order to reduce the
 * event backlog.
 * To check if the event is finished the algorithm checks if:
 * * No algorithms have been signed off by the control flow
 * * No algorithms have been signed off by the data flow
 * * No algorithms have been scheduled
*/
StatusCode AvalancheSchedulerSvc::updateStates( int si, const std::string& algo_name )
{

  StatusCode global_sc( StatusCode::SUCCESS );

  // Sort from the oldest to the newest event
  // Prepare a vector of pointers to the slots to avoid copies
  std::vector<EventSlot*> eventSlotsPtrs;

  // Consider all slots if si <0 or just one otherwise
  if ( si < 0 ) {
    const int eventsSlotsSize( m_eventSlots.size() );
    eventSlotsPtrs.reserve( eventsSlotsSize );
    for ( auto slotIt = m_eventSlots.begin(); slotIt != m_eventSlots.end(); slotIt++ ) {
      if ( !slotIt->complete ) eventSlotsPtrs.push_back( &( *slotIt ) );
    }
    std::sort( eventSlotsPtrs.begin(), eventSlotsPtrs.end(),
               []( EventSlot* a, EventSlot* b ) { return a->eventContext->evt() < b->eventContext->evt(); } );
  } else {
    eventSlotsPtrs.push_back( &m_eventSlots[si] );
  }

  for ( EventSlot* thisSlotPtr : eventSlotsPtrs ) {
    int iSlot = thisSlotPtr->eventContext->slot();

    // Cache the states of the algos to improve readability and performance
    auto& thisSlot                      = m_eventSlots[iSlot];
    AlgsExecutionStates& thisAlgsStates = thisSlot.algsStates;

    // Perform the I->CR->DR transitions
    if ( !algo_name.empty() ) {
      Cause cs = {Cause::source::Task, algo_name};
      if ( m_precSvc->iterate( thisSlot, cs ).isFailure() ) {
        error() << "Failed to call IPrecedenceSvc::iterate for slot " << iSlot << endmsg;
        global_sc = StatusCode::FAILURE;
      }
    }

    StatusCode partial_sc( StatusCode::FAILURE, true );

    // Perform DR->SCHEDULED
    if ( !m_optimizationMode.empty() ) {
      auto comp_nodes = [this]( const uint& i, const uint& j ) {
        return ( m_precSvc->getPriority( index2algname( i ) ) < m_precSvc->getPriority( index2algname( j ) ) );
      };
      std::priority_queue<uint, std::vector<uint>, std::function<bool( const uint&, const uint& )>> buffer(
          comp_nodes, std::vector<uint>() );
      for ( auto it = thisAlgsStates.begin( AlgsExecutionStates::State::DATAREADY );
            it != thisAlgsStates.end( AlgsExecutionStates::State::DATAREADY ); ++it )
        buffer.push( *it );
      /*std::stringstream s;
      auto buffer2 = buffer;
      while (!buffer2.empty()) {
        s << m_precSvc->getPriority(index2algname(buffer2.top())) << ", ";
        buffer2.pop();
      }
      info() << "DRBuffer is: [ " << s.str() << " ]  <--" << algo_name << " executed" << endmsg;*/

      /*while (!buffer.empty()) {
        partial_sc = promoteToScheduled(buffer.top(), iSlot);
        if (partial_sc.isFailure()) {
          if (msgLevel(MSG::VERBOSE))
            verbose() << "Could not apply transition from "
                      << AlgsExecutionStates::stateNames[AlgsExecutionStates::State::DATAREADY]
                      << " for algorithm " << index2algname(buffer.top()) << " on processing slot " << iSlot << endmsg;
          if (m_useIOBoundAlgScheduler) {
            partial_sc = promoteToAsyncScheduled(buffer.top(), iSlot);
            if (msgLevel(MSG::VERBOSE))
              if (partial_sc.isFailure())
                verbose() << "[Asynchronous] Could not apply transition from "
                          << AlgsExecutionStates::stateNames[AlgsExecutionStates::State::DATAREADY]
                          << " for algorithm " << index2algname(buffer.top()) << " on processing slot " << iSlot <<
      endmsg;
          }
        }
        buffer.pop();
      }*/
      while ( !buffer.empty() ) {
        bool IOBound                            = false;
        if ( m_useIOBoundAlgScheduler ) IOBound = m_precSvc->isBlocking( index2algname( buffer.top() ) );

        if ( !IOBound )
          partial_sc = promoteToScheduled( buffer.top(), iSlot );
        else
          partial_sc = promoteToAsyncScheduled( buffer.top(), iSlot );

        if ( msgLevel( MSG::VERBOSE ) )
          if ( partial_sc.isFailure() )
            verbose() << "Could not apply transition from "
                      << AlgsExecutionStates::stateNames[AlgsExecutionStates::State::DATAREADY] << " for algorithm "
                      << index2algname( buffer.top() ) << " on processing slot " << iSlot << endmsg;

        buffer.pop();
      }

    } else {
      for ( auto it = thisAlgsStates.begin( AlgsExecutionStates::State::DATAREADY );
            it != thisAlgsStates.end( AlgsExecutionStates::State::DATAREADY ); ++it ) {
        uint algIndex = *it;

        bool IOBound                            = false;
        if ( m_useIOBoundAlgScheduler ) IOBound = m_precSvc->isBlocking( index2algname( algIndex ) );

        if ( !IOBound )
          partial_sc = promoteToScheduled( algIndex, iSlot );
        else
          partial_sc = promoteToAsyncScheduled( algIndex, iSlot );

        if ( msgLevel( MSG::VERBOSE ) )
          if ( partial_sc.isFailure() )
            verbose() << "Could not apply transition from "
                      << AlgsExecutionStates::stateNames[AlgsExecutionStates::State::DATAREADY] << " for algorithm "
                      << index2algname( algIndex ) << " on processing slot " << iSlot << endmsg;
      }
    }

    if ( m_dumpIntraEventDynamics ) {
      std::stringstream s;
      s << algo_name << ", " << thisAlgsStates.sizeOfSubset( State::CONTROLREADY ) << ", "
        << thisAlgsStates.sizeOfSubset( State::DATAREADY ) << ", " << thisAlgsStates.sizeOfSubset( State::SCHEDULED )
        << ", " << std::chrono::high_resolution_clock::now().time_since_epoch().count() << "\n";
      auto threads = ( m_threadPoolSize != -1 ) ? std::to_string( m_threadPoolSize )
                                                : std::to_string( tbb::task_scheduler_init::default_num_threads() );
      std::ofstream myfile;
      myfile.open( "IntraEventConcurrencyDynamics_" + threads + "T.csv", std::ios::app );
      myfile << s.str();
      myfile.close();
    }

    // Not complete because this would mean that the slot is already free!
    if ( !thisSlot.complete && m_precSvc->CFRulesResolved( thisSlot ) &&
         !thisSlot.algsStates.algsPresent( AlgsExecutionStates::CONTROLREADY ) &&
         !thisSlot.algsStates.algsPresent( AlgsExecutionStates::DATAREADY ) &&
         !thisSlot.algsStates.algsPresent( AlgsExecutionStates::SCHEDULED ) ) {

      thisSlot.complete = true;
      // if the event did not fail, add it to the finished events
      // otherwise it is taken care of in the error handling already
      if ( m_algExecStateSvc->eventStatus( *thisSlot.eventContext ) == EventStatus::Success ) {
        m_finishedEvents.push( thisSlot.eventContext );
        if ( msgLevel( MSG::DEBUG ) )
          debug() << "Event " << thisSlot.eventContext->evt() << " finished (slot " << thisSlot.eventContext->slot()
                  << ")." << endmsg;
      }

      // now let's return the fully evaluated result of the control flow
      if ( msgLevel( MSG::DEBUG ) ) debug() << m_precSvc->printState( thisSlot ) << endmsg;

      thisSlot.eventContext = nullptr;
    } else {
      StatusCode eventStalledSC = isStalled( iSlot );
      if ( !eventStalledSC.isSuccess() ) {
        m_algExecStateSvc->setEventStatus( EventStatus::AlgStall, *thisSlot.eventContext );
        eventFailed( thisSlot.eventContext ).ignore();
      }
    }
  } // end loop on slots

  verbose() << "States Updated." << endmsg;

  return global_sc;
}

//---------------------------------------------------------------------------

/**
 * Check if we are in present of a stall condition for a particular slot.
 * This is the case when no actions are present in the actionsQueue,
 * no algorithm is in flight and no algorithm has all of its dependencies
 * satisfied.
*/
StatusCode AvalancheSchedulerSvc::isStalled( int iSlot )
{
  // Get the slot
  EventSlot& thisSlot = m_eventSlots[iSlot];

  if ( m_actionsQueue.empty() && m_algosInFlight == 0 && m_IOBoundAlgosInFlight == 0 &&
       ( !thisSlot.algsStates.algsPresent( AlgsExecutionStates::DATAREADY ) ) ) {

    info() << "About to declare a stall" << endmsg;
    fatal() << "*** Stall detected! ***\n" << endmsg;
    dumpSchedulerState( iSlot );
    // throw GaudiException ("Stall detected",name(),StatusCode::FAILURE);

    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

/**
 * Used for debugging purposes, the state of the scheduler is dumped on screen
 * in order to be inspected.
 * The dependencies of each algo are printed and the missing ones specified.
**/
void AvalancheSchedulerSvc::dumpSchedulerState( int iSlot )
{

  // To have just one big message
  std::ostringstream outputMessageStream;

  outputMessageStream << "============================== Execution Task State ============================="
                      << std::endl;
  dumpState( outputMessageStream );

  outputMessageStream << std::endl
                      << "============================== Scheduler State  ================================="
                      << std::endl;

  int slotCount = -1;
  for ( auto& thisSlot : m_eventSlots ) {
    slotCount++;
    if ( thisSlot.complete ) continue;

    // dump temporal and topological precedence analysis (if enabled in the PrecedenceSvc)
    if ( msgLevel( MSG::DEBUG ) ) m_precSvc->dumpPrecedenceRules( thisSlot );

    outputMessageStream << "-----------  slot: " << thisSlot.eventContext->slot()
                        << "  event: " << thisSlot.eventContext->evt() << " -----------" << std::endl;

    if ( 0 > iSlot or iSlot == slotCount ) {

      // Snapshot of the Control Flow and FSM states
      outputMessageStream << "\nControl Flow and FSM states:" << std::endl;
      outputMessageStream << m_precSvc->printState( thisSlot ) << std::endl;
    }
  }

  outputMessageStream << "=================================== END ======================================" << std::endl;

  info() << "Dumping Scheduler State " << std::endl << outputMessageStream.str() << endmsg;
}

//---------------------------------------------------------------------------

StatusCode AvalancheSchedulerSvc::promoteToScheduled( unsigned int iAlgo, int si )
{

  if ( m_algosInFlight == m_maxAlgosInFlight ) return StatusCode::FAILURE;

  const std::string& algName( index2algname( iAlgo ) );
  IAlgorithm* ialgoPtr = nullptr;
  StatusCode sc( m_algResourcePool->acquireAlgorithm( algName, ialgoPtr ) );

  if ( sc.isSuccess() ) { // if we managed to get an algorithm instance try to schedule it
    EventContext* eventContext( m_eventSlots[si].eventContext );
    if ( !eventContext ) {
      fatal() << "Event context for algorithm " << algName << " is a nullptr (slot " << si << ")" << endmsg;
      return StatusCode::FAILURE;
    }

    ++m_algosInFlight;
    auto promote2ExecutedClosure = [this, iAlgo, ialgoPtr, eventContext]() {
      this->m_actionsQueue.push( [this, iAlgo, ialgoPtr, eventContext]() {
        return this->AvalancheSchedulerSvc::promoteToExecuted( iAlgo, eventContext->slot(), ialgoPtr, eventContext );
      } );
      return StatusCode::SUCCESS;
    };

    // Avoid to use tbb if the pool size is 1 and run in this thread
    if ( -100 != m_threadPoolSize ) {
      // the child task that executes an Algorithm
      tbb::task* algoTask = new ( tbb::task::allocate_root() )
          AlgoExecutionTask( ialgoPtr, eventContext, serviceLocator(), m_algExecStateSvc, promote2ExecutedClosure );
      // schedule the algoTask
      tbb::task::enqueue( *algoTask );

    } else {
      AlgoExecutionTask theTask( ialgoPtr, eventContext, serviceLocator(), m_algExecStateSvc, promote2ExecutedClosure );
      theTask.execute();
    }

    if ( msgLevel( MSG::DEBUG ) )
      debug() << "Algorithm " << algName << " was submitted on event " << eventContext->evt() << " in slot " << si
              << ". Algorithms scheduled are " << m_algosInFlight << endmsg;

    StatusCode updateSc( m_eventSlots[si].algsStates.updateState( iAlgo, AlgsExecutionStates::SCHEDULED ) );

    if ( msgLevel( MSG::VERBOSE ) ) dumpSchedulerState( -1 );

    if ( updateSc.isSuccess() )
      if ( msgLevel( MSG::VERBOSE ) )
        verbose() << "Promoting " << index2algname( iAlgo ) << " to SCHEDULED on slot " << si << endmsg;
    return updateSc;
  } else {
    if ( msgLevel( MSG::DEBUG ) )
      debug() << "Could not acquire instance for algorithm " << index2algname( iAlgo ) << " on slot " << si << endmsg;
    return sc;
  }
}

//---------------------------------------------------------------------------

StatusCode AvalancheSchedulerSvc::promoteToAsyncScheduled( unsigned int iAlgo, int si )
{

  if ( m_IOBoundAlgosInFlight == m_maxIOBoundAlgosInFlight ) return StatusCode::FAILURE;

  // bool IOBound = m_precSvc->isBlocking(algName);

  const std::string& algName( index2algname( iAlgo ) );
  IAlgorithm* ialgoPtr = nullptr;
  StatusCode sc( m_algResourcePool->acquireAlgorithm( algName, ialgoPtr ) );

  if ( sc.isSuccess() ) { // if we managed to get an algorithm instance try to schedule it
    EventContext* eventContext( m_eventSlots[si].eventContext );
    if ( !eventContext ) {
      fatal() << "[Asynchronous] Event context for algorithm " << algName << " is a nullptr (slot " << si << ")"
              << endmsg;
      return StatusCode::FAILURE;
    }

    ++m_IOBoundAlgosInFlight;
    // Can we use tbb-based overloaded new-operator for a "custom" task (an algorithm wrapper, not derived from
    // tbb::task)? it seems it works..
    IOBoundAlgTask* theTask = new ( tbb::task::allocate_root() )
        IOBoundAlgTask( ialgoPtr, eventContext, serviceLocator(), m_algExecStateSvc );
    m_IOBoundAlgScheduler->push( *theTask );

    if ( msgLevel( MSG::DEBUG ) )
      debug() << "[Asynchronous] Algorithm " << algName << " was submitted on event " << eventContext->evt()
              << " in slot " << si << ". algorithms scheduled are " << m_IOBoundAlgosInFlight << endmsg;

    StatusCode updateSc( m_eventSlots[si].algsStates.updateState( iAlgo, AlgsExecutionStates::SCHEDULED ) );

    if ( updateSc.isSuccess() )
      if ( msgLevel( MSG::VERBOSE ) )
        verbose() << "[Asynchronous] Promoting " << index2algname( iAlgo ) << " to SCHEDULED on slot " << si << endmsg;
    return updateSc;
  } else {
    if ( msgLevel( MSG::DEBUG ) )
      debug() << "[Asynchronous] Could not acquire instance for algorithm " << index2algname( iAlgo ) << " on slot "
              << si << endmsg;
    return sc;
  }
}

//---------------------------------------------------------------------------
/**
 * The call to this method is triggered only from within the AlgoExecutionTask.
*/
StatusCode AvalancheSchedulerSvc::promoteToExecuted( unsigned int iAlgo, int si, IAlgorithm* algo,
                                                     EventContext* eventContext )
{
  // Put back the instance
  Algorithm* castedAlgo = dynamic_cast<Algorithm*>( algo ); // DP: expose context getter in IAlgo?
  if ( !castedAlgo ) fatal() << "The casting did not succeed!" << endmsg;
  //  EventContext* eventContext = castedAlgo->getContext();

  // Check if the execution failed
  if ( m_algExecStateSvc->eventStatus( *eventContext ) != EventStatus::Success ) eventFailed( eventContext ).ignore();

  Gaudi::Hive::setCurrentContext( eventContext );
  StatusCode sc = m_algResourcePool->releaseAlgorithm( algo->name(), algo );

  if ( !sc.isSuccess() ) {
    error() << "[Event " << eventContext->evt() << ", Slot " << eventContext->slot() << "] "
            << "Instance of algorithm " << algo->name() << " could not be properly put back." << endmsg;
    return StatusCode::FAILURE;
  }

  m_algosInFlight--;

  EventSlot& thisSlot = m_eventSlots[si];

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "Algorithm " << algo->name() << " executed in slot " << si << ". Algorithms scheduled are "
            << m_algosInFlight << endmsg;

  // Schedule an update of the status of the algorithms
  auto updateAction = std::bind( &AvalancheSchedulerSvc::updateStates, this, -1, algo->name() );
  m_actionsQueue.push( updateAction );

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "Trying to handle execution result of " << index2algname( iAlgo ) << " on slot " << si << endmsg;
  State state;
  if ( algo->filterPassed() ) {
    state = State::EVTACCEPTED;
  } else {
    state = State::EVTREJECTED;
  }

  sc = thisSlot.algsStates.updateState( iAlgo, state );

  if ( sc.isSuccess() )
    if ( msgLevel( MSG::VERBOSE ) )
      verbose() << "Promoting " << index2algname( iAlgo ) << " on slot " << si << " to "
                << AlgsExecutionStates::stateNames[state] << endmsg;

  return sc;
}

//---------------------------------------------------------------------------
/**
 * The call to this method is triggered only from within the IOBoundAlgTask.
*/
StatusCode AvalancheSchedulerSvc::promoteToAsyncExecuted( unsigned int iAlgo, int si, IAlgorithm* algo,
                                                          EventContext* eventContext )
{
  // Put back the instance
  Algorithm* castedAlgo = dynamic_cast<Algorithm*>( algo ); // DP: expose context getter in IAlgo?
  if ( !castedAlgo ) fatal() << "[Asynchronous] The casting did not succeed!" << endmsg;
  // EventContext* eventContext = castedAlgo->getContext();

  // Check if the execution failed
  if ( m_algExecStateSvc->eventStatus( *eventContext ) != EventStatus::Success ) eventFailed( eventContext ).ignore();

  StatusCode sc = m_algResourcePool->releaseAlgorithm( algo->name(), algo );

  if ( !sc.isSuccess() ) {
    error() << "[Asynchronous]  [Event " << eventContext->evt() << ", Slot " << eventContext->slot() << "] "
            << "Instance of algorithm " << algo->name() << " could not be properly put back." << endmsg;
    return StatusCode::FAILURE;
  }

  m_IOBoundAlgosInFlight--;

  EventSlot& thisSlot = m_eventSlots[si];

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "[Asynchronous] Algorithm " << algo->name() << " executed in slot " << si
            << ". Algorithms scheduled are " << m_IOBoundAlgosInFlight << endmsg;

  // Schedule an update of the status of the algorithms
  auto updateAction = std::bind( &AvalancheSchedulerSvc::updateStates, this, -1, algo->name() );
  m_actionsQueue.push( updateAction );

  if ( msgLevel( MSG::DEBUG ) )
    debug() << "[Asynchronous] Trying to handle execution result of " << index2algname( iAlgo ) << " on slot " << si
            << endmsg;
  State state;
  if ( algo->filterPassed() ) {
    state = State::EVTACCEPTED;
  } else {
    state = State::EVTREJECTED;
  }

  sc = thisSlot.algsStates.updateState( iAlgo, state );

  if ( sc.isSuccess() )
    if ( msgLevel( MSG::VERBOSE ) )
      verbose() << "[Asynchronous] Promoting " << index2algname( iAlgo ) << " on slot " << si << " to "
                << AlgsExecutionStates::stateNames[state] << endmsg;

  return sc;
}

//===========================================================================
void AvalancheSchedulerSvc::addAlg( Algorithm* a, EventContext* e, pthread_t t )
{

  std::lock_guard<std::mutex> lock( m_ssMut );
  m_sState.push_back( SchedulerState( a, e, t ) );
}

//===========================================================================
bool AvalancheSchedulerSvc::delAlg( Algorithm* a )
{

  std::lock_guard<std::mutex> lock( m_ssMut );

  for ( std::list<SchedulerState>::iterator itr = m_sState.begin(); itr != m_sState.end(); ++itr ) {
    if ( *itr == a ) {
      m_sState.erase( itr );
      return true;
    }
  }

  error() << "could not find Alg " << a->name() << " in Scheduler!" << endmsg;
  return false;
}

//===========================================================================
void AvalancheSchedulerSvc::dumpState( std::ostringstream& ost )
{

  std::lock_guard<std::mutex> lock( m_ssMut );

  for ( auto it : m_sState ) {
    ost << "  " << it << std::endl;
  }
}

//===========================================================================
void AvalancheSchedulerSvc::dumpState()
{

  std::lock_guard<std::mutex> lock( m_ssMut );

  std::ostringstream ost;
  ost << "dumping Executing Threads: [" << m_sState.size() << "]" << std::endl;
  dumpState( ost );

  info() << ost.str() << endmsg;
}
