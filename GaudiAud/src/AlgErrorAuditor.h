#ifndef GAUDIAUD_ALGERRORAUDITOR_H
#define GAUDIAUD_ALGERRORAUDITOR_H

#include "GaudiKernel/Auditor.h"
#include <map>
#include <string>

class IMessageSvc;

/** @class AlgErrorAuditor
    Monitors the cpu time usage of each algorithm

    @author David Quarrie
*/
class AlgErrorAuditor : public Auditor
{
public:
  using Auditor::Auditor;

  StatusCode initialize() override;
  StatusCode finalize() override;

  void beforeExecute( INamedInterface* alg ) override;
  void afterExecute( INamedInterface* alg, const StatusCode& ) override;

private:
  Gaudi::Property<bool> m_abort{this, "Abort", false, "abort job upon illegal Algorithm return code"};
  Gaudi::Property<bool> m_throw{this, "Throw", false, "throw GaudiException upon illegal Algorithm return code"};

  void incrMap( const std::string& algName, int level );

  int m_error = 0;
  int m_fatal = 0;

  std::map<std::string, int> m_algMap[2];
};

#endif
