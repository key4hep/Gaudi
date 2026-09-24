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
// Class: ProcStats
// Purpose:  To keep statistics on memory use

#include "ProcStats.h"

#if defined( __linux__ )
#  include <fstream>
#elif defined( __APPLE__ )
#  include <mach/mach.h>
#endif

ProcStats* ProcStats::instance() {
  static ProcStats inst{};
  return &inst;
}

bool ProcStats::fetch( procInfo& f ) {
  constexpr double MB = 1.0 / ( 1024 * 1024 );
#if defined( __linux__ )
  // see proc(5): /proc/self/statm is "size resident shared text lib data dt" in pages
  // size and resident are identical to the vsize and rss fields of /proc/self/stat
  std::ifstream statm{ "/proc/self/statm" };
  unsigned long size = 0, resident = 0; // in pages
  if ( !( statm >> size >> resident ) || size == 0 ) { return false; }
  f.vsize = size * m_pageSize * MB;
  f.rss   = resident * m_pageSize * MB;
  return true;
#elif defined( __APPLE__ )
  task_basic_info_64_data_t info;
  mach_msg_type_number_t    count = TASK_BASIC_INFO_64_COUNT;
  if ( task_info( mach_task_self(), TASK_BASIC_INFO_64, reinterpret_cast<task_info_t>( &info ), &count ) !=
       KERN_SUCCESS ) {
    return false;
  }
  f.vsize = info.virtual_size * MB;
  f.rss   = info.resident_size * MB;
  return true;
#else
  return false;
#endif
}
