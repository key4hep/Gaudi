// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/Memory.cpp,v 1.1 2001/03/14 15:30:16 mato Exp $
//====================================================================
//	Memory.cpp
//--------------------------------------------------------------------
//
//	Package    : System (The LHCb System service)
//
//  Description: Information of memory usage from a given process
//
//	Author     : M.Frank
//  Created    : 13/11/00
//	Changes    :
//
//====================================================================
#define GAUDIKERNEL_MEMORY_CPP

#ifdef _WIN32
#include "process.h"
  #define getpid _getpid
#else
  #include <errno.h>
  #include <string.h>
  #include "sys/times.h"
  #include "unistd.h"
  #include "libgen.h"
  #include <cstdio>
#endif

// Framework include files
#include <limits.h>
#include "GaudiKernel/Memory.h"
#include "ProcessDescriptor.h"

/// Convert requested memory value from kByte to requested value
long System::adjustMemory( MemoryUnit unit, long value )    {
  if ( value != -1 )    {
    switch ( unit )   {
    case Byte:   break;
    case kByte:     value =       value/1024;    break;
    case MByte:     value =      (value/1024)/1024;    break;
    case GByte:     value =     ((value/1024)/1024)/1024;    break;
    case TByte:     value =    (((value/1024)/1024)/1024)/1024;    break;
    case PByte:     value =   ((((value/1024)/1024)/1024)/1024)/1024;    break;
    case EByte:     value =  (((((value/1024)/1024)/1024)/1024)/1024)/1024;    break;
    default:        value =  -1;    break;
    }
  }
  return value;
}

/// Basic Process Information: Base priority
long System::basePriority(InfoType fetch, long pid)   {
  PROCESS_BASIC_INFORMATION info;
  if ( fetch != NoFetch && getProcess()->query(pid, ProcessBasics, &info) )
    return info.BasePriority;
  return 0;
}

/// Basic Process Information: Process ID
long System::procID()   {
  static long s_pid = ::getpid();
  return s_pid;
}

/// Basic Process Information: Parent's process ID
long System::parentID(InfoType fetch, long pid)   {
  PROCESS_BASIC_INFORMATION info;
  if ( fetch != NoFetch && getProcess()->query(pid, ProcessBasics, &info) )
    return info.InheritedFromUniqueProcessId;
  return 0;
}

/// Basic Process Information: Affinity mask
long System::affinityMask(InfoType fetch, long pid)   {
  PROCESS_BASIC_INFORMATION info;
  if ( fetch != NoFetch && getProcess()->query(pid, ProcessBasics, &info) )
    return info.AffinityMask;
  return 0;
}

/// Basic Process Information: Exit status (does not really make sense for the running process, but for others!)
long System::exitStatus(InfoType fetch, long pid)   {
  PROCESS_BASIC_INFORMATION info;
  if ( fetch != NoFetch && getProcess()->query(pid, ProcessBasics, &info) )
    return info.ExitStatus;
  return -2;
}

/// Basic Process Information: priority boost
long System::priorityBoost(InfoType fetch, long pid)   {
  long info;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &info) )
    return info;
  return -2;
}

/// Basic Process Information: priority boost
long System::nonPagedMemoryPeak(MemoryUnit unit, InfoType fetch, long pid)   {
  VM_COUNTERS info;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &info) )
    return adjustMemory(unit, info.QuotaPeakNonPagedPoolUsage);
  return -2;
}

/// Basic Process Information: priority boost
long System::nonPagedMemory(MemoryUnit unit, InfoType fetch, long pid)   {
  VM_COUNTERS info;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &info) )
    return adjustMemory(unit, info.QuotaNonPagedPoolUsage);
  return -2;
}

/// System Process Limits: Maximum amount of non-paged memory this process is allowed to use
long System::nonPagedMemoryLimit(MemoryUnit unit, InfoType fetch, long pid)   {
  POOLED_USAGE_AND_LIMITS quota;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &quota) )
    return adjustMemory(unit, quota.NonPagedPoolLimit);
  return 0;
}

