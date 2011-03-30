#include "CommonAuditor.h"

CommonAuditor::CommonAuditor(const std::string& name, ISvcLocator *svcloc): Auditor(name, svcloc) {
  declareProperty("EventTypes", m_types,
                  "List of event types to audit ([]=all, ['none']=none)");
  declareProperty("CustomEventTypes", m_customTypes,
                  "OBSOLETE, use EventTypes instead")->declareUpdateHandler(&CommonAuditor::i_updateCustomTypes, this);
}

CommonAuditor::~CommonAuditor() {}

void CommonAuditor::i_updateCustomTypes(Property &) {
  MsgStream log(msgSvc(), name());
  log << MSG::WARNING << "Property CustomEventTypes is deprecated, use EventTypes instead" << endmsg;
  m_types.setValue(m_customTypes.value());
}

void CommonAuditor::before(StandardEventType evt, INamedInterface* caller)
{
  if (caller) before(toStr(evt), caller->name());
}
void CommonAuditor::before(StandardEventType evt, const std::string& caller)
{
  before(toStr(evt), caller);
}
void CommonAuditor::before(CustomEventTypeRef evt, INamedInterface* caller)
{
  if (caller) before(evt, caller->name());
}
void CommonAuditor::before(CustomEventTypeRef evt, const std::string& caller)
{
  if (i_auditEventType(evt)) i_before(evt, caller);
}

void CommonAuditor::after(StandardEventType evt, INamedInterface* caller, const StatusCode& sc)
{
  if (caller) after(toStr(evt), caller->name(), sc);
}
void CommonAuditor::after(StandardEventType evt, const std::string& caller, const StatusCode& sc)
{
  after(toStr(evt), caller, sc);
}
void CommonAuditor::after(CustomEventTypeRef evt, INamedInterface* caller, const StatusCode& sc)
{
  if (caller) after(evt, caller->name(), sc);
}
void CommonAuditor::after(CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc)
{
  if (i_auditEventType(evt)) i_after(evt, caller, sc);
}
