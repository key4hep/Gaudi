/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
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

#include "ProcStats.h"
#include <Gaudi/Auditor.h>
#include <GaudiKernel/MsgStream.h>

/// Monitors the memory use of each algorithm
///
/// @author M. Shapiro, LBNL
/// @author Marco Clemencic
class MemoryAuditor : public Gaudi::Auditor {
public:
  using Auditor::Auditor;

protected:
  /// Default (catch-all) "before" Auditor hook
  void before( std::string const& evt, std::string const&, EventContext const& ) override;

  /// Default (catch-all) "after" Auditor hook
  void after( std::string const& evt, std::string const& caller, EventContext const&, const StatusCode& sc ) override;

  /// Report the memory usage.
  virtual void i_printinfo( std::string_view msg, std::string const& evt, std::string_view caller );
};

DECLARE_COMPONENT( MemoryAuditor )

void MemoryAuditor::before( std::string const& evt, std::string const& caller, EventContext const& ) {
  i_printinfo( "Memory usage before", evt, caller );
}

void MemoryAuditor::after( std::string const& evt, std::string const& caller, EventContext const&, const StatusCode& ) {
  i_printinfo( "Memory usage has changed after", evt, caller );
}

void MemoryAuditor::i_printinfo( std::string_view msg, std::string const& evt, std::string_view caller ) {
  /// Get the process informations.
  /// fetch true if it was possible to retrieve the informations.
  if ( procInfo pInfo; ProcStats::instance()->fetch( pInfo ) ) {
    if ( pInfo.vsize > 0 ) {
      info() << msg << " " << caller << " " << evt << " virtual size = " << pInfo.vsize << " MB"
             << " resident set size = " << pInfo.rss << " MB" << endmsg;
    }
  }
}
