#include "CPUCruncher.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "HiveNumbers.h"
#include <ctime>
#include <sys/resource.h>
#include <sys/times.h>

#include <tbb/tick_count.h>
#include <thread>

std::vector<unsigned int> CPUCruncher::m_niters_vect;
std::vector<double>       CPUCruncher::m_times_vect;
CPUCruncher::CHM          CPUCruncher::m_name_ncopies_map;

DECLARE_COMPONENT( CPUCruncher )

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define DEBUG_MSG ON_DEBUG debug()

#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )
#define VERBOSE_MSG ON_VERBOSE verbose()

//------------------------------------------------------------------------------

CPUCruncher::CPUCruncher( const std::string& name, // the algorithm instance name
                          ISvcLocator*       pSvc )
    : GaudiAlgorithm( name, pSvc )
{

  declareProperty( "NIterationsVect", m_niters_vect, "Number of iterations for the calibration." );
  declareProperty( "NTimesVect", m_times_vect, "Number of seconds for the calibration." );

  // Register the algo in the static concurrent hash map in order to
  // monitor the # of copies
  CHM::accessor name_ninstances;
  m_name_ncopies_map.insert( name_ninstances, name );
  name_ninstances->second += 1;
}

CPUCruncher::~CPUCruncher()
{
  for ( uint i = 0; i < m_inputHandles.size(); ++i ) delete m_inputHandles[i];

  for ( uint i = 0; i < m_outputHandles.size(); ++i ) delete m_outputHandles[i];
}

StatusCode CPUCruncher::initialize()
{
  auto sc = GaudiAlgorithm::initialize();
  if ( !sc ) return sc;

  if ( m_times_vect.size() == 0 ) calibrate();

  // if an algorithm was setup to sleep, for whatever period, it effectively becomes I/O-bound
  if ( m_sleepFraction != 0.0f ) setIOBound( true );

  // This is a bit ugly. There is no way to declare a vector of DataObjectHandles, so
  // we need to wait until initialize when we've read in the input and output key
  // properties, and know their size, and then turn them
  // into Handles and register them with the framework by calling declareProperty. We
  // could call declareInput/declareOutput on them too.

  int i = 0;
  for ( auto k : m_inpKeys ) {
    DEBUG_MSG << "adding input key " << k << endmsg;
    m_inputHandles.push_back( new DataObjectHandle<DataObject>( k, Gaudi::v1::DataHandle::Reader, this ) );
    declareProperty( "dummy_in_" + std::to_string( i ), *( m_inputHandles.back() ) );
    i++;
  }

  i = 0;
  for ( auto k : m_outKeys ) {
    DEBUG_MSG << "adding output key " << k << endmsg;
    m_outputHandles.push_back( new DataObjectHandle<DataObject>( k, Gaudi::v1::DataHandle::Writer, this ) );
    declareProperty( "dummy_out_" + std::to_string( i ), *( m_outputHandles.back() ) );
    i++;
  }

  return sc;
}

/*
Calibrate the crunching finding the right relation between max number to be searched and time spent.
The relation is a sqrt for times greater than 10^-4 seconds.
*/
void CPUCruncher::calibrate()
{
  m_niters_vect = {0,    500,  600,  700,  800,   1000,  1300,  1600,  2000,  2300,  2600,  3000,  3300,  3500, 3900,
                   4200, 5000, 6000, 8000, 10000, 12000, 15000, 17000, 20000, 25000, 30000, 35000, 40000, 60000};
  if ( !m_shortCalib ) {
    m_niters_vect.push_back( 100000 );
    m_niters_vect.push_back( 200000 );
  }

  m_times_vect.resize( m_niters_vect.size() );
  m_times_vect[0] = 0.;

  info() << "Starting calibration..." << endmsg;
  for ( unsigned int i = 1; i < m_niters_vect.size(); ++i ) {
    unsigned long niters = m_niters_vect[i];
    unsigned int  trials = 30;
    do {
      auto start_cali = tbb::tick_count::now();
      findPrimes( niters );
      auto   stop_cali = tbb::tick_count::now();
      double deltat    = ( stop_cali - start_cali ).seconds();
      m_times_vect[i]  = deltat;
      DEBUG_MSG << "Calibration: # iters = " << niters << " => " << deltat << endmsg;
      trials--;
    } while ( trials > 0 and m_times_vect[i] < m_times_vect[i - 1] ); // make sure that they are monotonic
  }
  info() << "Calibration finished!" << endmsg;
}

