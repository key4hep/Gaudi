// NameAuditor:
//  An auditor that prints the name of each algorithm method before
// and after it is called///

#include "NameAuditor.h"

#include "GaudiKernel/MsgStream.h"

DECLARE_COMPONENT(NameAuditor)

NameAuditor::NameAuditor(const std::string& name, ISvcLocator* pSvcLocator):
  CommonAuditor(name, pSvcLocator) {
}

void NameAuditor::i_before(CustomEventTypeRef evt, const std::string& caller)
{
  MsgStream log( msgSvc(), name() );
  log << MSG::INFO << "About to Enter " << caller << " with auditor trigger "
      << evt << endmsg;
}

void NameAuditor::i_after(CustomEventTypeRef evt, const std::string& caller, const StatusCode&)
{
  MsgStream log( msgSvc(), name() );
  log << MSG::INFO << "Just Exited " << caller << " with auditor trigger "
      << evt << endmsg;
}
