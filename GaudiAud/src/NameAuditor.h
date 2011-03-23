#ifndef ATLASAUDITOR_NAMEAUDITOR_H
#define ATLASAUDITOR_NAMEAUDITOR_H

#include "CommonAuditor.h"

/**
 * @brief Prints the name of each algorithm before entering the algorithm and after leaving it
 * @author M. Shapiro, LBNL
 * @author Marco Clemencic
 */
class NameAuditor:public CommonAuditor {
public:
  NameAuditor(const std::string& name, ISvcLocator* pSvcLocator);

private:
  /// Print a message on "before".
  virtual void i_before(CustomEventTypeRef evt, const std::string& caller);
  /// Print a message on "after".
  virtual void i_after(CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc);

};

#endif
