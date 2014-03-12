// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ICounterSvc.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"

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
  class CounterSvcAlg
    : public Algorithm
  {
  public:
    /// Constructor: A constructor of this form must be provided.
    CounterSvcAlg(const std::string& name, ISvcLocator* pSvcLocator)
      :	Algorithm(name, pSvcLocator), m_cntSvc(0) {
      declareProperty("CounterBaseName", m_counterBaseName = "CounterTest");
    }
    /// Standard Destructor
    virtual ~CounterSvcAlg() {}
    /// Initialize
    virtual StatusCode initialize()
    {
      MsgStream log(msgSvc(), name());
      StatusCode sc = service("CounterSvc", m_cntSvc, true);
      if ( !sc.isSuccess() )    {
        log << MSG::ERROR << "Could not connect to CounterSvc." << endmsg;
        return sc;
      }
      sc = m_cntSvc->create(m_counterBaseName, "EventCount", 1000, m_evtCount);
      if ( !sc.isSuccess() )    {
        log << MSG::ERROR << "Could not create counter CounterTest::EventCount." << endmsg;
        return sc;
      }
      m_total = m_cntSvc->create(m_counterBaseName, "TotalCount").counter();
      try  {
        m_total = m_cntSvc->create(m_counterBaseName, "TotalCount").counter();
      }
      catch( std::exception& e)  {
        log << MSG::ALWAYS << "Exception: " << e.what() << endmsg;
      }
      ICounterSvc::Printout p(m_cntSvc);
      m_cntSvc->print(m_counterBaseName, "EventCount", p).ignore();
      m_cntSvc->print(m_counterBaseName, p).ignore();
      //
      m_cntSvc->create(m_counterBaseName, "Eff1") ;
      m_cntSvc->create(m_counterBaseName, "Eff2") ;
      m_cntSvc->create(m_counterBaseName, "Sum") ;

      return sc ;
    }
    /// Finalize
    virtual StatusCode finalize()
    {
      MsgStream log(msgSvc(), name());
      ICounterSvc::Printout p(m_cntSvc);
      log << MSG::INFO << "Single counter:CounterTest::EventCount" << endmsg;
      m_cntSvc->print(m_counterBaseName, "EventCount", p).ignore();
      log << MSG::INFO << "Counter group: CounterTest" << endmsg;
      m_cntSvc->print(m_counterBaseName, p).ignore();
      m_cntSvc->remove(m_counterBaseName, "EventCount").ignore();
      log << MSG::INFO << "After removal CounterTest::EventCount:" << endmsg;
      m_cntSvc->print(p).ignore();
      if ( m_cntSvc ) m_cntSvc->release();
      m_cntSvc = 0;
      return StatusCode::SUCCESS;
    }
    /// Event callback
    virtual StatusCode execute()
    {
      static size_t nEvent = 0 ;
      ++nEvent ;

      ICounterSvc::CountObject cnt(m_total);
      (*m_evtCount)++;
      cnt++;

      ICounterSvc::CountObject e1 = m_cntSvc->get(m_counterBaseName,"Eff1") ;
      e1 += ( 0 == nEvent%2 ) ;

      ICounterSvc::CountObject e2 = m_cntSvc->get(m_counterBaseName,"Eff2") ;
      e2 += ( 0 == nEvent%3 ) ;

      ICounterSvc::CountObject sum = m_cntSvc->get(m_counterBaseName,"Sum") ;
      sum += nEvent ;

      return StatusCode::SUCCESS;
    }
  private:
    ICounterSvc::Counter* m_evtCount;
    ICounterSvc::Counter* m_total;
    ICounterSvc*  m_cntSvc;

    std::string m_counterBaseName;
  };

  // ==========================================================================
  DECLARE_COMPONENT(CounterSvcAlg)

} // end of namespace GaudiExamples

// ============================================================================
// The END
// ============================================================================
