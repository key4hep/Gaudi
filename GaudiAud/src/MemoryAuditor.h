#ifndef ATLASAUDITOR_MEMORYAUDITOR_H
#define ATLASAUDITOR_MEMORYAUDITOR_H

// ClassName: MemoryAuditor
//
// Description:  Monitors the memory use of each algorithm
//
// Author: M. Shapiro, LBNL
//
#include "GaudiKernel/Auditor.h"

class MemoryAuditor:public  Auditor {

public:
  MemoryAuditor(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~MemoryAuditor();
  virtual void beforeInitialize(INamedInterface* alg);
  virtual void afterInitialize(INamedInterface* alg);
  virtual void beforeReinitialize(INamedInterface* alg);
  virtual void afterReinitialize(INamedInterface* alg);
  virtual void beforeExecute(INamedInterface* alg);
  virtual void afterExecute(INamedInterface* alg, const StatusCode& );
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
  bool printinfo(std::string theString);
};

#endif