unsigned long CPUCruncher::getNCaliIters( double runtime )
{

  unsigned int smaller_i = 0;
  double       time      = 0.;
  bool         found     = false;
  // We know that the first entry is 0, so we start to iterate from 1
  for ( unsigned int i = 1; i < m_times_vect.size(); i++ ) {
    time = m_times_vect[i];
    if ( time > runtime ) {
      smaller_i = i - 1;
      found     = true;
      break;
    }
  }

  // Case 1: we are outside the interpolation range, we take the last 2 points
  if ( not found ) smaller_i = m_times_vect.size() - 2;

  // Case 2: we maeke a linear interpolation
  // y=mx+q
  const double x0 = m_times_vect[smaller_i];
  const double x1 = m_times_vect[smaller_i + 1];
  const double y0 = m_niters_vect[smaller_i];
  const double y1 = m_niters_vect[smaller_i + 1];
  const double m  = ( y1 - y0 ) / ( x1 - x0 );
  const double q  = y0 - m * x0;

  const unsigned long nCaliIters = m * runtime + q;
  // always() << x0 << "<" << runtime << "<" << x1 << " Corresponding to " << nCaliIters << " iterations" << endmsg;

  return nCaliIters;
}

void CPUCruncher::findPrimes( const unsigned long int n_iterations )
{
  // Flag to trigger the allocation
  bool is_prime;

  // Let's prepare the material for the allocations
  unsigned int   primes_size = 1;
  unsigned long* primes      = new unsigned long[primes_size];
  primes[0]                  = 2;

  unsigned long i = 2;

  // Loop on numbers
  for ( unsigned long int iiter = 0; iiter < n_iterations; iiter++ ) {
    // Once at max, it returns to 0
    i += 1;

    // Check if it can be divided by the smaller ones
    is_prime = true;
    for ( unsigned long j = 2; j < i && is_prime; ++j ) {
      if ( i % j == 0 ) is_prime = false;
    } // end loop on numbers < than tested one

    if ( is_prime ) {
      // copy the array of primes (INEFFICIENT ON PURPOSE!)
      unsigned int   new_primes_size = 1 + primes_size;
      unsigned long* new_primes      = new unsigned long[new_primes_size];

      for ( unsigned int prime_index = 0; prime_index < primes_size; prime_index++ ) {
        new_primes[prime_index] = primes[prime_index];
      }
      // attach the last prime
      new_primes[primes_size] = i;

      // Update primes array
      delete[] primes;
      primes      = new_primes;
      primes_size = new_primes_size;
    } // end is prime

  } // end of while loop

  // Fool Compiler optimisations:
  for ( unsigned int prime_index = 0; prime_index < primes_size; prime_index++ )
    if ( primes[prime_index] == 4 )
      debug() << "This does never happen, but it's necessary too fool aggressive compiler optimisations!" << endmsg;

  delete[] primes;
}

//------------------------------------------------------------------------------
void CPUCruncher::declareRuntimeRequestedOutputs()
{
  //
  for ( const auto& k : dataDependencies( Gaudi::v2::DataHandle::AccessMode::Write ) ) {
    auto outputHandle = new DataObjectHandle<DataObject>( k, Gaudi::v1::DataHandle::Writer, this );
    VERBOSE_MSG << "found late-attributed output: " << outputHandle->objKey() << endmsg;
    m_outputHandles.push_back( outputHandle );
    declareProperty( "dummy_out_" + outputHandle->objKey(), *( m_outputHandles.back() ) );
  }

  initializeDataHandleHolder();

  m_declAugmented = true;
}

//------------------------------------------------------------------------------

