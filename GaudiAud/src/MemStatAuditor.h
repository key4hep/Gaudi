#ifndef     GAUDIAUDITOR_MemStatAuditor_H
#define     GAUDIAUDITOR_MemStatAuditor_H 1

/** @class MemStatAuditor MemStatAuditor.h GaudiAud/MemStatAudit.h

    Just a minor modification of MemoryAuditor to allow
    the ouput memory statistics table to be printed

    @author  Vanya Belyaev
    @date    04/02/2001
*/

#include "GaudiKernel/Auditor.h"

class IChronoStatSvc;

class MemStatAuditor:public  Auditor {

public:
  MemStatAuditor(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~MemStatAuditor();
  virtual void beforeInitialize(INamedInterface* alg);
  virtual void afterInitialize(INamedInterface* alg);
  virtual void beforeReinitialize(INamedInterface* alg);
  virtual void afterReinitialize(INamedInterface* alg);
  virtual void beforeExecute(INamedInterface* alg);
  virtual void afterExecute(INamedInterface* alg, const StatusCode &);
  virtual void beforeBeginRun(INamedInterface* alg);
  virtual void afterBeginRun(INamedInterface *alg);
  virtual void beforeEndRun(INamedInterface* alg);
  virtual void afterEndRun(INamedInterface *alg);
  virtual void beforeFinalize(INamedInterface* alg);
  virtual void afterFinalize(INamedInterface* alg);

  virtual void before(CustomEventTypeRef evt, const std::string& caller);
  virtual void after(CustomEventTypeRef evt, const std::string& caller, const StatusCode& );

  virtual StatusCode sysFinalize( );

private:

  StringArrayProperty m_types;
  bool printinfo(const std::string& theString, const std::string& Tag );
  SmartIF<IChronoStatSvc>&  statSvc() const { return m_stat; }
  mutable SmartIF<IChronoStatSvc>  m_stat;
  double m_vSize ; // vsize of the previous call to printinfo
};

#endif  //  GAUDIAUDITOR_MemStatAuditor_H

