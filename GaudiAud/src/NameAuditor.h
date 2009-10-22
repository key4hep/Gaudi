#ifndef ATLASAUDITOR_NAMEAUDITOR_H
#define ATLASAUDITOR_NAMEAUDITOR_H

#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/Property.h"

/**
 * @brief Prints the name of each algorithm before entering the algorithm and after leaving it
 * @author M. Shapiro, LBNL
 *
 */
class NameAuditor:public  Auditor {
private:
  enum Action { BEFORE, AFTER };

public:
  NameAuditor(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~NameAuditor();

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

private:
  /// Implementation of the auditor
  void i_doAudit(const std::string& evt, const std::string& caller, Action action);

private:
  StringArrayProperty m_types;   ///< Filter for custom event types

};

#endif
