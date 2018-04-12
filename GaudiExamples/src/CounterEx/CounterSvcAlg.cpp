// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ICounterSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Stat.h"

// ============================================================================
namespace GaudiExamples
{
  /** @class CounterSvcAlg
   *  simple algorothm to demonstrate the functionality of ICounterSvc
   *  @see ICounterSvc
   *  @see  CounterSvc
   *  Actually it is a slightly modified version of
   *  the class GaudiSvcTest::CounterTestAlg by Markus FRANK
   *  @author Vanya BELYAEV ibelyaev@physucs.syr.edu
   *  @date 2007-05-25
   */
  class CounterSvcAlg : public Algorithm
  {
  public:
    /// Constructor: A constructor of this form must be provided.
    using Algorithm::Algorithm;
    /// Standard Destructor
    ~CounterSvcAlg() override = default;
    /// Initialize
    StatusCode initialize() override
    {
      m_cntSvc = service( "CounterSvc", true );
      if ( !m_cntSvc ) {
        error() << "Could not connect to CounterSvc." << endmsg;
        return StatusCode::FAILURE;
        ;
      }
      auto sc = m_cntSvc->create( m_counterBaseName, "EventCount", 1000, m_evtCount );
      if ( !sc.isSuccess() ) {
        error() << "Could not create counter CounterTest::EventCount." << endmsg;
        return sc;
      }
      m_total = m_cntSvc->create( m_counterBaseName, "TotalCount" ).counter();
      try {
        m_total = m_cntSvc->create( m_counterBaseName, "TotalCount" ).counter();
      } catch ( std::exception& e ) {
        always() << "Exception: " << e.what() << endmsg;
      }
      ICounterSvc::Printout p( m_cntSvc.get() );
      m_cntSvc->print( m_counterBaseName, "EventCount", p ).ignore();
      m_cntSvc->print( m_counterBaseName, p ).ignore();
      //
      m_cntSvc->create( m_counterBaseName, "Eff1" );
      m_cntSvc->create( m_counterBaseName, "Eff2" );
      m_cntSvc->create( m_counterBaseName, "Sum" );

      return sc;
    }
    /// Finalize
    StatusCode finalize() override
    {
      ICounterSvc::Printout p( m_cntSvc.get() );
      info() << "Single counter:CounterTest::EventCount" << endmsg;
      m_cntSvc->print( m_counterBaseName, "EventCount", p ).ignore();
      info() << "Counter group: CounterTest" << endmsg;
      m_cntSvc->print( m_counterBaseName, p ).ignore();
      m_cntSvc->remove( m_counterBaseName, "EventCount" ).ignore();
      info() << "After removal CounterTest::EventCount:" << endmsg;
      m_cntSvc->print( p ).ignore();
      m_cntSvc.reset();
      return StatusCode::SUCCESS;
    }
    /// Event callback
    StatusCode execute() override
    {
      static size_t nEvent = 0;
      ++nEvent;

      ICounterSvc::CountObject cnt( m_total );
      ( *m_evtCount )++;
      cnt++;

      ICounterSvc::CountObject e1 = m_cntSvc->get( m_counterBaseName, "Eff1" );
      e1 += ( 0 == nEvent % 2 );

      ICounterSvc::CountObject e2 = m_cntSvc->get( m_counterBaseName, "Eff2" );
      e2 += ( 0 == nEvent % 3 );

      ICounterSvc::CountObject sum = m_cntSvc->get( m_counterBaseName, "Sum" );
      sum += nEvent;

      return StatusCode::SUCCESS;
    }

  private:
    ICounterSvc::Counter* m_evtCount = nullptr;
    ICounterSvc::Counter* m_total    = nullptr;
    SmartIF<ICounterSvc>  m_cntSvc   = nullptr;

    Gaudi::Property<std::string> m_counterBaseName{this, "CounterBaseName", "CounterTest"};
  };

  // ==========================================================================
  DECLARE_COMPONENT( CounterSvcAlg )

} // end of namespace GaudiExamples

// ============================================================================
// The END
// ============================================================================
