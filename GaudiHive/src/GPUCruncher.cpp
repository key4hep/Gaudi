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
#include "GPUCruncher.h"
#include "HiveNumbers.h"
#include <chrono>
#include <ctime>
#include <fmt/format.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <tbb/tick_count.h>
#include <thread>

GPUCruncher::CHM GPUCruncher::m_name_ncopies_map;

DECLARE_COMPONENT( GPUCruncher )

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define DEBUG_MSG ON_DEBUG debug()

#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )
#define VERBOSE_MSG ON_VERBOSE verbose()

//------------------------------------------------------------------------------

GPUCruncher::GPUCruncher( const std::string& name, // the algorithm instance name
                          ISvcLocator*       pSvc )
    : Algorithm( name, pSvc ) {

  // Register the algo in the static concurrent hash map in order to
  // monitor the # of copies
  CHM::accessor name_ninstances;
  m_name_ncopies_map.insert( name_ninstances, name );
  name_ninstances->second += 1;
}

GPUCruncher::~GPUCruncher() {
  for ( uint i = 0; i < m_inputHandles.size(); ++i ) delete m_inputHandles[i];

  for ( uint i = 0; i < m_outputHandles.size(); ++i ) delete m_outputHandles[i];
}

StatusCode GPUCruncher::initialize() {
  auto sc = Algorithm::initialize();
  if ( !sc ) return sc;

  pinned = Gaudi::CUDA::get_pinned_memory_resource();

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

StatusCode GPUCruncher::execute( const EventContext& ctx ) const // the execution of the algorithm
{

  double                   crunchtime;
  std::pmr::vector<double> input( pinned );
  if ( m_local_rndm_gen ) {
    /* This will disappear with a thread safe random number generator service.
     * Use basic Box-Muller to generate Gaussian random numbers.
     * The quality is not good for in depth study given that the generator is a
     * linear congruent.
     * Throw away basically a free number: we are in a ~~cpu~~ /gpu/ cruncher after all.
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

    crunchtime = fabs( getGausRandom( m_avg_runtime, m_var_runtime ) );
    // Generate input vector
    input.reserve( 40000 * crunchtime );
    for ( int i = 0; i < 40000 * crunchtime; ++i ) { input.push_back( getGausRandom( 10.0, 1.0 ) ); }
    // End Of temp block
  } else {
    // Should be a member.
    HiveRndm::HiveNumbers rndmgaus( randSvc(), Rndm::Gauss( m_avg_runtime, m_var_runtime ) );
    crunchtime = std::fabs( rndmgaus() );
    // Generate input vector
    for ( int i = 0; i < 2000 * crunchtime; ++i ) { input.push_back( rndmgaus() ); }
  }
  unsigned int crunchtime_ms = 1000 * crunchtime;

  DEBUG_MSG << "Crunching time will be: " << crunchtime_ms << " ms" << endmsg;
  DEBUG_MSG << "Start event " << ctx.evt() << " in slot " << ctx.slot() << " on pthreadID " << std::hex
            << pthread_self() << std::dec << endmsg;

  // start timer
  tbb::tick_count starttbb = tbb::tick_count::now();

  VERBOSE_MSG << "inputs number: " << m_inputHandles.size() << endmsg;
  for ( auto& inputHandle : m_inputHandles ) {
    if ( !inputHandle->isValid() ) continue;

    VERBOSE_MSG << "get from TS: " << inputHandle->objKey() << endmsg;
    DataObject* obj = nullptr;
    obj             = inputHandle->get();
    if ( obj == nullptr ) error() << "A read object was a null pointer." << endmsg;
  }

  // Use fiber sleep, should eventually be a GPU computation
  info() << "Crunching..." << endmsg;
  auto                startcrunch = std::chrono::steady_clock::now();
  std::vector<double> out{ 3.0, 5.0 };
  gpuExecute( input, out ).orThrow( "GPU_EXECUTE" );
  auto endcrunch = std::chrono::steady_clock::now();
  info() << "Crunched." << endmsg;
  fmt::print( "{}   GPU Crunch time: {}. Input length {}, output length {}.\n", name(),
              Gaudi::CUDA::SI(
                  std::chrono::duration_cast<std::chrono::milliseconds>( endcrunch - startcrunch ).count() / 1e3, "s" ),
              input.size(), out.size() );

  VERBOSE_MSG << "outputs number: " << m_outputHandles.size() << endmsg;
  for ( auto& outputHandle : m_outputHandles ) {
    if ( !outputHandle->isValid() ) continue;

    VERBOSE_MSG << "put to TS: " << outputHandle->objKey() << endmsg;
    outputHandle->put( std::make_unique<DataObject>() );
  }

  tbb::tick_count endtbb        = tbb::tick_count::now();
  const double    actualRuntime = ( endtbb - starttbb ).seconds();

  DEBUG_MSG << "Finish event " << ctx.evt() << " in " << int( 1000 * actualRuntime ) << " ms" << endmsg;

  DEBUG_MSG << "Timing: ExpectedCrunchtime= " << crunchtime_ms
            << " ms. ActualTotalRuntime= " << int( 1000 * actualRuntime )
            << " ms. Ratio= " << crunchtime / actualRuntime << endmsg;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------

StatusCode GPUCruncher::finalize() // the finalization of the algorithm
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
