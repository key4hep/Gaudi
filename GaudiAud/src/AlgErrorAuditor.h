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
  virtual ~AlgErrorAuditor();

  virtual StatusCode initialize();
  virtual StatusCode finalize();

  virtual void beforeExecute(INamedInterface* alg);
  virtual void afterExecute(INamedInterface* alg, const StatusCode&);
private:

  BooleanProperty m_abort, m_throw;

  void incrMap(const std::string& algName, int level);

  int m_error;
  int m_fatal;

  std::map <std::string, int> m_algMap[2];

};

#endif
