#ifndef     GAUDIAUDITOR_MemStatAuditor_H
#define     GAUDIAUDITOR_MemStatAuditor_H 1

/** @class MemStatAuditor MemStatAuditor.h GaudiAud/MemStatAudit.h

    Just a minor modification of MemoryAuditor to allow
    the output memory statistics table to be printed

    @author  Vanya Belyaev
    @author  Marco Clemencic
    @date    04/02/2001
*/
#include "MemoryAuditor.h"

class MemStatAuditor:public  MemoryAuditor {
public:
  MemStatAuditor(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~MemStatAuditor();

  virtual StatusCode initialize();
private:
  /// Re-implement i_before to avoid monitoring the memory usage before a function.
  virtual void i_before(CustomEventTypeRef evt, const std::string& caller);

  virtual void i_printinfo(const std::string& msg, CustomEventTypeRef evt, const std::string& caller);

  SmartIF<IChronoStatSvc>& statSvc() { return m_stat; }
  SmartIF<IChronoStatSvc> m_stat;

  /// vsize of the previous call to printinfo
  double m_vSize ;
};

#endif  //  GAUDIAUDITOR_MemStatAuditor_H

