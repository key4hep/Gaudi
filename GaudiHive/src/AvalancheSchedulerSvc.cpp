/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "AvalancheSchedulerSvc.h"
#include "AlgTask.h"
#include "ThreadPoolSvc.h"

// Framework includes
#include "GaudiKernel/ConcurrencyFlags.h"
#include "GaudiKernel/DataHandleHolderVisitor.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include <Gaudi/Algorithm.h> // can be removed ASA dynamic casts to Algorithm are removed

// C++
#include <algorithm>
#include <map>
#include <queue>
#include <sstream>
#include <string_view>
#include <thread>
#include <unordered_set>

// External libs
#include "boost/algorithm/string.hpp"
#include "boost/thread.hpp"
#include "boost/tokenizer.hpp"
// DP waiting for the TBB service
#include "tbb/tbb_stddef.h"

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_COMPONENT( AvalancheSchedulerSvc )

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )

namespace {
  struct DataObjIDSorter {
    bool operator()( const DataObjID* a, const DataObjID* b ) { return a->fullKey() < b->fullKey(); }
  };

  // Sort a DataObjIDColl in a well-defined, reproducible manner.
  // Used for making debugging dumps.
  std::vector<const DataObjID*> sortedDataObjIDColl( const DataObjIDColl& coll ) {
    std::vector<const DataObjID*> v;
    v.reserve( coll.size() );
    for ( const DataObjID& id : coll ) v.push_back( &id );
    std::sort( v.begin(), v.end(), DataObjIDSorter() );
    return v;
  }

  bool subSlotAlgsInStates( const EventSlot& slot, std::initializer_list<AlgsExecutionStates::State> testStates ) {
    return std::any_of( slot.allSubSlots.begin(), slot.allSubSlots.end(),
                        [testStates]( const EventSlot& ss ) { return ss.algsStates.containsAny( testStates ); } );
  }
} // namespace

//---------------------------------------------------------------------------

/**
 * Here, among some "bureaucracy" operations, the scheduler is activated,
 * executing the activate() function in a new thread.
 * In addition the algorithms list is acquired from the algResourcePool.
 **/

