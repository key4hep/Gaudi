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
#ifndef ATLASAUDITOR_MEMORYAUDITOR_H
#define ATLASAUDITOR_MEMORYAUDITOR_H

#include "CommonAuditor.h"
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
  void i_before( CustomEventTypeRef evt, const std::string& caller ) override;

  /// Default (catch-all) "after" Auditor hook
  void i_after( CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc ) override;

  /// Report the memory usage.
  virtual void i_printinfo( const std::string& msg, CustomEventTypeRef evt, const std::string& caller );

  /// Get the process informations.
  /// Returns true if it was possible to retrieve the informations.
  inline static bool getProcInfo( procInfo& info ) { return ProcStats::instance()->fetch( info ); }
};

#endif
