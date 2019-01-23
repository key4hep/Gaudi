#ifndef ATLASAUDITOR_NAMEAUDITOR_H
#define ATLASAUDITOR_NAMEAUDITOR_H

#include "CommonAuditor.h"

/**
 * @brief Prints the name of each algorithm before entering the algorithm and after leaving it
 * @author M. Shapiro, LBNL
 * @author Marco Clemencic
 */
class NameAuditor : public CommonAuditor
{
public:
  using CommonAuditor::CommonAuditor;

private:
  /// Print a message on "before".
  void i_before( CustomEventTypeRef evt, const std::string& caller ) override;
  /// Print a message on "after".
  void i_after( CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc ) override;
};

#endif
