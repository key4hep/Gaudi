/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#pragma once

// Class: ProcStats
// Description:  Reports the memory usage (virtual size and RSS) of the current process
// Author: Jim Kowalkowski (FNAL), modified by M. Shapiro (LBNL)

#if defined( __linux__ )
#  include <unistd.h>
#endif

struct procInfo {
  double vsize{ 0 }; // virtual memory size, in MB
  double rss{ 0 };   // resident set size, in MB
};

class ProcStats {

public:
  static ProcStats* instance();
  /// Fill fill_me with the current memory usage of this process.
  /// Returns false (and leaves fill_me untouched) if the information
  /// could not be retrieved (e.g. unsupported platform).
  bool fetch( procInfo& fill_me );

private:
  ProcStats() = default;

#if defined( __linux__ )
  long m_pageSize{ sysconf( _SC_PAGESIZE ) };
#endif
};
