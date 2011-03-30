#include "GaudiKernel/Auditor.h"

/// Base class with common functionalities shared by few auditor implementations.
/// @author Marco Clemencic
class CommonAuditor: public Auditor {
public:
  /** Constructor
      @param name    The algorithm object's name
      @param svcloc  A pointer to a service location service */
  CommonAuditor(const std::string& name, ISvcLocator *svcloc);
  /// Destructor
  virtual ~CommonAuditor();

  /// \name "before" Auditor hooks
  /// The default behavior is to fall back on the version accepting 2 strings,
  /// which must be implemented in the derived class.
  //@{
  virtual void before(StandardEventType evt, const std::string& caller);
  virtual void before(StandardEventType evt, INamedInterface* caller);
  virtual void before(CustomEventTypeRef evt, const std::string& caller);
  virtual void before(CustomEventTypeRef evt, INamedInterface* caller);
  //@}

  /// \name "after" Auditor hooks
  /// The default behavior is to fall back on the version accepting 2 strings,
  /// which must be implemented in the derived class.
  //@{
  virtual void after(StandardEventType evt, const std::string& caller, const StatusCode& sc);
  virtual void after(StandardEventType evt, INamedInterface* caller, const StatusCode& sc);
  virtual void after(CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc);
  virtual void after(CustomEventTypeRef evt, INamedInterface* caller, const StatusCode& sc);
  //@}

protected:

  /// catch all "before" method, implemented in the derived class
  virtual void i_before(CustomEventTypeRef evt, const std::string& caller) = 0;
  /// catch all "after" method, implemented in the derived class
  virtual void i_after(CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc) = 0;

  /// Check if we are requested to audit the passed event type.
  inline bool i_auditEventType(const std::string& evt) {
    // Note: there is no way to extract from a Property type the type returned by
    // value().
    const std::vector<std::string> &v = m_types.value();
    // we need to return true is the list is empty or when the list does't
    // start by "none" and the list contain the event we got.
    return (v.size() == 0) || (
             (v[0] != "none") &&
             (find(v.begin(), v.end(), evt) != v.end())
           );
  }

  StringArrayProperty m_types;

  /// Update handler for the obsolete property CustomEventTypes
  void i_updateCustomTypes(Property &);
  StringArrayProperty m_customTypes;
};
