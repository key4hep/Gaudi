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
#pragma once

// Framework include files
#include <GaudiKernel/Kernel.h>
#include <GaudiKernel/SystemBase.h>

namespace System {
  // Forward declarations
  class ProcessDescriptor;
  /// Retrieve Process structure
  ProcessDescriptor* getProcess();

  /** Basic Process Information
      NtQueryInformationProcess using ProcessBasicInfo
  */
  typedef struct _PEB* PPEB;
  struct PROCESS_BASIC_INFORMATION {
    long          ExitStatus;
    PPEB          PebBaseAddress;
    unsigned long AffinityMask;
    long          BasePriority;
    unsigned long UniqueProcessId;
    unsigned long InheritedFromUniqueProcessId;
  };

  /** Process Quotas
      NtQueryInformationProcess using ProcessQuotaLimits
      NtQueryInformationProcess using ProcessPooledQuotaLimits
      NtSetInformationProcess using ProcessQuotaLimits
  */
  struct QUOTA_LIMITS {
    unsigned long PagedPoolLimit;
    unsigned long NonPagedPoolLimit;
    unsigned long MinimumWorkingSetSize;
    unsigned long MaximumWorkingSetSize;
    unsigned long PagefileLimit;
    long long     TimeLimit;
  };

  /** Process I/O Counters
      NtQueryInformationProcess using ProcessIoCounters
  */
  struct IO_COUNTERS {
    unsigned long ReadOperationCount;
    unsigned long WriteOperationCount;
    unsigned long OtherOperationCount;
    //      long long ReadOperationCount;
    //      long long WriteOperationCount;
    //      long long OtherOperationCount;
    long long ReadTransferCount;
    long long WriteTransferCount;
    long long OtherTransferCount;
  };

  /** Process Virtual Memory Counters
      NtQueryInformationProcess using ProcessVmCounters
  */
  struct VM_COUNTERS {
    unsigned long PeakVirtualSize;
    unsigned long VirtualSize;
    unsigned long PageFaultCount;
    unsigned long PeakWorkingSetSize;
    unsigned long WorkingSetSize;
    unsigned long QuotaPeakPagedPoolUsage;
    unsigned long QuotaPagedPoolUsage;
    unsigned long QuotaPeakNonPagedPoolUsage;
    unsigned long QuotaNonPagedPoolUsage;
    unsigned long PagefileUsage;
    unsigned long PeakPagefileUsage;
  };

  /** Process Pooled Quota Usage and Limits
      NtQueryInformationProcess using ProcessPooledUsageAndLimits
  */
  struct POOLED_USAGE_AND_LIMITS {
    unsigned long PeakPagedPoolUsage;
    unsigned long PagedPoolUsage;
    unsigned long PagedPoolLimit;
    unsigned long PeakNonPagedPoolUsage;
    unsigned long NonPagedPoolUsage;
    unsigned long NonPagedPoolLimit;
    unsigned long PeakPagefileUsage;
    unsigned long PagefileUsage;
    unsigned long PagefileLimit;
  };

  /** Process/Thread System and User Time
      NtQueryInformationProcess using ProcessTimes
      NtQueryInformationThread using ThreadTimes
  */
  struct KERNEL_USER_TIMES {
    long long CreateTime;
    long long ExitTime;
    long long KernelTime;
    long long UserTime;
    // long long EllapsedTime; // Added by M.Frank
  };

  /**
   * @class ProcessDescriptor ProcessDescriptor.h
   *
   * Provides access to process information
   *
   * @author M.Frank
   * @author Sebastien Ponce
   */
  class ProcessDescriptor {
    class ProcessHandle {
      void* m_handle;
      bool  m_needRelease;

    public:
      ProcessHandle( long pid );
      virtual ~ProcessHandle();
      long  item() { return m_needRelease ? 1 : 0; }
      void* handle() { return m_handle; }
    };

  public:
    ProcessDescriptor()          = default;
    virtual ~ProcessDescriptor() = default;
    long query( long pid, InfoType fetch, PROCESS_BASIC_INFORMATION* info );
    long query( long pid, InfoType fetch, POOLED_USAGE_AND_LIMITS* info );
    long query( long pid, InfoType fetch, KERNEL_USER_TIMES* info );
    long query( long pid, InfoType fetch, QUOTA_LIMITS* info );
    long query( long pid, InfoType fetch, VM_COUNTERS* info );
    long query( long pid, InfoType fetch, IO_COUNTERS* info );
    long query( long pid, InfoType fetch, long* info );
  };

  inline ProcessDescriptor* getProcess() {
    static ProcessDescriptor p;
    return &p;
  }
} // namespace System
