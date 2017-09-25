//	====================================================================
//  CounterTestAlg.cpp
//	--------------------------------------------------------------------
//
//	Package   : GaudiExamples/Example3
//
//	Author    : Markus Frank
//
//	====================================================================
// Framework include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ICounterSvc.h"
#include "GaudiKernel/Stat.h"
#include <string>

/** @class CounterTestAlg CounterTestAlg.h

    CounterTestAlg class

    @author Markus Frank
*/
namespace GaudiSvcTest
{
  class CounterTestAlg : public Algorithm
  {
    ICounterSvc::Counter* m_evtCount;
    ICounterSvc::Counter* m_total;
    ICounterSvc* m_cntSvc;

  public:
    /// Constructor: A constructor of this form must be provided.
    CounterTestAlg( const std::string& name, ISvcLocator* pSvcLocator )
        : Algorithm( name, pSvcLocator ), m_evtCount( 0 ), m_total( 0 ), m_cntSvc( 0 )
    {
    }
    /// Standard Destructor
    ~CounterTestAlg() override {}
    /// Initialize
    StatusCode initialize() override
    {
      StatusCode sc = service( "CounterSvc", m_cntSvc, true );
      if ( !sc.isSuccess() ) {
        error() << "Could not connect to CounterSvc." << endmsg;
        return sc;
      }
      sc = m_cntSvc->create( "CounterTest", "EventCount", 1000, m_evtCount );
      if ( !sc.isSuccess() ) {
        error() << "Could not create counter CounterTest::EventCount." << endmsg;
        return sc;
      }
      m_total = m_cntSvc->create( "CounterTest", "TotalCount" ).counter();
      try {
        m_total = m_cntSvc->create( "CounterTest", "TotalCount" ).counter();
      } catch ( std::exception& e ) {
        always() << "Exception: " << e.what() << endmsg;
      }
      ICounterSvc::Printout p( m_cntSvc );
      m_cntSvc->print( "CounterTest", "EventCount", p );
      m_cntSvc->print( "CounterTest", p );
      return StatusCode::SUCCESS;
    }
    /// Finalize
    StatusCode finalize() override
    {
      ICounterSvc::Printout p( m_cntSvc );
      info() << "Single counter:CounterTest::EventCount" << endmsg;
      m_cntSvc->print( "CounterTest", "EventCount", p );
      info() << "Counter group: CounterTest" << endmsg;
      m_cntSvc->print( "CounterTest", p );
      m_cntSvc->remove( "CounterTest", "EventCount" );
      info() << "After removal CounterTest::EventCount:" << endmsg;
      m_cntSvc->print( p );
      if ( m_cntSvc ) m_cntSvc->release();
      m_cntSvc = 0;
      return StatusCode::SUCCESS;
    }
    /// Event callback
    StatusCode execute() override
    {
      ICounterSvc::CountObject cnt( m_total );
      ( *m_evtCount )++;
      cnt++;
      return StatusCode::SUCCESS;
    }
  };

  DECLARE_COMPONENT( CounterTestAlg )
}
