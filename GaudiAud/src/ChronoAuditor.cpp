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
: Auditor(name, pSvcLocator) {
  service( "ChronoStatSvc", m_chronoSvc, true).ignore();
  declareProperty("CustomEventTypes",m_types);
}

ChronoAuditor::~ChronoAuditor(){
  m_chronoSvc->release();
}

void ChronoAuditor::before(StandardEventType evt, const std::string& caller)
{
  i_doAudit<BEFORE>(evt, caller);
}
void ChronoAuditor::before(StandardEventType evt, INamedInterface* caller)
{
  i_doAudit<BEFORE>(evt, caller);
}
void ChronoAuditor::before(CustomEventTypeRef evt, const std::string& caller)
{
  i_doAudit<BEFORE>(evt, caller);
}
void ChronoAuditor::before(CustomEventTypeRef evt, INamedInterface* caller)
{
  i_doAudit<BEFORE>(evt, caller);
}

void ChronoAuditor::after(StandardEventType evt, const std::string& caller, const StatusCode&)
{
  i_doAudit<AFTER>(evt, caller);
}
void ChronoAuditor::after(StandardEventType evt, INamedInterface* caller, const StatusCode&)
{
  i_doAudit<AFTER>(evt, caller);
}
void ChronoAuditor::after(CustomEventTypeRef evt, const std::string& caller, const StatusCode&)
{
  i_doAudit<AFTER>(evt, caller);
}
void ChronoAuditor::after(CustomEventTypeRef evt, INamedInterface* caller, const StatusCode&)
{
  i_doAudit<AFTER>(evt, caller);
}