StatusCode CPUCruncher::execute() // the execution of the algorithm
{

  if ( m_loader && !m_declAugmented ) declareRuntimeRequestedOutputs();

  float crunchtime;

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
      } while ( unif1 == 0. );

      const double normal = sqrt( -2. * log( unif1 ) ) * cos( 2 * M_PI * unif2 );

      return normal * sigma + mean;
    };

    crunchtime = fabs( getGausRandom( m_avg_runtime * ( 1. - m_sleepFraction ), m_var_runtime ) );
    // End Of temp block
  } else {
    // Should be a member.
    HiveRndm::HiveNumbers rndmgaus( randSvc(), Rndm::Gauss( m_avg_runtime * ( 1. - m_sleepFraction ), m_var_runtime ) );
    crunchtime = std::fabs( rndmgaus() );
  }

  // Prepare to sleep (even if we won't enter the following if clause for sleeping).
  // This is needed to distribute evenly among all algorithms the overhead (around sleeping) which is harmful when
  // trying to achieve uniform distribution of algorithm timings.
  const double                        dreamtime = m_avg_runtime * m_sleepFraction;
  const std::chrono::duration<double> dreamtime_duration( dreamtime );
  tbb::tick_count                     startSleeptbb;
  tbb::tick_count                     endSleeptbb;

  // Start to measure the total time here, together with the dreaming process straight ahead
  tbb::tick_count starttbb = tbb::tick_count::now();

  // If the algorithm was set as I/O-bound, we will replace requested part of crunching with plain sleeping
  if ( isIOBound() ) {
    // in this block (and not in other places around) msgLevel is checked for the same reason as above, when
    // preparing to sleep several lines above: to reduce as much as possible the overhead around sleeping
    DEBUG_MSG << "Dreaming time will be: " << dreamtime << endmsg;

    ON_DEBUG startSleeptbb = tbb::tick_count::now();
    std::this_thread::sleep_for( dreamtime_duration );
    ON_DEBUG endSleeptbb = tbb::tick_count::now();

    // actual sleeping time can be longer due to scheduling or resource contention delays
    ON_DEBUG
    {
      const double actualDreamTime = ( endSleeptbb - startSleeptbb ).seconds();
      debug() << "Actual dreaming time was: " << actualDreamTime << "s" << endmsg;
    }
  } // end of "sleeping block"

  DEBUG_MSG << "Crunching time will be: " << crunchtime << endmsg;
  const EventContext& context = Gaudi::Hive::currentContext();
  DEBUG_MSG << "Start event " << context.evt() << " in slot " << context.slot() << " on pthreadID " << std::hex
            << pthread_self() << std::dec << endmsg;

  VERBOSE_MSG << "inputs number: " << m_inputHandles.size() << endmsg;
  for ( auto& inputHandle : m_inputHandles ) {
    if ( !inputHandle->isValid() ) continue;

    VERBOSE_MSG << "get from TS: " << inputHandle->objKey() << endmsg;
    DataObject* obj = nullptr;
    for ( unsigned int i = 0; i < m_rwRepetitions; ++i ) {
      obj = inputHandle->get();
    }
    if ( obj == nullptr ) error() << "A read object was a null pointer." << endmsg;
  }

  const unsigned long n_iters = getNCaliIters( crunchtime );
  findPrimes( n_iters );

  // Return error on fraction of events if configured
  if ( m_failNEvents > 0 && context.evt() > 0 && ( context.evt() % m_failNEvents ) == 0 ) {
    return StatusCode::FAILURE;
  }

  VERBOSE_MSG << "outputs number: " << m_outputHandles.size() << endmsg;
  for ( auto& outputHandle : m_outputHandles ) {
    if ( !outputHandle->isValid() ) continue;

    VERBOSE_MSG << "put to TS: " << outputHandle->objKey() << endmsg;
    outputHandle->put( new DataObject() );
  }

  tbb::tick_count endtbb = tbb::tick_count::now();

  const double actualRuntime = ( endtbb - starttbb ).seconds();

  DEBUG_MSG << "Finish event " << context.evt()
            //      << " on pthreadID " << context.m_thread_id
            << " in " << actualRuntime << " seconds" << endmsg;

  DEBUG_MSG << "Timing: ExpectedCrunchtime= " << crunchtime << " ExpectedDreamtime= " << dreamtime
            << " ActualTotalRuntime= " << actualRuntime << " Ratio= " << ( crunchtime + dreamtime ) / actualRuntime
            << " Niters= " << n_iters << endmsg;

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

  return GaudiAlgorithm::finalize();
}

//------------------------------------------------------------------------------
