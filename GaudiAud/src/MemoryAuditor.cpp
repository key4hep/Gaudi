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
// MemoryAuditor:
//  An auditor that monitors memory usage

#include "CommonAuditor.h"
#include "GaudiKernel/MsgStream.h"
#include "ProcStats.h"

/// Monitors the memory use of each algorithm
///
/// @author M. Shapiro, LBNL
/// @author Marco Clemencic
class MemoryAuditor : public CommonAuditor {
public:
  using CommonAuditor::CommonAuditor;

protected:
  /// Default (catch-all) "before" Auditor hook
  void i_before( CustomEventTypeRef evt, std::string_view caller ) override;

  /// Default (catch-all) "after" Auditor hook
  void i_after( CustomEventTypeRef evt, std::string_view caller, const StatusCode& sc ) override;

  /// Report the memory usage.
  virtual void i_printinfo( std::string_view msg, CustomEventTypeRef evt, std::string_view caller );
};

DECLARE_COMPONENT( MemoryAuditor )

void MemoryAuditor::i_before( CustomEventTypeRef evt, std::string_view caller ) {
  i_printinfo( "Memory usage before", evt, caller );
}

void MemoryAuditor::i_after( CustomEventTypeRef evt, std::string_view caller, const StatusCode& ) {
  i_printinfo( "Memory usage has changed after", evt, caller );
}

void MemoryAuditor::i_printinfo( std::string_view msg, CustomEventTypeRef evt, std::string_view caller ) {
  /// Get the process informations.
  /// fetch true if it was possible to retrieve the informations.
  if ( procInfo pInfo; ProcStats::instance()->fetch( pInfo ) ) {
    info() << msg << " " << caller << " " << evt << " virtual size = " << pInfo.vsize << " MB"
           << " resident set size = " << pInfo.rss << " MB" << endmsg;
  }
}
