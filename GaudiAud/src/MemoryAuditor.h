#ifndef ATLASAUDITOR_MEMORYAUDITOR_H
#define ATLASAUDITOR_MEMORYAUDITOR_H

#include "CommonAuditor.h"
#include "ProcStats.h"

/// Monitors the memory use of each algorithm
///
/// @author M. Shapiro, LBNL
/// @author Marco Clemencic
class MemoryAuditor:public CommonAuditor {
public:
  MemoryAuditor(const std::string& name, ISvcLocator* pSvcLocator);

protected:
  /// Default (catch-all) "before" Auditor hook
  virtual void i_before(CustomEventTypeRef evt, const std::string& caller);

  /// Default (catch-all) "after" Auditor hook
  virtual void i_after(CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc);

  /// Report the memory usage.
  virtual void i_printinfo(const std::string& msg, CustomEventTypeRef evt, const std::string& caller);

  /// Get the process informations.
  /// Returns true if it was possible to retrieve the informations.
  inline static bool getProcInfo(procInfo &info) {
    return ProcStats::instance()->fetch(info);
  }
};

#endif
