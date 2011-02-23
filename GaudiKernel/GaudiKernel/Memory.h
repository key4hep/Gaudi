// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/Memory.h,v 1.2 2006/12/10 20:29:17 leggett Exp $
//====================================================================
//	Memory.h
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
#ifndef GAUDIKERNEL_MEMORY_H
#define GAUDIKERNEL_MEMORY_H

// Framework include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/SystemBase.h"


/** Note: OS specific details for memory usage

    Entrypoints:
    - basePriority            Base priority
    - procID                  Process ID
    - parentID                Parent's process ID
    - affinityMask            Affinity mask
    - exitStatus              Exit status
    - priorityBoost           Priority boost
    - numPageFault            Number of page faults
    - pagefileUsage           Current page file usage
    - pagefileUsagePeak       Peak usage of page file
    - nonPagedMemory          Current usage of non paged memory
    - nonPagedMemoryPeak      Peak usage of non paged memory
    - nonPagedMemoryLimit     Maximum amount of non-paged memory this process is allowed to use
    - pagedMemory             Amount of paged memory currently occupied
    - pagedMemoryPeak         Maximum of paged memory occupied by the process
    - pagedMemoryLimit        paged memory that can be occupied by the process
    - memorySize              Size of currently occupied memory
    - memorySizePeak          Maximum memory occupied by the process
    - memoryLimit             Amount of memory that can be occupied by the process
    - virtualMemorySize       Virtual memory size currenly occupied
    - peakVirtualMemorySize   Maximum amount of virtual memory occupied
    - virtualMemoryLimit      Amount of virtual memory that can be occupied by the process
    - minMemoryLimit          Minimum amount of virtual memory this process may use
    - maxMemoryLimit          Maximum amount of virtual memory this process is allowed to use
    - mappedMemorySize        Currenly mapped virtual memory size
    - mappedMemorySizePeak    Peak mapped virtual memory size

    @author:  M.Frank
    @version: 1.0
*/
namespace System    {
  /// Unit of memory
  enum MemoryUnit { Byte, kByte, MByte, GByte, TByte, PByte, EByte };
  /// Convert time from kByte to requested representation (Experts only)
  GAUDI_API long adjustMemory(MemoryUnit typ, long value);
  /// Basic Process Information: Process ID
  GAUDI_API long procID();
  /// Basic Process Information: Parent's process ID
  GAUDI_API long parentID(InfoType fetch = ProcessBasics, long pid = -1);
  /// Basic Process Information: Affinity mask
  GAUDI_API long affinityMask(InfoType fetch = ProcessBasics, long pid = -1);
  /// Basic Process Information: Exit status (does not really make sense for the running process, but for others!)
  GAUDI_API long exitStatus(InfoType fetch = ProcessBasics, long pid = -1);
  /// Basic Process Information: Base priority
  GAUDI_API long basePriority(InfoType fetch = ProcessBasics, long pid = -1);
  /// Basic Process Information: priority boost
  GAUDI_API long priorityBoost(InfoType fetch = PriorityBoost, long pid = -1);

  /// Basic Process Information: Number of page faults
  GAUDI_API long numPageFault(InfoType fetch = Memory, long pid = -1);
  /// Basic Process Information: Current page file usage
  GAUDI_API long pagefileUsage(MemoryUnit unit = kByte, InfoType fetch = Memory, long pid = -1);
  /// Basic Process Information: Peak usage of page file
  GAUDI_API long pagefileUsagePeak(MemoryUnit unit = kByte, InfoType fetch = Memory, long pid = -1);
  /// Basic Process Information: Peak usage of page file
  GAUDI_API long pagefileUsageLimit(MemoryUnit unit = kByte, InfoType fetch = Quota, long pid = -1);

  /// Basic Process Information: Current usage of non paged memory
  GAUDI_API long nonPagedMemory( MemoryUnit unit = kByte, InfoType fetch = Memory, long pid = -1 );
  /// Basic Process Information: Peak usage of non paged memory
  GAUDI_API long nonPagedMemoryPeak( MemoryUnit unit = kByte, InfoType fetch = Memory, long pid = -1 );
  /// System Process Limits: Maximum amount of non-paged memory this process is allowed to use
  GAUDI_API long nonPagedMemoryLimit( MemoryUnit unit = kByte, InfoType fetch = Quota, long pid = -1 );

  /// Basic Process Information: Amount of paged memory currently occupied by the process 'pid'
  GAUDI_API long pagedMemory( MemoryUnit unit = kByte, InfoType fetch = Memory, long pid = -1 );
  /// Basic Process Information: Maximum of paged memory occupied by the process 'pid'
  GAUDI_API long pagedMemoryPeak( MemoryUnit unit = kByte, InfoType fetch = Memory, long pid = -1 );
  /// Basic Process Information: Amount of paged memory that can be occupied by the process 'pid'
  GAUDI_API long pagedMemoryLimit( MemoryUnit unit = kByte, InfoType fetch = Quota, long pid = -1 );

  /// System Process Limits: Minimum amount of virtual memory this process may use
  GAUDI_API long minMemoryLimit( MemoryUnit unit = kByte, InfoType fetch = Quota, long pid = -1 );
  /// System Process Limits: Maximum amount of virtual memory this process is allowed to use
  GAUDI_API long maxMemoryLimit( MemoryUnit unit = kByte, InfoType fetch = Quota, long pid = -1 );
  /// Basic Process Information: priority boost
  GAUDI_API long mappedMemory( MemoryUnit unit = kByte, InfoType fetch = Memory, long pid = -1 );
  /// Basic Process Information: priority boost
  GAUDI_API long mappedMemoryPeak( MemoryUnit unit = kByte, InfoType fetch = Memory, long pid = -1 );
  /// Basic Process Information: priority boost
  GAUDI_API long virtualMemory( MemoryUnit unit = kByte, InfoType fetch = Memory, long pid = -1 );
  /// Basic Process Information: priority boost
  GAUDI_API long virtualMemoryPeak( MemoryUnit unit = kByte, InfoType fetch = Memory, long pid = -1 );
  /// System Process Limits: Maximum amount of the page file this process is allowed to use
  GAUDI_API long virtualMemoryLimit( MemoryUnit unit = kByte, InfoType fetch = Quota, long pid = -1 );
}
#endif //GAUDIKERNEL_MEMORY_H
