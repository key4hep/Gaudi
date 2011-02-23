#ifndef GAUDIAUD_CHRONOAUDITOR_H
#define GAUDIAUD_CHRONOAUDITOR_H

#include "GaudiKernel/Auditor.h"

class IChronoStatSvc;

/** @class ChronoAuditor
    Monitors the cpu time usage of each algorithm

    @author David Quarrie
*/
class ChronoAuditor : virtual public Auditor {
public:
  ChronoAuditor(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~ChronoAuditor();
  virtual void beforeInitialize(INamedInterface* alg);
  virtual void afterInitialize(INamedInterface* alg);
  virtual void beforeReinitialize(INamedInterface* alg);
  virtual void afterReinitialize(INamedInterface* alg);
  virtual void beforeExecute(INamedInterface* alg);
  virtual void afterExecute(INamedInterface* alg, const StatusCode&);
  virtual void beforeBeginRun(INamedInterface* alg);
  virtual void afterBeginRun(INamedInterface *alg);
  virtual void beforeEndRun(INamedInterface* alg);
  virtual void afterEndRun(INamedInterface *alg);
  virtual void beforeFinalize(INamedInterface* alg);
  virtual void afterFinalize(INamedInterface* alg);

  virtual void before(CustomEventTypeRef evt, const std::string& caller);
  virtual void after(CustomEventTypeRef evt, const std::string& caller, const StatusCode& );

  IChronoStatSvc* chronoSvc( ) { return m_chronoSvc; }
private:
  IChronoStatSvc* m_chronoSvc;
  StringArrayProperty m_types;

};

#endif
