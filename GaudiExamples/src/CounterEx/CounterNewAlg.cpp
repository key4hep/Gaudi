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
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "Gaudi/Timers.h"

#include "GaudiKernel/RndmGenerators.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/Producer.h"
// ============================================================================

// ============================================================================
/** @file
 *  Simple example showing the usage of the new 'counter' facilities
 *  (with a simple 'producer' type of functional algorithm)
 *
 *  @see GaudiAlgorithm
 *  @see StatEntity
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date 2008-08-06
 *
 *  @author Andrea Valassi
 *  @date 2018-02-13
 */
// ============================================================================

// ============================================================================
/** @class CounterNewAlg
 *
 *  Simple example showing the usage of the new 'counter' facilities
 *  (with a simple 'producer' type of functional algorithm)
 *
 *  @see GaudiAlgorithm
 *  @see StatEntity
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date 2008-08-06
 *
 *  @author Andrea Valassi
 *  @date 2018-02-13
 */
// ============================================================================

class CounterNewAlg : public Gaudi::Functional::Producer<int()> {
public:
  int operator()() const override;

  CounterNewAlg( const std::string& name, ISvcLocator* pSvc )
      : Producer( name, pSvc, KeyValue( "OutputLocation", "dummy" ) ) {
    setProperty( "StatPrint", "true" ).ignore();
  }

private:
  // counters
  mutable StatEntity   m_assign_counter{this, "assign"};
  mutable StatEntity   m_eff_counter{this, "eff"};
  mutable StatEntity   m_executed_counter{this, "executed"};
  mutable StatEntity   m_G_counter{this, "G"};
  mutable StatEntity   m_g2_counter{this, "g2"};
  mutable StatEntity   m_gauss_counter{this, "gauss"};
  mutable StatEntity   m_Gneg_counter{this, "Gneg"};
  mutable StatEntity   m_Gpos_counter{this, "Gpos"};
  mutable StatEntity   m_NG_counter{this, "NG"};
  mutable Gaudi::Timer m_timer{this, "time"};
};
// ============================================================================

// ============================================================================
DECLARE_COMPONENT( CounterNewAlg )
// ============================================================================

// ============================================================================
int CounterNewAlg::operator()() const {

  auto timeit = m_timer();
  // count overall number of executions:
  ++m_executed_counter;

  Rndm::Numbers gauss( randSvc(), Rndm::Gauss( 0.0, 1.0 ) );
  Rndm::Numbers poisson( randSvc(), Rndm::Poisson( 5.0 ) );

  // 'accumulate' gauss
  const double value = gauss();

  m_gauss_counter += value;
  m_g2_counter += value * value;

  ( 0 < value ) ? ++m_Gpos_counter : ++m_Gneg_counter;

  StatEntity& stat1 = m_NG_counter;
  StatEntity& stat2 = m_G_counter;

  const int num = (int)poisson();
  for ( int i = 0; i < num; ++i ) {
    stat1++;
    stat2 += gauss();
  }

  // assignement
  m_assign_counter = value;

  // counter of efficiency
  m_eff_counter += ( 0 < value );

  // print the statistics every 1000 events
  StatEntity& executed = m_executed_counter;
  const int   print    = (int)executed.flag();
  if ( 0 == print % 1000 ) {
    info() << " Event number " << print << endmsg;
    always() << "Number of counters : " << nCounters();
    this->forEachCounter(
        [&]( const std::string& label, const auto& counter ) { counter.print( this->always() << '\n', label ); } );
    always() << endmsg;
    info() << " Efficiency (binomial counter: \"eff\"): (" << m_eff_counter.eff() * 100.0 << " +- "
           << m_eff_counter.effErr() * 100.0 << ")%" << endmsg;
  }

  return 42;
}
// ============================================================================
