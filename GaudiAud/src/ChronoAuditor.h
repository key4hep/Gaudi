/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIAUD_CHRONOAUDITOR_H
#define GAUDIAUD_CHRONOAUDITOR_H

#include "CommonAuditor.h"

#include "GaudiKernel/IChronoStatSvc.h"

/** @class ChronoAuditor
    Monitors the cpu time usage of each algorithm

    @author David Quarrie
    @author Marco Clemencic
*/
class ChronoAuditor : public CommonAuditor {
public:
  using CommonAuditor::CommonAuditor;

  StatusCode initialize() override;

private:
  /// Default (catch-all) "before" Auditor hook
  void i_before( CustomEventTypeRef evt, std::string_view caller ) override;

  /// Default (catch-all) "after" Auditor hook
  void i_after( CustomEventTypeRef evt, std::string_view caller, const StatusCode& sc ) override;

  /// Compute the id string to be used for the chrono entity.
  std::string i_id( CustomEventTypeRef evt, std::string_view caller ) { return std::string{caller} + ":" + evt; }

  SmartIF<IChronoStatSvc>& chronoSvc() { return m_chronoSvc; }
  SmartIF<IChronoStatSvc>  m_chronoSvc;
};

#endif