/// Basic Process Information: Amount of paged memory currently occupied by the process 'pid'
long System::pagedMemory(MemoryUnit unit, InfoType fetch, long pid)   {
  VM_COUNTERS info;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &info) )
    return adjustMemory(unit, info.QuotaPagedPoolUsage);
  return -2;
}

/// Basic Process Information: Maximum of paged memory occupied by the process 'pid'
long System::pagedMemoryPeak(MemoryUnit unit, InfoType fetch, long pid)   {
  VM_COUNTERS info;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &info) )
    return adjustMemory(unit, info.QuotaPeakPagedPoolUsage);
  return -2;
}

/// Basic Process Information: Amount of paged memory that can be occupied by the process 'pid'
long System::pagedMemoryLimit(MemoryUnit unit, InfoType fetch, long pid)   {
  POOLED_USAGE_AND_LIMITS quota;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &quota) )
    return adjustMemory(unit, quota.PagedPoolLimit);
  return 0;
}

/// Basic Process Information: priority boost
long System::numPageFault(InfoType fetch, long pid)   {
  VM_COUNTERS info;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &info) )
    return info.PageFaultCount;
  return -2;
}

/// Basic Process Information: priority boost
long System::pagefileUsage(MemoryUnit unit, InfoType fetch, long pid)   {
  VM_COUNTERS info;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &info) )
    return adjustMemory(unit, info.PagefileUsage);
  return -2;
}

/// Basic Process Information: priority boost
long System::pagefileUsagePeak(MemoryUnit unit, InfoType fetch, long pid)   {
  VM_COUNTERS info;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &info) )
    return adjustMemory(unit, info.PeakPagefileUsage);
  return -2;
}

/// Basic Process Information: priority boost
long System::pagefileUsageLimit(MemoryUnit unit, InfoType fetch, long pid)   {
  POOLED_USAGE_AND_LIMITS quota;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &quota) )    {
    if ( long(quota.PagefileLimit) < 0 )
      return -1;//LONG_MAX;
    return adjustMemory(unit, quota.PagefileLimit);
  }
  return -2;
}

/// Basic Process Information: priority boost
long System::mappedMemory(MemoryUnit unit, InfoType fetch, long pid)   {
  VM_COUNTERS info;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &info) )
    return adjustMemory(unit, info.WorkingSetSize);
  return -2;
}

/// Basic Process Information: priority boost
long System::mappedMemoryPeak(MemoryUnit unit, InfoType fetch, long pid)   {
  VM_COUNTERS info;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &info) )
    return adjustMemory(unit, info.PeakWorkingSetSize);
  return -2;
}

/// System Process Limits: Minimum amount of virtual memory this process may use
long System::minMemoryLimit(MemoryUnit unit, InfoType fetch, long pid)   {
  QUOTA_LIMITS quota;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &quota) )
    return adjustMemory(unit, quota.MinimumWorkingSetSize);
  return 0;
}

/// System Process Limits: Maximum amount of virtual memory this process is allowed to use
long System::maxMemoryLimit(MemoryUnit unit, InfoType fetch, long pid)   {
  QUOTA_LIMITS quota;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &quota) )
    return adjustMemory(unit, quota.MaximumWorkingSetSize);
  return 0;
}

/// Basic Process Information: priority boost
long System::virtualMemory(MemoryUnit unit, InfoType fetch, long pid)   {
  VM_COUNTERS info;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &info) )
    return adjustMemory(unit, info.VirtualSize);
  return -2;
}

/// Basic Process Information: priority boost
long System::virtualMemoryPeak(MemoryUnit unit, InfoType fetch, long pid)   {
  VM_COUNTERS info;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &info) )
    return adjustMemory(unit, info.PeakVirtualSize);
  return -2;
}

/// System Process Limits: Maximum amount of the page file this process is allowed to use
long System::virtualMemoryLimit(MemoryUnit unit, InfoType fetch, long pid)   {
  QUOTA_LIMITS quota;
  if ( fetch != NoFetch && getProcess()->query(pid, fetch, &quota) )    {
    if ( long(quota.PagefileLimit) == -1 )
      return -1;//LONG_MAX;
    return adjustMemory(unit, quota.PagefileLimit);
  }
  return 0;
}

