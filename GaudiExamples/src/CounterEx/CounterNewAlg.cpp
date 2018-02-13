// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
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
class CounterNewAlg : public Gaudi::Functional::Producer<int()>
{
public:
  int operator()() const override;

  CounterNewAlg( const std::string& name, ISvcLocator* pSvc )
      : Producer( name, pSvc, KeyValue( "OutputLocation", "dummy" ) )
  {
    setProperty( "StatPrint", "true" ).ignore();
  }

  // copy constructor is disabled
  CounterNewAlg( const CounterNewAlg& ) = delete;

  // assignement operator is disabled
  CounterNewAlg& operator=( const CounterNewAlg& ) = delete;

private:
  // counters
  StatEntity m_assign_counter{this, "assign"};
  StatEntity m_eff_counter{this, "eff"};
  StatEntity m_executed_counter{this, "executed"};
  StatEntity m_G_counter{this, "G"};
  StatEntity m_g2_counter{this, "g2"};
  StatEntity m_gauss_counter{this, "gauss"};
  StatEntity m_Gneg_counter{this, "Gneg"};
  StatEntity m_Gpos_counter{this, "Gpos"};
  StatEntity m_NG_counter{this, "NG"};
};
// ============================================================================

// ============================================================================
DECLARE_COMPONENT( CounterNewAlg )
// ============================================================================

// ============================================================================
int CounterNewAlg::operator()() const
{

  // count overall number of executions:
  ++m_executed_counter;

  Rndm::Numbers gauss( randSvc(), Rndm::Gauss( 0.0, 1.0 ) );
  Rndm::Numbers poisson( randSvc(), Rndm::Poisson( 5.0 ) );

  // 'accumulate' gauss
  const double value = gauss();

  m_gauss_counter += value;
  m_g2_counter += value * value;

  ( 0 < value ) ? ++m_Gpos_counter : ++m_Gneg_counter;

  const StatEntity& stat1 = m_NG_counter;
  const StatEntity& stat2 = m_G_counter;

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
  const StatEntity& executed = m_executed_counter;
  const int print            = (int)executed.flag();
  if ( 0 == print % 1000 ) {
    info() << " Event number " << print << endmsg;
    printStat();
    info() << " Efficiency (binomial counter: \"eff\"): (" << m_eff_counter.eff() * 100.0 << " +- "
           << m_eff_counter.effErr() * 100.0 << ")%" << endmsg;
  }

  return 42;
}
// ============================================================================
