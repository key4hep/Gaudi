#ifndef ATLASAUDITOR_MEMORYAUDITOR_H
#define ATLASAUDITOR_MEMORYAUDITOR_H

// ClassName: MemoryAuditor
//
// Description:  Monitors the memory use of each algorithm
//
// Author: M. Shapiro, LBNL
//
#include "CommonAuditor.h"

class MemoryAuditor:public CommonAuditor {
public:
  MemoryAuditor(const std::string& name, ISvcLocator* pSvcLocator);

private:
  /// Default (catch-all) "before" Auditor hook
  virtual void i_before(CustomEventTypeRef evt, const std::string& caller);

  /// Default (catch-all) "after" Auditor hook
  virtual void i_after(CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc);

  bool i_printinfo(const std::string& msg, CustomEventTypeRef evt, const std::string& caller);
};

#endif
