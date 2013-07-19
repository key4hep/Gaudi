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
#include <string>
#include "GaudiKernel/ICounterSvc.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"

/** @class CounterTestAlg CounterTestAlg.h

    CounterTestAlg class

    @author Markus Frank
*/
namespace GaudiSvcTest  {
  class CounterTestAlg : public Algorithm {
    ICounterSvc::Counter* m_evtCount;
    ICounterSvc::Counter* m_total;
    ICounterSvc*  m_cntSvc;
  public:
    /// Constructor: A constructor of this form must be provided.
    CounterTestAlg(const std::string& name, ISvcLocator* pSvcLocator)
    :	Algorithm(name, pSvcLocator), m_cntSvc(0) {
    }
    /// Standard Destructor
    virtual ~CounterTestAlg() {
    }
    /// Initialize
    virtual StatusCode initialize()   {
      MsgStream log(msgSvc(), name());
      StatusCode sc = service("CounterSvc", m_cntSvc, true);
      if ( !sc.isSuccess() )    {
        log << MSG::ERROR << "Could not connect to CounterSvc." << endmsg;
        return sc;
      }
      sc = m_cntSvc->create("CounterTest", "EventCount", 1000, m_evtCount);
      if ( !sc.isSuccess() )    {
        log << MSG::ERROR << "Could not create counter CounterTest::EventCount." << endmsg;
        return sc;
      }
      m_total = m_cntSvc->create("CounterTest", "TotalCount").counter();
      try  {
        m_total = m_cntSvc->create("CounterTest", "TotalCount").counter();
      }
      catch( std::exception& e)  {
        log << MSG::ALWAYS << "Exception: " << e.what() << endmsg;
      }
      ICounterSvc::Printout p(m_cntSvc);
      m_cntSvc->print("CounterTest", "EventCount", p);
      m_cntSvc->print("CounterTest", p);
      return StatusCode::SUCCESS;
    }
    /// Finalize
    virtual StatusCode finalize()   {
      MsgStream log(msgSvc(), name());
      ICounterSvc::Printout p(m_cntSvc);
      log << MSG::INFO << "Single counter:CounterTest::EventCount" << endmsg;
      m_cntSvc->print("CounterTest", "EventCount", p);
      log << MSG::INFO << "Counter group: CounterTest" << endmsg;
      m_cntSvc->print("CounterTest", p);
      m_cntSvc->remove("CounterTest", "EventCount");
      log << MSG::INFO << "After removal CounterTest::EventCount:" << endmsg;
      m_cntSvc->print(p);
      if ( m_cntSvc ) m_cntSvc->release();
      m_cntSvc = 0;
      return StatusCode::SUCCESS;
    }
    /// Event callback
    virtual StatusCode execute()    {
      ICounterSvc::CountObject cnt(m_total);
      (*m_evtCount)++;
      cnt++;
      return StatusCode::SUCCESS;
    }
  };

  DECLARE_COMPONENT(CounterTestAlg)
}
