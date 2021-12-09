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
#include "CPUCrunchSvc.h"

#include "GaudiKernel/ISvcLocator.h"

#include <sys/times.h>
#include <tbb/tick_count.h>

DECLARE_COMPONENT( CPUCrunchSvc )

#define ON_DEBUG if ( msgLevel( MSG::DEBUG ) )
#define DEBUG_MSG ON_DEBUG debug()

#define ON_VERBOSE if ( msgLevel( MSG::VERBOSE ) )
#define VERBOSE_MSG ON_VERBOSE verbose()

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CPUCrunchSvc::CPUCrunchSvc( const std::string& name, ISvcLocator* svc ) : base_class( name, svc ) {}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StatusCode CPUCrunchSvc::initialize() {
  debug() << "initialize" << endmsg;

  if ( base_class::initialize().isFailure() ) {
    error() << "Error initializing base class" << endmsg;
    return StatusCode::FAILURE;
  }

  calibrate();

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
Calibrate the crunching finding the right relation between max number to be searched and time spent.
The relation is a sqrt for times greater than 10^-4 seconds.
*/
void CPUCrunchSvc::calibrate() {
  if ( m_niters_vect.value().size() == 0 ) {
    m_niters_vect = { 0,     500,   600,   700,   800,   1000,  1300,  1600,  2000,  2300,
                      2600,  3000,  3300,  3500,  3900,  4200,  5000,  6000,  8000,  10000,
                      12000, 15000, 17000, 20000, 25000, 30000, 35000, 40000, 50000, 60000 };
    if ( !m_shortCalib ) {
      m_niters_vect.value().push_back( 80000 );
      m_niters_vect.value().push_back( 100000 );
      m_niters_vect.value().push_back( 150000 );
      m_niters_vect.value().push_back( 200000 );
    }
  }

  if ( m_niters_vect.value().at( 0 ) != 0 ) {
    warning() << "NIterationsVect[0]= " << m_niters_vect.value().at( 0 ) << " but needs to be zero. resetting it."
              << endmsg;
    m_niters_vect.value().at( 0 ) = 0;
  }

  m_times_vect.resize( m_niters_vect.value().size() );
  m_times_vect.at( 0 ) = 0;

  const unsigned int minCalibTime_us = m_minCalibTime * 1000;

  // warm it up by doing 20k iterations
  findPrimes( 20000 );

  for ( int irun = 0; irun < m_numCalibRuns; ++irun ) {

    debug() << "Starting calibration run " << irun + 1 << " ..." << endmsg;
    for ( unsigned int i = 1; i < m_niters_vect.value().size(); ++i ) {
      unsigned int niters = m_niters_vect.value().at( i );
      unsigned int trials = 30;
      do {
        auto start_cali = tbb::tick_count::now();
        findPrimes( niters );
        auto stop_cali       = tbb::tick_count::now();
        auto deltat          = ( stop_cali - start_cali ).seconds();
        m_times_vect.at( i ) = deltat * 1000000; // in microseconds
        debug() << " Calibration: # iters = " << niters << " => " << m_times_vect.at( i ) << " us" << endmsg;
        trials--;
      } while ( trials > 0 && m_times_vect.at( i ) < m_times_vect.at( i - 1 ) ); // make sure that they are monotonic

      if ( i == m_niters_vect.value().size() - 1 && minCalibTime_us != 0 ) {
        if ( m_times_vect.at( i ) < minCalibTime_us ) {
          debug() << "  increasing calib vect with " << int( m_niters_vect.value().back() * 1.2 )
                  << " iterations to reach min calib time of " << m_minCalibTime.value() << " ms " << endmsg;
          m_niters_vect.value().push_back( int( m_niters_vect.value().back() * 1.2 ) );
          m_times_vect.push_back( 0. );
        }
      }
    }
  }
  if ( m_corrFact != 1. ) {
    debug() << "Adjusting times with correction factor " << m_corrFact.value() << endmsg;
    for ( auto& t : m_times_vect ) { t = t * m_corrFact; }
  }
  debug() << "Calibration finished!" << endmsg;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

unsigned int CPUCrunchSvc::getNCaliIters( std::chrono::microseconds runtime ) const {

  unsigned int smaller_i   = 0;
  double       time        = 0.;
  bool         found       = false;
  double       corrRuntime = runtime.count(); // * m_corrFact;
  // We know that the first entry is 0, so we start to iterate from 1
  for ( unsigned int i = 1; i < m_times_vect.size(); i++ ) {
    time = m_times_vect.at( i );
    if ( time > corrRuntime ) {
      smaller_i = i - 1;
      found     = true;
      break;
    }
  }

  // Case 1: we are outside the interpolation range, we take the last 2 points
  if ( not found ) smaller_i = m_times_vect.size() - 2;

  // Case 2: we maeke a linear interpolation
  // y=mx+q
  const auto   x0 = m_times_vect.at( smaller_i );
  const auto   x1 = m_times_vect.at( smaller_i + 1 );
  const auto   y0 = m_niters_vect.value().at( smaller_i );
  const auto   y1 = m_niters_vect.value().at( smaller_i + 1 );
  const double m  = (double)( y1 - y0 ) / (double)( x1 - x0 );
  const double q  = y0 - m * x0;

  const unsigned int nCaliIters = m * corrRuntime + q;

  VERBOSE_MSG << "x0: " << x0 << " x1: " << x1 << " y0: " << y0 << " y1: " << y1 << "  m: " << m << " q: " << q
              << "  itr: " << nCaliIters << endmsg;

  return nCaliIters;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CPUCrunchSvc::findPrimes( const unsigned int n_iterations ) const {
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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Translate the required crunch time into interations, and do it
std::chrono::milliseconds CPUCrunchSvc::crunch_for( const std::chrono::milliseconds& crunchtime ) const {

  const unsigned int niters = getNCaliIters( crunchtime );

  auto start_cali = tbb::tick_count::now();
  findPrimes( niters );
  auto stop_cali = tbb::tick_count::now();

  std::chrono::milliseconds actual( int( 1000 * ( stop_cali - start_cali ).seconds() ) );

  DEBUG_MSG << "crunch for " << crunchtime.count() << " ms == " << niters << " iter. actual time: " << actual.count()
            << " ms. ratio: " << float( actual.count() ) / crunchtime.count() << endmsg;

  return actual;
}
