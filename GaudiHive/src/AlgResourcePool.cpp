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
#include "AlgResourcePool.h"
#include "GaudiAlg/GaudiSequencer.h"
#include "GaudiKernel/ISvcLocator.h"

// C++
#include <functional>
#include <queue>
#include <sstream>

// DP TODO: Manage SmartIFs and not pointers to algorithms

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_COMPONENT( AlgResourcePool )

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define DEBUG_MSG ON_DEBUG debug()

//---------------------------------------------------------------------------

// Destructor
AlgResourcePool::~AlgResourcePool() {
  for ( auto& algoId_algoQueue : m_algqueue_map ) {
    auto* queue = algoId_algoQueue.second;
    delete queue;
  }
}

//---------------------------------------------------------------------------

// Initialize the pool with the list of algorithms known to the IAlgManager
StatusCode AlgResourcePool::initialize() {

  StatusCode sc( Service::initialize() );
  if ( !sc.isSuccess() ) warning() << "Base class could not be started" << endmsg;

  // Try to recover the topAlgList from the ApplicationManager for backward-compatibility
  if ( m_topAlgNames.value().empty() ) {
    info() << "TopAlg list empty. Recovering the one of Application Manager" << endmsg;
    const Gaudi::Utils::TypeNameString appMgrName( "ApplicationMgr/ApplicationMgr" );
    SmartIF<IProperty>                 appMgrProps( serviceLocator()->service( appMgrName ) );
    m_topAlgNames.assign( appMgrProps->getProperty( "TopAlg" ) );
  }

  sc = decodeTopAlgs();
  if ( sc.isFailure() ) warning() << "Algorithms could not be properly decoded." << endmsg;

  // let's assume all resources are there
  m_available_resources.set();
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::start() {

  StatusCode startSc = Service::start();
  if ( !startSc.isSuccess() ) return startSc;

  // sys-Start the algorithms
  for ( auto& ialgo : m_algList ) {
    startSc = ialgo->sysStart();
    if ( startSc.isFailure() ) {
      error() << "Unable to start Algorithm: " << ialgo->name() << endmsg;
      return startSc;
    }
  }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::acquireAlgorithm( std::string_view name, IAlgorithm*& algo, bool blocking ) {

  std::hash<std::string_view> hash_function;
  size_t                      algo_id        = hash_function( name );
  auto                        itQueueIAlgPtr = m_algqueue_map.find( algo_id );

  if ( itQueueIAlgPtr == m_algqueue_map.end() ) {
    error() << "Algorithm " << name << " requested, but not recognised" << endmsg;
    algo = nullptr;
    return StatusCode::FAILURE;
  }

  StatusCode sc;
  if ( blocking ) {
    itQueueIAlgPtr->second->pop( algo );
  } else {
    if ( !itQueueIAlgPtr->second->try_pop( algo ) ) {
      if ( m_countAlgInstMisses ) {
        auto result = m_algInstanceMisses.find( name );
        if ( result != m_algInstanceMisses.end() )
          ++( result->second );
        else
          m_algInstanceMisses[name] = 1;
      }
      sc = StatusCode::FAILURE;
    }
  }

  // Note that reentrant algorithms are not consumed so we put them
  // back immediately in the queue at the end of this function.
  // Now we may still be called again in between and get this
  // error. In such a case, the Scheduler will retry later.
  // This is of course not optimal, but should only happen very
  // seldom and thud won't affect the global efficiency
  if ( sc.isFailure() )
    DEBUG_MSG << "No instance of algorithm " << name << " could be retrieved in non-blocking mode" << endmsg;

  // if (m_lazyCreation ) {
  //    TODO: fill the lazyCreation part
  // }
  if ( sc.isSuccess() ) {
    state_type requirements = m_resource_requirements[algo_id];
    m_resource_mutex.lock();
    if ( requirements.is_subset_of( m_available_resources ) ) {
      m_available_resources ^= requirements;
    } else {
      sc = StatusCode::FAILURE;
      error() << "Failure to allocate resources of algorithm " << name << endmsg;
      // in case of not reentrant, push it back. Reentrant ones are pushed back
      // in all cases further down
      if ( 0 != algo->cardinality() ) { itQueueIAlgPtr->second->push( algo ); }
    }
    m_resource_mutex.unlock();
    if ( 0 == algo->cardinality() ) {
      // push back reentrant algorithms immediately as it can be reused
      itQueueIAlgPtr->second->push( algo );
    }
  }
  return sc;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::releaseAlgorithm( std::string_view name, IAlgorithm*& algo ) {

  std::hash<std::string_view> hash_function;
  size_t                      algo_id = hash_function( name );

  // release resources used by the algorithm
  m_resource_mutex.lock();
  m_available_resources |= m_resource_requirements[algo_id];
  m_resource_mutex.unlock();

  // release algorithm itself if not reentrant
  if ( 0 != algo->cardinality() ) { m_algqueue_map[algo_id]->push( algo ); }
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::acquireResource( std::string_view name ) {
  m_resource_mutex.lock();
  m_available_resources[m_resource_indices[name]] = false;
  m_resource_mutex.unlock();
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::releaseResource( std::string_view name ) {
  m_resource_mutex.lock();
  m_available_resources[m_resource_indices[name]] = true;
  m_resource_mutex.unlock();
  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::flattenSequencer( Gaudi::Algorithm* algo, ListAlg& alglist, unsigned int recursionDepth ) {

  StatusCode sc = StatusCode::SUCCESS;

  if ( algo->isSequence() ) {
    auto seq = dynamic_cast<Gaudi::Sequence*>( algo );
    if ( seq == 0 ) {
      error() << "Unable to dcast Algorithm " << algo->name() << " to a Sequence, but it has isSequence==true"
              << endmsg;
      return StatusCode::FAILURE;
    }

    auto subAlgorithms = seq->subAlgorithms();

    // Recursively unroll
    ++recursionDepth;

    for ( auto subalgo : *subAlgorithms ) {
      sc = flattenSequencer( subalgo, alglist, recursionDepth );
      if ( sc.isFailure() ) {
        error() << "Algorithm " << subalgo->name() << " could not be flattened" << endmsg;
        return sc;
      }
    }
  } else {
    alglist.emplace_back( algo );
    return sc;
  }
  return sc;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::decodeTopAlgs() {

  SmartIF<IAlgManager> algMan( serviceLocator() );
  if ( !algMan.isValid() ) {
    error() << "Algorithm manager could not be properly fetched." << endmsg;
    return StatusCode::FAILURE;
  }

  // Useful lambda not to repeat ourselves --------------------------
  auto createAlg = [&algMan, this]( const std::string& item_type, const std::string& item_name, IAlgorithm*& algo ) {
    StatusCode createAlgSc = algMan->createAlgorithm( item_type, item_name, algo, true, false );
    if ( createAlgSc.isFailure() )
      this->warning() << "Algorithm " << item_type << "/" << item_name << " could not be created." << endmsg;
  };
  // End of lambda --------------------------------------------------

  StatusCode sc = StatusCode::SUCCESS;

  // Fill the top algorithm list ----
  const std::vector<std::string>& topAlgNames = m_topAlgNames.value();
  for ( auto& name : topAlgNames ) {
    IAlgorithm* algo( nullptr );

    Gaudi::Utils::TypeNameString item( name );
    const std::string&           item_name = item.name();
    const std::string&           item_type = item.type();
    SmartIF<IAlgorithm>          algoSmartIF( algMan->algorithm( item_name, false ) );

    if ( !algoSmartIF.isValid() ) {
      createAlg( item_type, item_name, algo );
      algoSmartIF = algo;
    }

    algoSmartIF->sysInitialize().ignore();
    m_topAlgList.push_back( algoSmartIF );
  }
  // Top algorithm list filled ----

  // Now we unroll it ----
  for ( auto& algoSmartIF : m_topAlgList ) {
    Gaudi::Algorithm* algorithm = dynamic_cast<Gaudi::Algorithm*>( algoSmartIF.get() );
    if ( !algorithm ) {
      fatal() << "Conversion from IAlgorithm to Gaudi::Algorithm failed" << endmsg;
      return StatusCode::FAILURE;
    }
    sc = flattenSequencer( algorithm, m_flatUniqueAlgList );
  }
  // stupid O(N^2) unique-ification..
  for ( auto i = begin( m_flatUniqueAlgList ); i != end( m_flatUniqueAlgList ); ++i ) {
    auto n = next( i );
    while ( n != end( m_flatUniqueAlgList ) ) {
      if ( *n == *i )
        n = m_flatUniqueAlgList.erase( n );
      else
        ++n;
    }
  }
  if ( msgLevel( MSG::DEBUG ) ) {
    debug() << "List of algorithms is: " << endmsg;
    for ( auto& algo : m_flatUniqueAlgList )
      debug() << "  o " << algo->type() << "/" << algo->name() << " @ " << algo << endmsg;
  }

  // Unrolled ---

  // Now let's manage the clones
  unsigned int           resource_counter( 0 );
  std::hash<std::string> hash_function;
  for ( auto& ialgoSmartIF : m_flatUniqueAlgList ) {

    const std::string& item_name = ialgoSmartIF->name();

    verbose() << "Treating resource management and clones of " << item_name << endmsg;

    Gaudi::Algorithm* algo = dynamic_cast<Gaudi::Algorithm*>( ialgoSmartIF.get() );
    if ( !algo ) {
      fatal() << "Conversion from IAlgorithm to Gaudi::Algorithm failed" << endmsg;
      return StatusCode::FAILURE;
    }
    const std::string& item_type = algo->type();

    size_t                  algo_id = hash_function( item_name );
    concurrentQueueIAlgPtr* queue   = new concurrentQueueIAlgPtr();
    m_algqueue_map[algo_id]         = queue;

    // DP TODO Do it properly with SmartIFs, also in the queues
    IAlgorithm* ialgo( ialgoSmartIF.get() );

    queue->push( ialgo );
    m_algList.push_back( ialgo );
    if ( ialgo->isReEntrant() ) {
      if ( ialgo->cardinality() != 0 ) {
        info() << "Algorithm " << ialgo->name() << " is ReEntrant, but Cardinality was set to " << ialgo->cardinality()
               << endmsg;
        m_n_of_allowed_instances[algo_id] = ialgo->cardinality();
      } else {
        m_n_of_allowed_instances[algo_id] = 1;
      }
    } else if ( ialgo->isClonable() ) {
      m_n_of_allowed_instances[algo_id] = ialgo->cardinality();
    } else {
      if ( ialgo->cardinality() == 1 ) {
        m_n_of_allowed_instances[algo_id] = 1;
      } else {
        if ( !m_overrideUnClonable ) {
          info() << "Algorithm " << ialgo->name() << " is un-Clonable but Cardinality was set to "
                 << ialgo->cardinality() << ". Only creating 1 instance" << endmsg;
          m_n_of_allowed_instances[algo_id] = 1;
        } else {
          warning() << "Overriding UnClonability of Algorithm " << ialgo->name() << ". Setting Cardinality to "
                    << ialgo->cardinality() << endmsg;
          m_n_of_allowed_instances[algo_id] = ialgo->cardinality();
        }
      }
    }
    m_n_of_created_instances[algo_id] = 1;

    state_type requirements( 0 );

    for ( auto& resource_name : ialgo->neededResources() ) {
      auto ret = m_resource_indices.emplace( resource_name, resource_counter );
      // insert successful means == wasn't known before. So increment counter
      if ( ret.second ) ++resource_counter;
      // Resize for every algorithm according to the found resources
      requirements.resize( resource_counter );
      // in any case the return value holds the proper product index
      requirements[ret.first->second] = true;
    }

    m_resource_requirements[algo_id] = requirements;

    // potentially create clones; if not lazy creation we have to do it now
    if ( !m_lazyCreation ) {
      for ( unsigned int i = 1, end = m_n_of_allowed_instances[algo_id]; i < end; ++i ) {
        debug() << "type/name to create clone of: " << item_type << "/" << item_name << endmsg;
        IAlgorithm* ialgoClone( nullptr );
        createAlg( item_type, item_name, ialgoClone );
        ialgoClone->setIndex( i );
        if ( ialgoClone->sysInitialize().isFailure() ) {
          error() << "unable to initialize Algorithm clone " << ialgoClone->name() << endmsg;
          sc = StatusCode::FAILURE;
          // FIXME: should we delete this failed clone?
        } else {
          queue->push( ialgoClone );
          m_n_of_created_instances[algo_id] += 1;
        }
      }
    }
  }

  // Now resize all the requirement bitsets to the same size
  for ( auto& kv : m_resource_requirements ) { kv.second.resize( resource_counter ); }

  // Set all resources to be available
  m_available_resources.resize( resource_counter );
  m_available_resources.set();

  return sc;
}

//---------------------------------------------------------------------------

std::list<IAlgorithm*> AlgResourcePool::getFlatAlgList() {
  m_flatUniqueAlgPtrList.clear();
  for ( auto algoSmartIF : m_flatUniqueAlgList ) m_flatUniqueAlgPtrList.push_back( algoSmartIF.get() );
  return m_flatUniqueAlgPtrList;
}

//---------------------------------------------------------------------------

std::list<IAlgorithm*> AlgResourcePool::getTopAlgList() {
  m_topAlgPtrList.clear();
  for ( auto algoSmartIF : m_topAlgList ) m_topAlgPtrList.push_back( algoSmartIF.get() );
  return m_topAlgPtrList;
}

//---------------------------------------------------------------------------
void AlgResourcePool::dumpInstanceMisses() const {

  std::multimap<unsigned int, std::string_view, std::greater<unsigned int>> sortedAlgInstanceMisses;

  for ( auto& p : m_algInstanceMisses ) sortedAlgInstanceMisses.insert( {p.second, p.first} );

  // determine optimal indentation
  int indnt = std::to_string( sortedAlgInstanceMisses.cbegin()->first ).length();

  std::ostringstream out;

  out << "Hit parade of algorithm instance misses:\n"
      << std::right << std::setfill( ' ' )
      << " ===============================================================================\n"
      << std::setw( indnt + 7 ) << "Misses "
      << "| Algorithm (# of clones) \n"
      << " ===============================================================================\n";

  std::hash<std::string_view> hash_function;

  out << std::right << std::setfill( ' ' );
  for ( const auto& p : sortedAlgInstanceMisses ) {
    out << std::setw( indnt + 7 ) << std::to_string( p.first ) + " "
        << "  " << p.second << " (" << m_n_of_allowed_instances.at( hash_function( p.second ) ) << ")\n";
  }

  info() << out.str() << endmsg;
}

//---------------------------------------------------------------------------

StatusCode AlgResourcePool::stop() {

  StatusCode stopSc = Service::stop();
  if ( !stopSc.isSuccess() ) return stopSc;

  // sys-Stop the algorithm
  for ( auto& ialgo : m_algList ) {
    stopSc = ialgo->sysStop();
    if ( stopSc.isFailure() ) {
      error() << "Unable to stop Algorithm: " << ialgo->name() << endmsg;
      return stopSc;
    }
  }
  if ( m_countAlgInstMisses ) dumpInstanceMisses();

  return StatusCode::SUCCESS;
}

StatusCode AlgResourcePool::finalize() {
  // we do not need to hold the ref counts of the algorithms anymore
  // (this triggers algorithms destructors)
  m_topAlgList.clear();
  m_algList.clear();
  m_flatUniqueAlgList.clear();
  return extends::finalize();
}