StatusCode AvalancheSchedulerSvc::initialize() {

  // Initialise mother class (read properties, ...)
  StatusCode sc( Service::initialize() );
  if ( sc.isFailure() ) warning() << "Base class could not be initialized" << endmsg;

  // Get hold of the TBBSvc. This should initialize the thread pool
  m_threadPoolSvc = serviceLocator()->service( "ThreadPoolSvc" );
  if ( !m_threadPoolSvc.isValid() ) {
    fatal() << "Error retrieving ThreadPoolSvc" << endmsg;
    return StatusCode::FAILURE;
  }
  auto castTPS = dynamic_cast<ThreadPoolSvc*>( m_threadPoolSvc.get() );
  if ( !castTPS ) {
    fatal() << "Cannot cast ThreadPoolSvc" << endmsg;
    return StatusCode::FAILURE;
  }
  m_arena = castTPS->getArena();
  if ( !m_arena ) {
    fatal() << "Cannot find valid TBB task_arena" << endmsg;
    return StatusCode::FAILURE;
  }

  // Activate the scheduler in another thread.
  info() << "Activating scheduler in a separate thread" << endmsg;
  m_thread = std::thread( [this]() { this->activate(); } );

  while ( m_isActive != ACTIVE ) {
    if ( m_isActive == FAILURE ) {
      fatal() << "Terminating initialization" << endmsg;
      return StatusCode::FAILURE;
    } else {
      ON_DEBUG debug() << "Waiting for AvalancheSchedulerSvc to activate" << endmsg;
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

  // Set the MaxEventsInFlight parameters from the number of WB stores
  m_maxEventsInFlight = m_whiteboard->getNumberOfStores();

  // Set the number of free slots
  m_freeSlots = m_maxEventsInFlight;

  // Get the list of algorithms
  const std::list<IAlgorithm*>& algos      = m_algResourcePool->getFlatAlgList();
  const unsigned int            algsNumber = algos.size();
  if ( algsNumber != 0 ) {
    info() << "Found " << algsNumber << " algorithms" << endmsg;
  } else {
    error() << "No algorithms found" << endmsg;
    return StatusCode::FAILURE;
  }

  /* Dependencies
   1) Look for handles in algo, if none
   2) Assume none are required
  */

  DataObjIDColl globalInp, globalOutp;

  // figure out all outputs
  for ( IAlgorithm* ialgoPtr : algos ) {
    Gaudi::Algorithm* algoPtr = dynamic_cast<Gaudi::Algorithm*>( ialgoPtr );
    if ( !algoPtr ) {
      fatal() << "Could not convert IAlgorithm into Gaudi::Algorithm: this will result in a crash." << endmsg;
      return StatusCode::FAILURE;
    }
    for ( auto id : algoPtr->outputDataObjs() ) globalOutp.insert( id );
  }

  std::ostringstream ostdd;
  ostdd << "Data Dependencies for Algorithms:";

  std::map<std::string, DataObjIDColl> algosDependenciesMap;
  for ( IAlgorithm* ialgoPtr : algos ) {
    Gaudi::Algorithm* algoPtr = dynamic_cast<Gaudi::Algorithm*>( ialgoPtr );
    if ( nullptr == algoPtr ) {
      fatal() << "Could not convert IAlgorithm into Gaudi::Algorithm for " << ialgoPtr->name()
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
    algosDependenciesMap[algoPtr->name()] = algoDependencies;
  }

  if ( m_showDataDeps ) { info() << ostdd.str() << endmsg; }

  // Check if we have unmet global input dependencies, and, optionally, heal them
  // WARNING: this step must be done BEFORE the Precedence Service is initialized
  if ( m_checkDeps ) {
    DataObjIDColl unmetDep;
    for ( auto o : globalInp )
      if ( globalOutp.find( o ) == globalOutp.end() ) unmetDep.insert( o );

    if ( unmetDep.size() > 0 ) {

      auto printUnmet = [&]( auto msg ) {
        for ( const DataObjID* o : sortedDataObjIDColl( unmetDep ) ) {
          msg << "   o " << *o << "    required by Algorithm: " << endmsg;

          for ( const auto& p : algosDependenciesMap )
            if ( p.second.find( *o ) != p.second.end() ) msg << "       * " << p.first << endmsg;
        }
      };

      if ( !m_useDataLoader.empty() ) {

        // Find the DataLoader Alg
        IAlgorithm* dataLoaderAlg( nullptr );
        for ( IAlgorithm* algo : algos )
          if ( algo->name() == m_useDataLoader ) {
            dataLoaderAlg = algo;
            break;
          }

        if ( dataLoaderAlg == nullptr ) {
          fatal() << "No DataLoader Algorithm \"" << m_useDataLoader.value()
                  << "\" found, and unmet INPUT dependencies "
                  << "detected:" << endmsg;
          printUnmet( fatal() );
          return StatusCode::FAILURE;
        }

        info() << "Will attribute the following unmet INPUT dependencies to \"" << dataLoaderAlg->type() << "/"
               << dataLoaderAlg->name() << "\" Algorithm" << endmsg;
        printUnmet( info() );

        // Set the property Load of DataLoader Alg
        Gaudi::Algorithm* dataAlg = dynamic_cast<Gaudi::Algorithm*>( dataLoaderAlg );
        if ( !dataAlg ) {
          fatal() << "Unable to dcast DataLoader \"" << m_useDataLoader.value() << "\" IAlg to Gaudi::Algorithm"
                  << endmsg;
          return StatusCode::FAILURE;
        }

        for ( auto& id : unmetDep ) {
          ON_DEBUG debug() << "adding OUTPUT dep \"" << id << "\" to " << dataLoaderAlg->type() << "/"
                           << dataLoaderAlg->name() << endmsg;
          dataAlg->addDependency( id, Gaudi::DataHandle::Writer );
        }

      } else {
        fatal() << "Auto DataLoading not requested, "
                << "and the following unmet INPUT dependencies were found:" << endmsg;
        printUnmet( fatal() );
        return StatusCode::FAILURE;
      }

    } else {
      info() << "No unmet INPUT data dependencies were found" << endmsg;
    }
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

  // Fill the containers to convert algo names to index
  m_algname_vect.resize( algsNumber );
  for ( IAlgorithm* algo : algos ) {
    const std::string& name    = algo->name();
    auto               index   = precSvc->getRules()->getAlgorithmNode( name )->getAlgoIndex();
    m_algname_index_map[name]  = index;
    m_algname_vect.at( index ) = name;
  }

  // Shortcut for the message service
  SmartIF<IMessageSvc> messageSvc( serviceLocator() );
  if ( !messageSvc.isValid() ) error() << "Error retrieving MessageSvc interface IMessageSvc." << endmsg;

  m_eventSlots.reserve( m_maxEventsInFlight );
  for ( size_t i = 0; i < m_maxEventsInFlight; ++i ) {
    m_eventSlots.emplace_back( algsNumber, precSvc->getRules()->getControlFlowNodeCounter(), messageSvc );
    m_eventSlots.back().complete = true;
  }

  if ( m_threadPoolSize > 1 ) { m_maxAlgosInFlight = (size_t)m_threadPoolSize; }

  // Clearly inform about the level of concurrency
  info() << "Concurrency level information:" << endmsg;
  info() << " o Number of events in flight: " << m_maxEventsInFlight << endmsg;
  info() << " o TBB thread pool size: " << m_threadPoolSize << endmsg;

  // Inform about task scheduling prescriptions
  info() << "Task scheduling settings:" << endmsg;
  info() << " o Avalanche generation mode: "
         << ( m_optimizationMode.empty() ? "disabled" : m_optimizationMode.toString() ) << endmsg;
  info() << " o Preemptive scheduling of CPU-blocking tasks: "
         << ( m_enablePreemptiveBlockingTasks
                  ? ( "enabled (max. " + std::to_string( m_maxBlockingAlgosInFlight ) + " concurrent tasks)" )
                  : "disabled" )
         << endmsg;
  info() << " o Scheduling of condition tasks: " << ( m_enableCondSvc ? "enabled" : "disabled" ) << endmsg;

  if ( m_showControlFlow ) m_precSvc->dumpControlFlow();

  if ( m_showDataFlow ) m_precSvc->dumpDataFlow();

  // Simulate execution flow
  if ( m_simulateExecution ) sc = m_precSvc->simulate( m_eventSlots[0] );

  return sc;
}
//---------------------------------------------------------------------------

/**
 * Here the scheduler is deactivated and the thread joined.
 **/
StatusCode AvalancheSchedulerSvc::finalize() {

  StatusCode sc( Service::finalize() );
  if ( sc.isFailure() ) warning() << "Base class could not be finalized" << endmsg;

  sc = deactivate();
  if ( sc.isFailure() ) warning() << "Scheduler could not be deactivated" << endmsg;

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
void AvalancheSchedulerSvc::activate() {

  ON_DEBUG debug() << "AvalancheSchedulerSvc::activate()" << endmsg;

  if ( m_threadPoolSvc->initPool( m_threadPoolSize ).isFailure() ) {
    error() << "problems initializing ThreadPoolSvc" << endmsg;
    m_isActive = FAILURE;
    return;
  }

  // Wait for actions pushed into the queue by finishing tasks.
  action     thisAction;
  StatusCode sc( StatusCode::SUCCESS );

  m_isActive = ACTIVE;

  // Continue to wait if the scheduler is running or there is something to do
  ON_DEBUG debug() << "Start checking the actionsQueue" << endmsg;
  while ( m_isActive == ACTIVE || m_actionsQueue.size() != 0 ) {
    m_actionsQueue.pop( thisAction );
    sc = thisAction();
    ON_VERBOSE {
      if ( sc.isFailure() )
        verbose() << "Action did not succeed (which is not bad per se)." << endmsg;
      else
        verbose() << "Action succeeded." << endmsg;
    }
    else sc.ignore();

    // If all queued actions have been processed, update the slot states
    if ( m_needsUpdate.load() && m_actionsQueue.empty() ) {
      sc = iterate();
      ON_VERBOSE {
        if ( sc.isFailure() )
          verbose() << "Iteration did not succeed (which is not bad per se)." << endmsg;
        else
          verbose() << "Iteration succeeded." << endmsg;
      }
      else sc.ignore();
    }
  }

  ON_DEBUG debug() << "Terminating thread-pool resources" << endmsg;
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
StatusCode AvalancheSchedulerSvc::deactivate() {

  if ( m_isActive == ACTIVE ) {

    // Set the number of slots available to an error code
    m_freeSlots.store( 0 );

    // Empty queue
    action thisAction;
    while ( m_actionsQueue.try_pop( thisAction ) ) {};

    // This would be the last action
    m_actionsQueue.push( [this]() -> StatusCode {
      ON_VERBOSE verbose() << "Deactivating scheduler" << endmsg;
      m_isActive = INACTIVE;
      return StatusCode::SUCCESS;
    } );
  }

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

// EventSlot management
/**
 * Add event to the scheduler. There are two cases possible:
 *  1) No slot is free. A StatusCode::FAILURE is returned.
 *  2) At least one slot is free. An action which resets the slot and kicks
 * off its update is queued.
 */
StatusCode AvalancheSchedulerSvc::pushNewEvent( EventContext* eventContext ) {

  if ( !eventContext ) {
    fatal() << "Event context is nullptr" << endmsg;
    return StatusCode::FAILURE;
  }

  if ( m_freeSlots.load() == 0 ) {
    ON_DEBUG debug() << "A free processing slot could not be found." << endmsg;
    return StatusCode::FAILURE;
  }

  // no problem as push new event is only called from one thread (event loop manager)
  --m_freeSlots;

  auto action = [this, eventContext]() -> StatusCode {
    // Event processing slot forced to be the same as the wb slot
    const unsigned int thisSlotNum = eventContext->slot();
    EventSlot&         thisSlot    = m_eventSlots[thisSlotNum];
    if ( !thisSlot.complete ) {
      fatal() << "The slot " << thisSlotNum << " is supposed to be a finished event but it's not" << endmsg;
      return StatusCode::FAILURE;
    }

    ON_DEBUG debug() << "Executing event " << eventContext->evt() << " on slot " << thisSlotNum << endmsg;
    thisSlot.reset( eventContext );

    // Result status code:
    StatusCode result = StatusCode::SUCCESS;

    // promote to CR and DR the initial set of algorithms
    Cause cs = {Cause::source::Root, "RootDecisionHub"};
    if ( m_precSvc->iterate( thisSlot, cs ).isFailure() ) {
      error() << "Failed to call IPrecedenceSvc::iterate for slot " << thisSlotNum << endmsg;
      result = StatusCode::FAILURE;
    }

    if ( this->iterate().isFailure() ) {
      error() << "Failed to call AvalancheSchedulerSvc::updateStates for slot " << thisSlotNum << endmsg;
      result = StatusCode::FAILURE;
    }

    return result;
  }; // end of lambda

  // Kick off scheduling
  ON_VERBOSE {
    verbose() << "Pushing the action to update the scheduler for slot " << eventContext->slot() << endmsg;
    verbose() << "Free slots available " << m_freeSlots.load() << endmsg;
  }

  m_actionsQueue.push( action );

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AvalancheSchedulerSvc::pushNewEvents( std::vector<EventContext*>& eventContexts ) {
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
 * Get a finished event or block until one becomes available.
 */
StatusCode AvalancheSchedulerSvc::popFinishedEvent( EventContext*& eventContext ) {

  // ON_DEBUG debug() << "popFinishedEvent: queue size: " << m_finishedEvents.size() << endmsg;
  if ( m_freeSlots.load() == (int)m_maxEventsInFlight || m_isActive == INACTIVE ) {
    // ON_DEBUG debug() << "freeslots: " << m_freeSlots << "/" << m_maxEventsInFlight
    //      << " active: " << m_isActive << endmsg;
    return StatusCode::FAILURE;
  } else {
    // ON_DEBUG debug() << "freeslots: " << m_freeSlots << "/" << m_maxEventsInFlight
    //      << " active: " << m_isActive << endmsg;
    m_finishedEvents.pop( eventContext );
    ++m_freeSlots;
    ON_DEBUG debug() << "Popped slot " << eventContext->slot() << " (event " << eventContext->evt() << ")" << endmsg;
    return StatusCode::SUCCESS;
  }
}

//---------------------------------------------------------------------------
/**
 * Try to get a finished event, if not available just return a failure
 */
StatusCode AvalancheSchedulerSvc::tryPopFinishedEvent( EventContext*& eventContext ) {

  if ( m_finishedEvents.try_pop( eventContext ) ) {
    ON_DEBUG debug() << "Try Pop successful slot " << eventContext->slot() << "(event " << eventContext->evt() << ")"
                     << endmsg;
    ++m_freeSlots;
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

//--------------------------------------------------------------------------

/**
 * Loop on all slots to schedule DATAREADY algorithms, sign off ready ones or
 * detect execution stalls. To check if an event is finished the method verifies
 * that the root control flow decision of the task precedence graph is resolved
 * and there are no algorithms moving in-between INITIAL and EVTACCEPTED FSM
 * states.
 */
StatusCode AvalancheSchedulerSvc::iterate() {

  StatusCode global_sc( StatusCode::SUCCESS );

  // Retry algorithms
  const size_t retries = m_retryQueue.size();
  for ( unsigned int retryIndex = 0; retryIndex < retries; ++retryIndex ) {
    TaskSpec retryTS = std::move( m_retryQueue.front() );
    m_retryQueue.pop();
    global_sc = schedule( std::move( retryTS ) );
  }

  // Loop over all slots
  OccupancySnapshot nextSnap;
  auto              now = std::chrono::system_clock::now();
  for ( EventSlot& thisSlot : m_eventSlots ) {

    // Ignore slots without a valid context (relevant when populating scheduler for first time)
    if ( !thisSlot.eventContext ) continue;

    int iSlot = thisSlot.eventContext->slot();

    // Cache the states of the algorithms to improve readability and performance
    AlgsExecutionStates& thisAlgsStates = thisSlot.algsStates;

    StatusCode partial_sc = StatusCode::FAILURE;

    // Make an occupancy snapshot
    if ( m_snapshotInterval != std::chrono::duration<int64_t, std::milli>::min() &&
         now - m_lastSnapshot >= m_snapshotInterval ) {

      // Initialise snapshot
      if ( nextSnap.states.empty() ) {
        nextSnap.time = now;
        nextSnap.states.resize( m_eventSlots.size() );
      }

      // Store alg states
      std::vector<int>& slotStateTotals = nextSnap.states[iSlot];
      slotStateTotals.resize( AState::MAXVALUE );
      for ( uint8_t state = 0; state < AState::MAXVALUE; ++state ) {
        slotStateTotals[state] = thisSlot.algsStates.sizeOfSubset( AState( state ) );
      }

      // Add subslot alg states
      for ( auto& subslot : thisSlot.allSubSlots ) {
        for ( uint8_t state = 0; state < AState::MAXVALUE; ++state ) {
          slotStateTotals[state] += subslot.algsStates.sizeOfSubset( AState( state ) );
        }
      }
    }

    // Perform DR->SCHEDULED
    auto& drAlgs = thisAlgsStates.algsInState( AState::DATAREADY );
    for ( uint algIndex : drAlgs ) {
      const std::string& algName{index2algname( algIndex )};
      unsigned int       rank{m_optimizationMode.empty() ? 0 : m_precSvc->getPriority( algName )};
      bool               blocking{m_enablePreemptiveBlockingTasks ? m_precSvc->isBlocking( algName ) : false};

      partial_sc =
          schedule( TaskSpec( nullptr, algIndex, algName, rank, blocking, iSlot, thisSlot.eventContext.get() ) );

      ON_VERBOSE if ( partial_sc.isFailure() ) verbose()
          << "Could not apply transition from " << AState::DATAREADY << " for algorithm " << algName
          << " on processing slot " << iSlot << endmsg;
    }

    // Check for algorithms ready in sub-slots
    for ( auto& subslot : thisSlot.allSubSlots ) {
      auto& drAlgsSubSlot = subslot.algsStates.algsInState( AState::DATAREADY );
      for ( uint algIndex : drAlgsSubSlot ) {
        const std::string& algName{index2algname( algIndex )};
        unsigned int       rank{m_optimizationMode.empty() ? 0 : m_precSvc->getPriority( algName )};
        bool               blocking{m_enablePreemptiveBlockingTasks ? m_precSvc->isBlocking( algName ) : false};
        partial_sc =
            schedule( TaskSpec( nullptr, algIndex, algName, rank, blocking, iSlot, subslot.eventContext.get() ) );
      }
    }

    if ( m_dumpIntraEventDynamics ) {
      std::stringstream s;
      s << "START, " << thisAlgsStates.sizeOfSubset( AState::CONTROLREADY ) << ", "
        << thisAlgsStates.sizeOfSubset( AState::DATAREADY ) << ", " << thisAlgsStates.sizeOfSubset( AState::SCHEDULED )
        << ", " << std::chrono::high_resolution_clock::now().time_since_epoch().count() << "\n";
      auto threads = ( m_threadPoolSize != -1 ) ? std::to_string( m_threadPoolSize )
                                                : std::to_string( std::thread::hardware_concurrency() );
      std::ofstream myfile;
      myfile.open( "IntraEventFSMOccupancy_" + threads + "T.csv", std::ios::app );
      myfile << s.str();
      myfile.close();
    }

    // Not complete because this would mean that the slot is already free!
    if ( m_precSvc->CFRulesResolved( thisSlot ) &&
         !thisSlot.algsStates.containsAny(
             {AState::CONTROLREADY, AState::DATAREADY, AState::SCHEDULED, AState::RESOURCELESS} ) &&
         !subSlotAlgsInStates( thisSlot,
                               {AState::CONTROLREADY, AState::DATAREADY, AState::SCHEDULED, AState::RESOURCELESS} ) &&
         !thisSlot.complete ) {

      thisSlot.complete = true;
      // if the event did not fail, add it to the finished events
      // otherwise it is taken care of in the error handling
      if ( m_algExecStateSvc->eventStatus( *thisSlot.eventContext ) == EventStatus::Success ) {
        ON_DEBUG debug() << "Event " << thisSlot.eventContext->evt() << " finished (slot "
                         << thisSlot.eventContext->slot() << ")." << endmsg;
        m_finishedEvents.push( thisSlot.eventContext.release() );
      }

      // now let's return the fully evaluated result of the control flow
      ON_DEBUG debug() << m_precSvc->printState( thisSlot ) << endmsg;

      thisSlot.eventContext.reset( nullptr );

    } else if ( isStalled( thisSlot ) ) {
      m_algExecStateSvc->setEventStatus( EventStatus::AlgStall, *thisSlot.eventContext );
      eventFailed( thisSlot.eventContext.get() ); // can't release yet
    }
    partial_sc.ignore();
  } // end loop on slots

  // Process snapshot
  if ( !nextSnap.states.empty() ) {
    m_lastSnapshot = nextSnap.time;
    m_snapshotCallback( std::move( nextSnap ) );
  }

  ON_VERBOSE verbose() << "Iteration done." << endmsg;
  m_needsUpdate.store( false );
  return global_sc;
}

//---------------------------------------------------------------------------
// Update algorithm state and, optionally, revise states of other downstream algorithms
StatusCode AvalancheSchedulerSvc::revise( unsigned int iAlgo, EventContext* contextPtr, AState state, bool iterate ) {
  StatusCode sc;
  auto       slotIndex = contextPtr->slot();
  EventSlot& slot      = m_eventSlots[slotIndex];
  Cause      cs        = {Cause::source::Task, index2algname( iAlgo )};

  if ( UNLIKELY( contextPtr->usesSubSlot() ) ) {
    // Sub-slot
    auto       subSlotIndex = contextPtr->subSlot();
    EventSlot& subSlot      = slot.allSubSlots[subSlotIndex];

    sc = subSlot.algsStates.set( iAlgo, state );

    if ( LIKELY( sc.isSuccess() ) ) {
      ON_VERBOSE verbose() << "Promoted " << index2algname( iAlgo ) << " to " << state << " [slot:" << slotIndex
                           << ", subslot:" << subSlotIndex << ", event:" << contextPtr->evt() << "]" << endmsg;
      // Revise states of algorithms downstream the precedence graph
      if ( iterate ) sc = m_precSvc->iterate( subSlot, cs );
    }
  } else {
    // Event level (standard behaviour)
    sc = slot.algsStates.set( iAlgo, state );

    if ( LIKELY( sc.isSuccess() ) ) {
      ON_VERBOSE verbose() << "Promoted " << index2algname( iAlgo ) << " to " << state << " [slot:" << slotIndex
                           << ", event:" << contextPtr->evt() << "]" << endmsg;
      // Revise states of algorithms downstream the precedence graph
      if ( iterate ) sc = m_precSvc->iterate( slot, cs );
    }
  }
  return sc;
}

//---------------------------------------------------------------------------

/**
 * Check if we are in present of a stall condition for a particular slot.
 * This is the case when a slot has no actions queued in the actionsQueue,
 * has no scheduled algorithms and has no algorithms with all of its dependencies
 * satisfied.
 */
bool AvalancheSchedulerSvc::isStalled( const EventSlot& slot ) const {

  if ( !slot.algsStates.containsAny( {AState::DATAREADY, AState::SCHEDULED, AState::RESOURCELESS} ) &&
       !subSlotAlgsInStates( slot, {AState::DATAREADY, AState::SCHEDULED, AState::RESOURCELESS} ) ) {

    error() << "*** Stall detected in slot " << slot.eventContext->slot() << "! ***" << endmsg;

    return true;
  }
  return false;
}

//---------------------------------------------------------------------------

/**
 * It can be possible that an event fails. In this case this method is called.
 * It dumps the state of the scheduler and marks the event as finished.
 */
void AvalancheSchedulerSvc::eventFailed( EventContext* eventContext ) {
  const uint slotIdx = eventContext->slot();

  error() << "Event " << eventContext->evt() << " on slot " << slotIdx << " failed" << endmsg;

  dumpSchedulerState( msgLevel( MSG::VERBOSE ) ? -1 : slotIdx );

  // dump temporal and topological precedence analysis (if enabled in the PrecedenceSvc)
  m_precSvc->dumpPrecedenceRules( m_eventSlots[slotIdx] );

  // Push into the finished events queue the failed context
  m_eventSlots[slotIdx].complete = true;
  m_finishedEvents.push( m_eventSlots[slotIdx].eventContext.release() );
}

//---------------------------------------------------------------------------

/**
 * Used for debugging purposes, the state of the scheduler is dumped on screen
 * in order to be inspected.
 **/
void AvalancheSchedulerSvc::dumpSchedulerState( int iSlot ) {

  // To have just one big message
  std::ostringstream outputMS;

  outputMS << "Dumping scheduler state\n"
           << "=========================================================================================\n"
           << "++++++++++++++++++++++++++++++++++++ SCHEDULER STATE ++++++++++++++++++++++++++++++++++++\n"
           << "=========================================================================================\n\n";

  //===========================================================================

  outputMS << "------------------ Last schedule: Task/Event/Slot/Thread/State Mapping "
           << "------------------\n\n";

  // Figure if TimelineSvc is available (used below to detect threads IDs)
  auto timelineSvc = serviceLocator()->service<ITimelineSvc>( "TimelineSvc", false );
  if ( !timelineSvc.isValid() || !timelineSvc->isEnabled() ) {
    outputMS << "WARNING Enable TimelineSvc in record mode (RecordTimeline = True) to trace the mapping\n";
  } else {

    // Figure optimal printout layout
    size_t indt( 0 );
    for ( auto& slot : m_eventSlots ) {

      auto& schedAlgs = slot.algsStates.algsInState( AState::SCHEDULED );
      for ( uint algIndex : schedAlgs ) {
        if ( index2algname( algIndex ).length() > indt ) indt = index2algname( algIndex ).length();
      }
    }

    // Figure the last running schedule across all slots
    for ( auto& slot : m_eventSlots ) {

      auto& schedAlgs = slot.algsStates.algsInState( AState::SCHEDULED );
      for ( uint algIndex : schedAlgs ) {

        const std::string& algoName{index2algname( algIndex )};

        outputMS << "  task: " << std::setw( indt ) << algoName << " evt/slot: " << slot.eventContext->evt() << "/"
                 << slot.eventContext->slot();

        // Try to get POSIX threads IDs the currently running tasks are scheduled to
        if ( timelineSvc.isValid() ) {
          TimelineEvent te{};
          te.algorithm = algoName;
          te.slot      = slot.eventContext->slot();
          te.event     = slot.eventContext->evt();

          if ( timelineSvc->getTimelineEvent( te ) )
            outputMS << " thread.id: 0x" << std::hex << te.thread << std::dec;
          else
            outputMS << " thread.id: [unknown]"; // this means a task has just
                                                 // been signed off as SCHEDULED,
                                                 // but has not been assigned to a thread yet
                                                 // (i.e., not running yet)
        }
        outputMS << " state: [" << m_algExecStateSvc->algExecState( algoName, *( slot.eventContext ) ) << "]\n";
      }
    }
  }

  //===========================================================================

  outputMS << "\n---------------------------- Task/CF/FSM Mapping "
           << ( 0 > iSlot ? "[all slots] --" : "[target slot] " ) << "--------------------------\n\n";

  int  slotCount   = -1;
  bool wasAlgError = ( iSlot >= 0 ) ? m_eventSlots[iSlot].algsStates.containsAny( {AState::ERROR} ) ||
                                          subSlotAlgsInStates( m_eventSlots[iSlot], {AState::ERROR} )
                                    : false;

  for ( auto& slot : m_eventSlots ) {
    ++slotCount;
    if ( slot.complete ) continue;

    outputMS << "[ slot: "
             << ( slot.eventContext->valid() ? std::to_string( slot.eventContext->slot() ) : "[ctx invalid]" )
             << "  event: "
             << ( slot.eventContext->valid() ? std::to_string( slot.eventContext->evt() ) : "[ctx invalid]" )
             << " ]:\n\n";

    if ( 0 > iSlot || iSlot == slotCount ) {

      // If an alg has thrown an error then it's not a failure of the CF/DF graph
      if ( wasAlgError ) {
        outputMS << "ERROR alg(s):";
        int   errorCount = 0;
        auto& errorAlgs  = slot.algsStates.algsInState( AState::ERROR );
        for ( uint algIndex : errorAlgs ) {
          outputMS << " " << index2algname( algIndex );
          ++errorCount;
        }
        if ( errorCount == 0 ) outputMS << " in subslot(s)";
        outputMS << "\n\n";
      } else {
        // Snapshot of the Control Flow and FSM states
        outputMS << m_precSvc->printState( slot ) << "\n";
      }

      // Mention sub slots (this is expensive if the number of sub-slots is high)
      if ( m_verboseSubSlots && !slot.allSubSlots.empty() ) {
        outputMS << "\nNumber of sub-slots: " << slot.allSubSlots.size() << "\n\n";
        auto slotID = slot.eventContext->valid() ? std::to_string( slot.eventContext->slot() ) : "[ctx invalid]";
        for ( auto& ss : slot.allSubSlots ) {
          outputMS << "[ slot: " << slotID << ", sub-slot: "
                   << ( ss.eventContext->valid() ? std::to_string( ss.eventContext->subSlot() ) : "[ctx invalid]" )
                   << ", entry: " << ss.entryPoint << ", event: "
                   << ( ss.eventContext->valid() ? std::to_string( ss.eventContext->evt() ) : "[ctx invalid]" )
                   << " ]:\n\n";
          if ( wasAlgError ) {
            outputMS << "ERROR alg(s):";
            auto& errorAlgs = ss.algsStates.algsInState( AState::ERROR );
            for ( uint algIndex : errorAlgs ) { outputMS << " " << index2algname( algIndex ); }
            outputMS << "\n\n";
          } else {
            // Snapshot of the Control Flow and FSM states in sub slot
            outputMS << m_precSvc->printState( ss ) << "\n";
          }
        }
      }
    }
  }

  //===========================================================================

  if ( 0 <= iSlot && !wasAlgError ) {
    outputMS << "\n------------------------------ Algorithm Execution States -----------------------------\n\n";
    m_algExecStateSvc->dump( outputMS, *( m_eventSlots[iSlot].eventContext ) );
  }

  outputMS << "\n=========================================================================================\n"
           << "++++++++++++++++++++++++++++++++++++++ END OF DUMP ++++++++++++++++++++++++++++++++++++++\n"
           << "=========================================================================================\n\n";

  info() << outputMS.str() << endmsg;
}

//---------------------------------------------------------------------------

StatusCode AvalancheSchedulerSvc::schedule( TaskSpec&& ts ) {

  if ( UNLIKELY( ts.blocking && m_blockingAlgosInFlight == m_maxBlockingAlgosInFlight ) ) {
    m_retryQueue.push( std::move( ts ) );
    return StatusCode::SUCCESS;
  }

  // Check if a free Algorithm instance is available
  StatusCode getAlgSC( m_algResourcePool->acquireAlgorithm( ts.algName, ts.algPtr ) );

  // If an instance is available, proceed to scheduling
  StatusCode sc;
  if ( LIKELY( getAlgSC.isSuccess() ) ) {

    // Decide how to schedule the task and schedule it
    if ( LIKELY( -100 != m_threadPoolSize ) ) {

      // Cache values before moving the TaskSpec further
      unsigned int     algIndex{ts.algIndex};
      std::string_view algName( ts.algName );
      unsigned int     algRank{ts.algRank};
      bool             blocking{ts.blocking};
      int              slotIndex{ts.slotIndex};
      EventContext*    contextPtr{ts.contextPtr};

      if ( LIKELY( !blocking ) ) {
        // Add the algorithm to the scheduled queue
        m_scheduledQueue.push( std::move( ts ) );

        // Prepare a TBB task that will execute the Algorithm according to the above queued specs
        m_arena->enqueue( AlgTask( this, serviceLocator(), m_algExecStateSvc, false ) );
        ++m_algosInFlight;

      } else { // schedule blocking algorithm in independent thread
        m_scheduledBlockingQueue.push( std::move( ts ) );

        // Schedule the blocking task in an independent thread
        ++m_blockingAlgosInFlight;
        std::thread _t( AlgTask( this, serviceLocator(), m_algExecStateSvc, true ) );
        _t.detach();

      } // end scheduling blocking Algorithm

      sc = revise( algIndex, contextPtr, AState::SCHEDULED );

      ON_DEBUG debug() << "Scheduled " << algName << " [slot:" << slotIndex << ", event:" << contextPtr->evt()
                       << ", rank:" << algRank << ", blocking:" << ( blocking ? "yes" : "no" )
                       << "]. Scheduled algorithms: " << m_algosInFlight + m_blockingAlgosInFlight
                       << ( m_enablePreemptiveBlockingTasks
                                ? " (including " + std::to_string( m_blockingAlgosInFlight ) + " - off TBB runtime)"
                                : "" )
                       << endmsg;

    } else { // Avoid scheduling via TBB if the pool size is -100. Instead, run here in the scheduler's control thread
      ++m_algosInFlight;
      sc = revise( ts.algIndex, ts.contextPtr, AState::SCHEDULED );
      AlgTask( this, serviceLocator(), m_algExecStateSvc, false )();
      --m_algosInFlight;
    }
  } else { // if no Algorithm instance available, retry later

    sc = revise( ts.algIndex, ts.contextPtr, AState::RESOURCELESS );
    // Add the algorithm to the retry queue
    m_retryQueue.push( std::move( ts ) );
  }

  ON_VERBOSE dumpSchedulerState( -1 );

  return sc;
}

//---------------------------------------------------------------------------

/**
 * The call to this method is triggered only from within the AlgTask.
 */
StatusCode AvalancheSchedulerSvc::signoff( const TaskSpec& ts ) {

  Gaudi::Hive::setCurrentContext( ts.contextPtr );

  if ( LIKELY( !ts.blocking ) )
    --m_algosInFlight;
  else
    --m_blockingAlgosInFlight;

  const AlgExecState& algstate = m_algExecStateSvc->algExecState( ts.algPtr, *( ts.contextPtr ) );
  AState              state    = algstate.execStatus().isSuccess()
                     ? ( algstate.filterPassed() ? AState::EVTACCEPTED : AState::EVTREJECTED )
                     : AState::ERROR;

  // Update algorithm state and revise the downstream states
  auto sc = revise( ts.algIndex, ts.contextPtr, state, true );

  ON_DEBUG debug() << "Executed " << ts.algName << " [slot:" << ts.slotIndex << ", event:" << ts.contextPtr->evt()
                   << ", rank:" << ts.algRank << ", blocking:" << ( ts.blocking ? "yes" : "no" )
                   << "]. Scheduled algorithms: " << m_algosInFlight + m_blockingAlgosInFlight
                   << ( m_enablePreemptiveBlockingTasks
                            ? " (including " + std::to_string( m_blockingAlgosInFlight ) + " - off TBB runtime)"
                            : "" )
                   << endmsg;

  // Prompt a call to updateStates
  m_needsUpdate.store( true );
  return sc;
}

//---------------------------------------------------------------------------

// Method to inform the scheduler about event views

StatusCode AvalancheSchedulerSvc::scheduleEventView( const EventContext* sourceContext, const std::string& nodeName,
                                                     std::unique_ptr<EventContext> viewContext ) {
  //  Prevent view nesting
  if ( sourceContext->usesSubSlot() ) {
    fatal() << "Attempted to nest EventViews at node " << nodeName << ": this is not supported" << endmsg;
    return StatusCode::FAILURE;
  }

  ON_VERBOSE verbose() << "Queuing a view for [" << viewContext.get() << "]" << endmsg;

  // It's not possible to create an std::functional from a move-capturing lambda
  // So, we have to release the unique pointer
  auto action = [this, slotIndex = sourceContext->slot(), viewContextPtr = viewContext.release(),
                 &nodeName]() -> StatusCode {
    // Attach the sub-slot to the top-level slot
    EventSlot& topSlot = this->m_eventSlots[slotIndex];

    if ( viewContextPtr ) {
      // Re-create the unique pointer
      auto viewContext = std::unique_ptr<EventContext>( viewContextPtr );
      topSlot.addSubSlot( std::move( viewContext ), nodeName );
      return StatusCode::SUCCESS;
    } else {
      // Disable the view node if there are no views
      topSlot.disableSubSlots( nodeName );
      return StatusCode::SUCCESS;
    }
  };

  m_actionsQueue.push( std::move( action ) );

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

// Sample occupancy at fixed interval (ms)
// Negative value to deactivate, 0 to snapshot every change
// Each sample, apply the callback function to the result

void AvalancheSchedulerSvc::recordOccupancy( int samplePeriod, std::function<void( OccupancySnapshot )> callback ) {

  auto action = [this, samplePeriod, callback{std::move( callback )}]() -> StatusCode {
    if ( samplePeriod < 0 ) {
      this->m_snapshotInterval = std::chrono::duration<int64_t, std::milli>::min();
    } else {
      this->m_snapshotInterval = std::chrono::duration<int64_t, std::milli>( samplePeriod );
      m_snapshotCallback       = std::move( callback );
    }
    return StatusCode::SUCCESS;
  };

  m_actionsQueue.push( std::move( action ) );
}
