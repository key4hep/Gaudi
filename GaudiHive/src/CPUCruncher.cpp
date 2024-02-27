/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "CPUCruncher.h"
#include "HiveNumbers.h"
#include <GaudiKernel/ThreadLocalContext.h>
#include <ctime>
#include <sys/resource.h>
#include <sys/times.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/tick_count.h>
#include <thread>

CPUCruncher::CHM CPUCruncher::m_name_ncopies_map;

DECLARE_COMPONENT( CPUCruncher )

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define DEBUG_MSG ON_DEBUG debug()

#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )
#define VERBOSE_MSG ON_VERBOSE verbose()

//------------------------------------------------------------------------------

CPUCruncher::CPUCruncher( const std::string& name, // the algorithm instance name
                          ISvcLocator*       pSvc )
    : Algorithm( name, pSvc ) {

  // Register the algo in the static concurrent hash map in order to
  // monitor the # of copies
  CHM::accessor name_ninstances;
  m_name_ncopies_map.insert( name_ninstances, name );
  name_ninstances->second += 1;
}

CPUCruncher::~CPUCruncher() {
  for ( uint i = 0; i < m_inputHandles.size(); ++i ) delete m_inputHandles[i];

  for ( uint i = 0; i < m_outputHandles.size(); ++i ) delete m_outputHandles[i];
}

