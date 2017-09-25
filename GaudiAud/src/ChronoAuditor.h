#ifndef GAUDIAUD_CHRONOAUDITOR_H
#define GAUDIAUD_CHRONOAUDITOR_H

#include "CommonAuditor.h"

#include "GaudiKernel/IChronoStatSvc.h"

/** @class ChronoAuditor
    Monitors the cpu time usage of each algorithm

    @author David Quarrie
    @author Marco Clemencic
*/
class ChronoAuditor : virtual public CommonAuditor
{
public:
  ChronoAuditor( const std::string& name, ISvcLocator* pSvcLocator );

  ~ChronoAuditor() override = default;

  StatusCode initialize() override;

private:
  /// Default (catch-all) "before" Auditor hook
  void i_before( CustomEventTypeRef evt, const std::string& caller ) override;

  /// Default (catch-all) "after" Auditor hook
  void i_after( CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc ) override;

  /// Compute the id string to be used for the chrono entity.
  inline std::string i_id( CustomEventTypeRef evt, const std::string& caller ) { return caller + ":" + evt; }

  SmartIF<IChronoStatSvc>& chronoSvc() { return m_chronoSvc; }
  SmartIF<IChronoStatSvc> m_chronoSvc;
};

#endif
