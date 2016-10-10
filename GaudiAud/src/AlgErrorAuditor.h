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
class AlgErrorAuditor : virtual public Auditor {
public:
  AlgErrorAuditor(const std::string& name, ISvcLocator* pSvcLocator);
  ~AlgErrorAuditor() override = default;

  StatusCode initialize() override;
  StatusCode finalize() override;

  void beforeExecute(INamedInterface* alg) override;
  void afterExecute(INamedInterface* alg, const StatusCode&) override;
private:

  BooleanProperty m_abort, m_throw;

  void incrMap(const std::string& algName, int level);

  int m_error;
  int m_fatal;

  std::map <std::string, int> m_algMap[2];

};

#endif
