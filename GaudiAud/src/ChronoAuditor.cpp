// ChronoAuditor:
// An auditor that monitors time

#ifdef __ICC
// disable icc warning #654: overloaded virtual function "B::Y" is only partially overridden in class "C"
//   TODO: there is only a partial overload of IAuditor::before and IAuditor::after
#pragma warning(disable:654)
#endif

#include "ChronoAuditor.h"
#include "GaudiKernel/AudFactory.h"

DECLARE_AUDITOR_FACTORY(ChronoAuditor)

ChronoAuditor::ChronoAuditor(const std::string& name, ISvcLocator* pSvcLocator)
: CommonAuditor(name, pSvcLocator) {
  m_chronoSvc = serviceLocator()->service("ChronoStatSvc");
}

void ChronoAuditor::i_before(CustomEventTypeRef evt, const std::string& caller)
{
  chronoSvc()->chronoStart(i_id(evt, caller));
}

void ChronoAuditor::i_after(CustomEventTypeRef evt, const std::string& caller, const StatusCode&)
{
  chronoSvc()->chronoStop(i_id(evt, caller));
}
