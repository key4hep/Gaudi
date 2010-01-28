// NameAuditor:
//  An auditor that prints the name of each algorithm method before
// and after it is called///

#include "NameAuditor.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AudFactory.h"

#include <sstream>

DECLARE_AUDITOR_FACTORY(NameAuditor)

NameAuditor::NameAuditor(const std::string& name, ISvcLocator* pSvcLocator) :
  Auditor(name, pSvcLocator)
{

  declareProperty("CustomEventTypes", m_types,
                  "List of custom event types to audit ([]=all, ['none']=none");

}

NameAuditor::~NameAuditor(){
}


void NameAuditor::before(StandardEventType evt, const std::string& caller)
{
  std::ostringstream oss;
  oss << evt;
  before(oss.str(), caller);
}


void NameAuditor::after(StandardEventType evt, const std::string& caller, const StatusCode& sc)
{
  std::ostringstream oss;
  oss << evt;
  after(oss.str(), caller, sc);
}

void
NameAuditor::i_doAudit(const std::string& evt, const std::string& caller, Action action)
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

  MsgStream log( msgSvc(), name() );
  if ( action==BEFORE ) {
    log << MSG::INFO << "About to Enter " << caller << " with auditor trigger "
        << evt << endmsg;
  }
  else {
    log << MSG::INFO << "Just Exited " << caller << " with auditor trigger "
        << evt << endmsg;
  }
}