StatusCode CPUCruncher::initialize() {
  auto sc = Algorithm::initialize();
  if ( !sc ) return sc;

  m_crunchSvc = serviceLocator()->service( "CPUCrunchSvc" );
  if ( !m_crunchSvc.isValid() ) {
    fatal() << "unable to acquire CPUCruncSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  // This is a bit ugly. There is no way to declare a vector of DataObjectHandles, so
  // we need to wait until initialize when we've read in the input and output key
  // properties, and know their size, and then turn them
  // into Handles and register them with the framework by calling declareProperty. We
  // could call declareInput/declareOutput on them too.

  int i = 0;
  for ( auto k : m_inpKeys ) {
    DEBUG_MSG << "adding input key " << k << endmsg;
    m_inputHandles.push_back( new DataObjectHandle<DataObject>( k, Gaudi::DataHandle::Reader, this ) );
    declareProperty( "dummy_in_" + std::to_string( i ), *( m_inputHandles.back() ) );
    i++;
  }

  i = 0;
  for ( auto k : m_outKeys ) {
    DEBUG_MSG << "adding output key " << k << endmsg;
    m_outputHandles.push_back( new DataObjectHandle<DataObject>( k, Gaudi::DataHandle::Writer, this ) );
    declareProperty( "dummy_out_" + std::to_string( i ), *( m_outputHandles.back() ) );
    i++;
  }

  return sc;
}

//------------------------------------------------------------------------------
void CPUCruncher::declareRuntimeRequestedOutputs() {
  //
  for ( const auto& k : outputDataObjs() ) {
    auto outputHandle = new DataObjectHandle<DataObject>( k, Gaudi::DataHandle::Writer, this );
    VERBOSE_MSG << "found late-attributed output: " << outputHandle->objKey() << endmsg;
    m_outputHandles.push_back( outputHandle );
    declareProperty( "dummy_out_" + outputHandle->objKey(), *( m_outputHandles.back() ) );
  }

  initDataHandleHolder();

  m_declAugmented = true;
}

//------------------------------------------------------------------------------

StatusCode CPUCruncher::execute() // the execution of the algorithm
{

  if ( m_loader && !m_declAugmented ) declareRuntimeRequestedOutputs();

  double crunchtime;

  if ( m_local_rndm_gen ) {
    /* This will disappear with a thread safe random number generator service.
     * Use basic Box-Muller to generate Gaussian random numbers.
     * The quality is not good for in depth study given that the generator is a
     * linear congruent.
     * Throw away basically a free number: we are in a cpu cruncher after all.
     * The seed is taken from the clock, but we could assign a seed per module to
     * ensure reproducibility.
     *
     * This is not an overkill but rather an exercise towards a thread safe
     * random number generation.
     */

    auto getGausRandom = []( double mean, double sigma ) -> double {
      unsigned int seed = std::clock();

      auto getUnifRandom = []( unsigned int& seed ) -> double {
        // from "Numerical Recipes"
        constexpr unsigned int m = 232;
        constexpr unsigned int a = 1664525;
        constexpr unsigned int c = 1013904223;
        seed                     = ( a * seed + c ) % m;
        const double unif        = double( seed ) / m;
        return unif;
      };

      double unif1, unif2;
      do {
        unif1 = getUnifRandom( seed );
        unif2 = getUnifRandom( seed );
      } while ( unif1 < std::numeric_limits<double>::epsilon() );

      const double normal = sqrt( -2. * log( unif1 ) ) * cos( 2 * M_PI * unif2 );

      return normal * sigma + mean;
    };

    crunchtime = std::abs( getGausRandom( m_avg_runtime * ( 1. - m_sleepFraction ), m_var_runtime ) );
    // End Of temp block
  } else {
    // Should be a member.
    HiveRndm::HiveNumbers rndmgaus( randSvc(), Rndm::Gauss( m_avg_runtime * ( 1. - m_sleepFraction ), m_var_runtime ) );
    crunchtime = std::abs( rndmgaus() );
  }
  unsigned int crunchtime_ms = 1000 * crunchtime;

  // Prepare to sleep (even if we won't enter the following if clause for sleeping).
  // This is needed to distribute evenly among all algorithms the overhead (around sleeping) which is harmful when
  // trying to achieve uniform distribution of algorithm timings.
  const double                        dreamtime = m_avg_runtime * m_sleepFraction;
  const std::chrono::duration<double> dreamtime_duration( dreamtime );
  tbb::tick_count                     startSleeptbb;
  tbb::tick_count                     endSleeptbb;

  // Start to measure the total time here, together with the dreaming process straight ahead
  tbb::tick_count starttbb = tbb::tick_count::now();

  DEBUG_MSG << "Crunching time will be: " << crunchtime_ms << " ms" << endmsg;
  const EventContext& context = Gaudi::Hive::currentContext();
  DEBUG_MSG << "Start event " << context.evt() << " in slot " << context.slot() << " on pthreadID " << std::hex
            << pthread_self() << std::dec << endmsg;

  VERBOSE_MSG << "inputs number: " << m_inputHandles.size() << endmsg;
  for ( auto& inputHandle : m_inputHandles ) {
    if ( !inputHandle->isValid() ) continue;

    VERBOSE_MSG << "get from TS: " << inputHandle->objKey() << endmsg;
    DataObject* obj = nullptr;
    for ( unsigned int i = 0; i < m_rwRepetitions; ++i ) { obj = inputHandle->get(); }
    if ( obj == nullptr ) error() << "A read object was a null pointer." << endmsg;
  }

  if ( m_nParallel > 1 ) {
    tbb::parallel_for( tbb::blocked_range<size_t>( 0, m_nParallel ), [&]( tbb::blocked_range<size_t> r ) {
      m_crunchSvc->crunch_for( std::chrono::milliseconds( crunchtime_ms ) );
      debug() << "CPUCrunch complete in TBB parallel for block " << r.begin() << " to " << r.end() << endmsg;
    } );
  } else {
    m_crunchSvc->crunch_for( std::chrono::milliseconds( crunchtime_ms ) );
  }

  // Return error on fraction of events if configured
  if ( m_failNEvents > 0 && context.evt() > 0 && ( context.evt() % m_failNEvents ) == 0 ) {
    return StatusCode::FAILURE;
  }

  VERBOSE_MSG << "outputs number: " << m_outputHandles.size() << endmsg;
  for ( auto& outputHandle : m_outputHandles ) {
    if ( !outputHandle->isValid() ) continue;

    VERBOSE_MSG << "put to TS: " << outputHandle->objKey() << endmsg;
    outputHandle->put( std::make_unique<DataObject>() );
  }

  tbb::tick_count endtbb        = tbb::tick_count::now();
  const double    actualRuntime = ( endtbb - starttbb ).seconds();

  DEBUG_MSG << "Finish event " << context.evt() << " in " << int( 1000 * actualRuntime ) << " ms" << endmsg;

  DEBUG_MSG << "Timing: ExpectedCrunchtime= " << crunchtime_ms << " ms. ExpectedDreamtime= " << int( 1000 * dreamtime )
            << " ms. ActualTotalRuntime= " << int( 1000 * actualRuntime )
            << " ms. Ratio= " << ( crunchtime + dreamtime ) / actualRuntime << endmsg;

  setFilterPassed( !m_invertCFD );

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------

StatusCode CPUCruncher::finalize() // the finalization of the algorithm
{
  MsgStream log( msgSvc(), name() );

  unsigned int ninstances;

  {
    CHM::const_accessor const_name_ninstances;
    m_name_ncopies_map.find( const_name_ninstances, name() );
    ninstances = const_name_ninstances->second;
  }

  constexpr double s2ms = 1000.;
  // do not show repetitions
  if ( ninstances != 0 ) {
    info() << "Summary: name= " << name() << "\t avg_runtime= " << m_avg_runtime * s2ms << "\t n_clones= " << ninstances
           << endmsg;

    CHM::accessor name_ninstances;
    m_name_ncopies_map.find( name_ninstances, name() );
    name_ninstances->second = 0;
  }

  return Algorithm::finalize();
}

//------------------------------------------------------------------------------
