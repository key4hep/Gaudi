// ChronoAuditor:
// An auditor that monitors time

#ifdef __ICC
// disable icc warning #654: overloaded virtual function "B::Y" is only partially overridden in class "C"
//   TODO: there is only a partial overload of IAuditor::before and IAuditor::after
#pragma warning(disable:654)
#endif

#include "ChronoAuditor.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AudFactory.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/Chrono.h"

DECLARE_AUDITOR_FACTORY(ChronoAuditor)

ChronoAuditor::ChronoAuditor(const std::string& name, ISvcLocator* pSvcLocator)
: Auditor(name, pSvcLocator) {
  service( "ChronoStatSvc", m_chronoSvc, true).ignore();
  declareProperty("CustomEventTypes",m_types);
}

ChronoAuditor::~ChronoAuditor(){
  m_chronoSvc->release();
}

void ChronoAuditor::before(StandardEventType evt, const std::string& caller)
{
  m_oss.str("");  
  m_oss << evt;
  before(m_oss.str(), caller);
}


void ChronoAuditor::after(StandardEventType evt, const std::string& caller, const StatusCode& sc)
{
  m_oss.str("");
  m_oss << evt;
  after(m_oss.str(), caller, sc);
}

void ChronoAuditor::i_doAudit(const std::string& evt, const std::string& caller, Action action)
{
  if (m_types.value().size() != 0) {
    if ( (m_types.value())[0] == "none") {
      return;
    }
    
    if ( find(m_types.value().begin(), m_types.value().end(), evt) ==
	 m_types.value().end() ) {
      return;
    }
  }

  if (action==BEFORE) {
    chronoSvc( )->chronoStart( caller + ":" + evt ) ;
  }
  else {
    chronoSvc( )->chronoStop( caller + ":" + evt ) ;
  }
}
