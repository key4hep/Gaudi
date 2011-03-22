#ifndef GAUDIAUD_CHRONOAUDITOR_H
#define GAUDIAUD_CHRONOAUDITOR_H

#include "GaudiKernel/Auditor.h"

class IChronoStatSvc;

/** @class ChronoAuditor
    Monitors the cpu time usage of each algorithm

    @author David Quarrie
*/
class ChronoAuditor : virtual public Auditor {
private:
  enum Action { BEFORE, AFTER };
  
public:
  ChronoAuditor(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~ChronoAuditor();

  /// \name "before" Auditor hooks
  //@{
  virtual void before(StandardEventType evt, const std::string& caller);

  virtual void before(StandardEventType evt, INamedInterface* caller) {
    if (caller) before(evt, caller->name());
  }
  virtual void before(CustomEventTypeRef evt, const std::string& caller) {
    i_doAudit(evt, caller, BEFORE);
  }
  virtual void before(CustomEventTypeRef evt, INamedInterface* caller) {
    if (caller) before(evt, caller->name());
  }
  //@}

  /// \name "after" Auditor hooks
  //@{
  virtual void after(StandardEventType evt, const std::string& caller, const StatusCode& sc);

  virtual void after(StandardEventType evt, INamedInterface* caller, const StatusCode& sc) {
    if (caller) after(evt, caller->name(), sc);
  }
  virtual void after(CustomEventTypeRef evt, const std::string& caller, const StatusCode&) {
    i_doAudit(evt, caller, AFTER);
  }
  virtual void after(CustomEventTypeRef evt, INamedInterface* caller, const StatusCode& sc) {
    if (caller) after(evt, caller->name(), sc);
  }
  //@}
  
  IChronoStatSvc* chronoSvc( ) { return m_chronoSvc; }
  
private:
  /// Implementation of the auditor
  void i_doAudit(const std::string& evt, const std::string& caller, Action action);

  std::ostringstream m_oss;
  IChronoStatSvc* m_chronoSvc;
  StringArrayProperty m_types;
};

#endif
