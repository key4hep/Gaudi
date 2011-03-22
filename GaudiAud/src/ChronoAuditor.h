#ifndef GAUDIAUD_CHRONOAUDITOR_H
#define GAUDIAUD_CHRONOAUDITOR_H

#include "GaudiKernel/Auditor.h"

#include "GaudiKernel/IChronoStatSvc.h"

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
  virtual void before(StandardEventType evt, INamedInterface* caller);
  virtual void before(CustomEventTypeRef evt, const std::string& caller);
  virtual void before(CustomEventTypeRef evt, INamedInterface* caller);
  //@}

  /// \name "after" Auditor hooks
  //@{
  virtual void after(StandardEventType evt, const std::string& caller, const StatusCode& sc);
  virtual void after(StandardEventType evt, INamedInterface* caller, const StatusCode& sc);
  virtual void after(CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc);
  virtual void after(CustomEventTypeRef evt, INamedInterface* caller, const StatusCode& sc);
  //@}

  IChronoStatSvc* chronoSvc( ) { return m_chronoSvc; }

private:

  /// \name i_doAudit
  /// internal functions to simplify the implementation of the handler methods.
  //@{
  /// Handle the special case of INamedInterface.
  template <Action WHEN, class EVT>
  inline void i_doAudit(EVT evt, INamedInterface* caller) {
    if (caller) i_doAudit<WHEN>(evt, caller->name());
  }
  /// Check if we need to audit the event and call the IChronoStatSvc.
  template <Action WHEN, class EVT>
  inline void i_doAudit(EVT evt, const std::string& caller) {
    std::string s(i_toStr(evt));
    if (i_auditEventType(s))
      i_doAudit<WHEN>(s + ":" + caller);
  }
  //@}

  /// Call the IChronoStatSvc.
  template <Action WHEN>
  inline void i_doAudit(const std::string& id);

  /// Convert the event to a string.
  template <class T>
  inline std::string i_toStr(T evt) const { return evt; }

  /// Check if we are requested to audit the passed event type.
  inline bool i_auditEventType(const std::string& evt) {
    // Note: there is no way to extract from a Property type the type returned by
    // value().
    const std::vector<std::string> &v = m_types.value();
    return (v.size() != 0) && (
             (v[0] != "none") &&
             (find(v.begin(), v.end(), evt) != v.end())
           );
  }

  IChronoStatSvc* m_chronoSvc;
  StringArrayProperty m_types;
};

// Partial specializations
template <>
void ChronoAuditor::i_doAudit<ChronoAuditor::BEFORE>(const std::string& id) {
  chronoSvc()->chronoStart(id);
}
template <>
void ChronoAuditor::i_doAudit<ChronoAuditor::AFTER>(const std::string& id) {
  chronoSvc()->chronoStop(id);
}
template <>
std::string ChronoAuditor::i_toStr(StandardEventType evt) const {
  return toStr(evt);
}

#endif
