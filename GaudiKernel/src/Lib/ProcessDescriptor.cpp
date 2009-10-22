// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/ProcessDescriptor.cpp,v 1.16 2007/11/21 13:04:53 marcocle Exp $
//====================================================================
//	ProcStat.cpp
//--------------------------------------------------------------------
//
//	Package    : System (The LHCb System service)
//
//  Description: Invoke interactively the ProcStat from a 
//               running application
//
//	Author     : M.Frank
//      Created    : 13/11/00
//	Changes    : 
//
//====================================================================
#define GAUDIKERNEL_PROCSTAT_CPP

static const long TICK_TO_100NSEC = 100000;

namespace System    {
  enum ProcessInfoCommand {
      ProcessBasicInformation,
      ProcessQuotaLimits,
      ProcessIoCounters,
      ProcessVmCounters,
      ProcessTimes,
      ProcessBasePriority,
      ProcessRaisePriority,
      ProcessDebugPort,
      ProcessExceptionPort,
      ProcessAccessToken,
      ProcessLdtInformation,
      ProcessLdtSize,
      ProcessDefaultHardErrorMode,
      ProcessIoPortHandlers,          // Note: this is kernel mode only
      ProcessPooledUsageAndLimits,
      ProcessWorkingSetWatch,
      ProcessUserModeIOPL,
      ProcessEnableAlignmentFaultFixup,
      ProcessPriorityClass,
      ProcessWx86Information,
      ProcessHandleCount,
      ProcessAffinityMask,
      ProcessPriorityBoost,
      MaxProcessInfoClass,  //
      ProcessEllapsedTime
  };
}
#ifdef _WIN32
  #  define strcasecmp _stricmp
  #  define strncasecmp _strnicmp
  #define NOMSG
  #define NOGDI
  #include "process.h"
  #include "windows.h"
  #define getpid _getpid
namespace NtApi {
//__declspec(dllimport) long __stdcall NtQueryInformationProcess(
//  typedef __declspec(dllimport) long __stdcall (*__NtQueryInformationProcess)(
//  extern "C" long __cdecl NtQueryInformationProcess(

    typedef long (WINAPI *__NtQueryInformationProcess)(
  
    void* ProcessHandle,
    long ProcessInformationClass,
    void* ProcessInformation,
    unsigned long ProcessInformationLength,
    unsigned long* ReturnLength
    );
  __NtQueryInformationProcess NtQueryInformationProcess;
};
#else  // UNIX...: first the EGCS stuff, then the OS dependent includes
#define WINVER 0
#include <errno.h>
#include <string>
#include "unistd.h"
#include "libgen.h"
#include <cstdio>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <fcntl.h>
#include "sys/times.h"
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/syscall.h>
#ifndef __APPLE__
#include <sys/procfs.h>
#endif
#include <sys/time.h>
#include <sys/resource.h>
#include <cstdio>

struct linux_proc {
	int pid; // %d
	char comm[400]; // %s
	char state; // %c
	int ppid; // %d
	int pgrp; // %d
	int session; // %d
	int tty; // %d
	int tpgid; // %d
	unsigned int flags; // %u
	unsigned int minflt; // %u
	unsigned int cminflt; // %u
	unsigned int majflt; // %u
	unsigned int cmajflt; // %u
	int utime; // %d
	int stime; // %d
	int cutime; // %d
	int cstime; // %d
	int counter; // %d
	int priority; // %d
	unsigned int timeout; // %u
	unsigned int itrealvalue; // %u
	int starttime; // %d
	unsigned int vsize; // %u
	unsigned int rss; // %u
	unsigned int rlim; // %u
	unsigned long startcode; // %u
	unsigned long endcode; // %u
	unsigned int startstack; // %u
	unsigned int kstkesp; // %u
	unsigned int kstkeip; // %u
	int signal; // %d
	int blocked; // %d
	int sigignore; // %d
	int sigcatch; // %d
	unsigned int wchan; // %u
};

#ifdef __APPLE__
// static long  pg_size = 0;
#else
static long  pg_size = sysconf(_SC_PAGESIZE); // getpagesize();
#endif
void readProcStat(long pid, linux_proc& pinfo) {

  int cnt, fd;
  char buf[512];

  std::ostringstream ost; 

  ost << "/proc/" << pid << "/stat";
  std::string fname = ost.str();
  if((fd=open(fname.c_str(),O_RDONLY))<0)  {
    std::cerr << "Failed to open " << ost.str() << std::endl;
    return;
  }

  lseek(fd,0,SEEK_SET);
  if((cnt=read(fd,buf,sizeof(buf)))<0)	{
    std::cout << "LINUX Read of Proc file failed:" << std::endl;
    return;
  }
  
  if(cnt>0)	{
    buf[cnt]='\0';
    sscanf(buf,
           "%d %s %c %d %d %d %d %d %u %u %u %u %u %d %d %d %d %d %d %u %u %d %u %u %u %lu %lu %u %u %u %d %d %d %d %u",
	   &pinfo.pid, // %d
	   pinfo.comm, // %s
	   &pinfo.state, // %c
	   &pinfo.ppid, // %d
	   &pinfo.pgrp, // %d
	   &pinfo.session, // %d
	   &pinfo.tty, // %d
	   &pinfo.tpgid, // %d
	   &pinfo.flags, // %u
	   &pinfo.minflt, // %u
	   &pinfo.cminflt, // %u
	   &pinfo.majflt, // %u
	   &pinfo.cmajflt, // %u
	   &pinfo.utime, // %d
	   &pinfo.stime, // %d
	   &pinfo.cutime, // %d
	   &pinfo.cstime, // %d
	   &pinfo.counter, // %d
	   &pinfo.priority, // %d
	   &pinfo.timeout, // %u
	   &pinfo.itrealvalue, // %u
	   &pinfo.starttime, // %d
	   &pinfo.vsize, // %u
	   &pinfo.rss, // %u
	   &pinfo.rlim, // %u
	   &pinfo.startcode, // %l
	   &pinfo.endcode, // %l
	   &pinfo.startstack, // %u
	   &pinfo.kstkesp, // %u
	   &pinfo.kstkeip, // %u
	   &pinfo.signal, // %d
	   &pinfo.blocked, // %d
	   &pinfo.sigignore, // %d
	   &pinfo.sigcatch, // %d
	   &pinfo.wchan // %u
	   );
  }
  close(fd);
}
#endif

static long s_myPid  = ::getpid();
static inline long processID(long pid) {
  long thePid = (pid>0) ? pid : s_myPid;
  return thePid;
}

// Framework include files
#include "ProcessDescriptor.h"
#include "GaudiKernel/ModuleInfo.h"
#include "GaudiKernel/System.h"

System::ProcessDescriptor* System::getProcess()   {
  static ProcessDescriptor p;
#ifdef _WIN32
  static bool first = true;
  if ( first )    {
    first = false;
    void* mh = ::LoadLibrary("NTDll.dll");
    if ( mh )  {
      NtApi::NtQueryInformationProcess = (NtApi::__NtQueryInformationProcess)
        ::GetProcAddress((HINSTANCE)mh, "NtQueryInformationProcess");
    }
  }
#endif
  return &p;
}

System::ProcessDescriptor::ProcessHandle::ProcessHandle(long pid)   {
  if ( pid > 0 )    {
    if ( pid != s_myPid )    {
#ifdef _WIN32
      m_handle = ::OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,
                               pid);
#else
      m_handle = (void*)s_myPid;
#endif
      m_needRelease = true;
      return;
    }
  }
  m_handle = processHandle();
  m_needRelease = false;
}

System::ProcessDescriptor::ProcessHandle::~ProcessHandle()   {
  if ( m_needRelease )    {
#ifdef _WIN32
    ::CloseHandle(m_handle);
#else
      m_handle = 0;
#endif
  }
}

System::ProcessDescriptor::ProcessDescriptor()
{
}

System::ProcessDescriptor::~ProcessDescriptor()   {
}

long System::ProcessDescriptor::query(long pid,
                                      InfoType fetch,
                                      IO_COUNTERS* info) {
  long status = 1;
  ProcessHandle h(pid);
  IO_COUNTERS* vb = &m_IO_COUNTERS[h.item()];
  if ( fetch == IO )   {
#if defined(_WIN32) && WINVER>=0x0400     // Windows NT
    status = NtApi::NtQueryInformationProcess(h.handle(),
                                              ProcessIoCounters,
                                              vb,
                                              sizeof(IO_COUNTERS),
                                              0);
    status = (status==0) ? 1 : 0;
#elif _WIN32                              // Windows 95,98...
#elif defined(linux)
    linux_proc prc;
    readProcStat(processID(pid), prc);
    rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    vb->ReadOperationCount    = usage.ru_inblock;
    vb->WriteOperationCount   = usage.ru_oublock;
    vb->OtherOperationCount   = 0;
    vb->ReadTransferCount     = usage.ru_inblock;
    vb->WriteTransferCount    = usage.ru_oublock;
    vb->OtherTransferCount    = 0;
#else                                     // All Other
#endif                                    // End ALL OS
  }
  if ( info ) *info = *vb;
  return status;
}

long System::ProcessDescriptor::query(long pid,
                                      InfoType fetch,
                                      POOLED_USAGE_AND_LIMITS* info)    {
  long status = 1;
  ProcessHandle h(pid);
  POOLED_USAGE_AND_LIMITS* vb = &m_POOLED_USAGE_AND_LIMITS[h.item()];
  if ( fetch == Quota )   {
#if defined(_WIN32) && WINVER>=0x0400     // Windows NT
    status = NtApi::NtQueryInformationProcess(h.handle(),
                                              ProcessPooledUsageAndLimits,
                                              vb,
                                              sizeof(POOLED_USAGE_AND_LIMITS),
                                              0);
    status = (status==0) ? 1 : 0;
#elif _WIN32                              // Windows 95,98...
#elif defined(linux)                      // Linux
    //rusage usage;
    //getrusage(RUSAGE_SELF, &usage);
    rlimit lim;

    getrlimit(RLIMIT_DATA, &lim);
    if ( lim.rlim_max == RLIM_INFINITY ) lim.rlim_max = 0xFFFFFFFF;
    vb->PeakPagedPoolUsage = lim.rlim_cur;
    vb->PagedPoolUsage     = lim.rlim_cur;
    vb->PagedPoolLimit     = lim.rlim_max;

    getrlimit(RLIMIT_STACK, &lim);
    if ( lim.rlim_max == RLIM_INFINITY ) lim.rlim_max = 0xFFFFFFFF;
    vb->PeakNonPagedPoolUsage = lim.rlim_cur;
    vb->NonPagedPoolUsage     = lim.rlim_cur;
    vb->NonPagedPoolLimit     = lim.rlim_max;

    linux_proc prc;
    readProcStat(processID(pid), prc);
    vb->PeakPagefileUsage     = prc.rss * pg_size;
    vb->PagefileUsage         = prc.rss * pg_size;
    vb->PagefileLimit         = 0xFFFFFFFF;
#elif defined(__APPLE__)
#else                                     // All Other
#endif                                    // End ALL OS
  }
  if ( info ) *info = *vb;
  return status;
}

long System::ProcessDescriptor::query(long pid, InfoType fetch, long* info)    {
  long status = 1, *vb = &status;
  ProcessHandle h(pid);
  vb = &m_PRIORITYBOOST[h.item()];
  *vb = 0;
  switch ( fetch )    {
  case PriorityBoost:
#if defined(_WIN32) && WINVER>=0x0400     // Windows NT
    status = NtApi::NtQueryInformationProcess(h.handle(),
                                              ProcessPriorityBoost,
                                              vb,
                                              sizeof(long),
                                              0);
#elif _WIN32                              // Windows 95,98...
#else
    // Not applicable
    status = 0;
    *vb = 0;
#endif                                    // End ALL OS
    status = (status==0) ? 1 : 0;
    break;
  default:
    status = -1;
    vb = &status;
    break;
  }
  if ( info ) *info = *vb;
  return status;
}

long System::ProcessDescriptor::query(long pid,
                                      InfoType fetch,
                                      VM_COUNTERS* info)    {
  long status = 1;
  ProcessHandle h(pid);
  VM_COUNTERS* vb = &m_VM_COUNTERS[h.item()];
  if ( fetch == Memory )   {
#if defined(_WIN32) && WINVER>=0x0400     // Windows NT
    status = NtApi::NtQueryInformationProcess(h.handle(),
                                              ProcessVmCounters,
                                              vb,
                                              sizeof(VM_COUNTERS),
                                              0);
    status = (status==0) ? 1 : 0;
#elif _WIN32                              // Windows 95,98...
#elif defined(linux)                      // Linux
    char buf[1024];
    sprintf(buf,"/proc/%ld/statm", processID(pid));
    long size, resident, share, trs, lrs, drs, dt;
    int fd = open(buf,O_RDONLY);
    read(fd, buf, sizeof(buf));  
    close(fd);
    fd = sscanf(buf, "%ld %ld %ld %ld %ld %ld %ld",
                     &size, &resident, &share, &trs, &drs, &lrs, &dt);
    linux_proc prc;
    readProcStat( processID(pid), prc);
    vb->PeakVirtualSize            = prc.vsize;
    vb->VirtualSize                = prc.vsize;
    vb->PeakWorkingSetSize         = resident * pg_size;
    vb->WorkingSetSize             = resident * pg_size;
    vb->QuotaPeakPagedPoolUsage    = share    * pg_size;
    vb->QuotaPagedPoolUsage        = share    * pg_size;
    vb->QuotaNonPagedPoolUsage     = (trs+drs)* pg_size;// drs = data/stack size
    vb->QuotaPeakNonPagedPoolUsage = (trs+drs)* pg_size;// trs = VmExe size
    vb->PageFaultCount             = prc.majflt + prc.minflt;
    vb->PagefileUsage              = prc.vsize-resident*pg_size;
    vb->PeakPagefileUsage          = prc.vsize-resident*pg_size;
#elif defined(__APPLE__)
#else                                     // All Other
#endif                                    // End ALL OS
  }
  if ( info ) *info = *vb;
  return status;
}

long System::ProcessDescriptor::query(long pid,
                                      InfoType fetch,
                                      QUOTA_LIMITS* info)    {
  long status = 1;
  ProcessHandle h(pid);
  QUOTA_LIMITS* vb = &m_QUOTA_LIMITS[h.item()];
  if ( fetch == Quota )   {
#if defined(_WIN32) && WINVER>=0x0400     // Windows NT
    status = NtApi::NtQueryInformationProcess(h.handle(),
                                              ProcessQuotaLimits,
                                              vb,
                                              sizeof(QUOTA_LIMITS),
                                              0);
    status = (status==0) ? 1 : 0;
#elif _WIN32                              // Windows 95,98...
#elif defined(linux)                      // Linux
    // On linux all this stuff typically is not set
    // (ie. rlim_max=RLIM_INFINITY...)
    rlimit lim;
    getrlimit(RLIMIT_DATA, &lim);
    if ( lim.rlim_max == RLIM_INFINITY ) lim.rlim_max = 0xFFFFFFFF;
    vb->PagedPoolLimit        = lim.rlim_max;

    getrlimit(RLIMIT_STACK, &lim);
    if ( lim.rlim_max == RLIM_INFINITY ) lim.rlim_max = 0xFFFFFFFF;
    vb->NonPagedPoolLimit     = lim.rlim_max;
    vb->MinimumWorkingSetSize = 0;

    getrlimit(RLIMIT_RSS, &lim);
    if ( lim.rlim_max == RLIM_INFINITY ) lim.rlim_max = 0xFFFFFFFF;
    vb->MaximumWorkingSetSize = lim.rlim_max;

    getrlimit(RLIMIT_AS, &lim);
    if ( lim.rlim_max == RLIM_INFINITY ) lim.rlim_max = 0xFFFFFFFF;
    vb->PagefileLimit         = lim.rlim_max;

    getrlimit(RLIMIT_CPU, &lim);
    if ( lim.rlim_max == RLIM_INFINITY ) lim.rlim_max = 0xFFFFFFFF;
    vb->TimeLimit             = lim.rlim_max;
#elif defined(__APPLE__)
#else                                     // All Other
#endif                                    // End ALL OS
  }
  if ( info ) *info = *vb;
  return status;
}

long System::ProcessDescriptor::query(long pid,
                                      InfoType fetch,
                                      PROCESS_BASIC_INFORMATION* info)    {
  long status = 1;
  ProcessHandle h(pid);
  PROCESS_BASIC_INFORMATION* vb = &m_PROCESS_BASIC_INFORMATION[h.item()];
  if ( fetch == ProcessBasics )   {
#if defined(_WIN32) && WINVER>=0x0400     // Windows NT
    status = NtApi::NtQueryInformationProcess(h.handle(),
                                              ProcessBasicInformation,
                                              vb,
                                              sizeof(PROCESS_BASIC_INFORMATION),
                                              0);
    status = (status==0) ? 1 : 0;
#elif _WIN32                              // Windows 95,98...
#elif defined(linux)                      // Linux
    linux_proc prc;
    readProcStat( processID(pid), prc);
    vb->ExitStatus                   = 0;
    vb->PebBaseAddress               = (PPEB)prc.startcode;
    vb->BasePriority                 = 2*15-prc.priority;
    // std::cout << "Base Priority=" << vb->BasePriority << "|"
    // << prc.priority << std::endl;
    vb->AffinityMask                 = prc.flags;
    // std::cout << "Flags        =" << vb->AffinityMask << "|"
    // << prc.flags << std::endl;
    vb->UniqueProcessId              = processID(pid);
    vb->InheritedFromUniqueProcessId = prc.ppid;
#else                                     // All Other
#endif                                    // End ALL OS
  }
  if ( info ) *info = *vb;
  return status;
}

long System::ProcessDescriptor::query(long pid,
                                      InfoType fetch,
                                      KERNEL_USER_TIMES* info)   {
  long status = 1;
  ProcessHandle h(pid);
  KERNEL_USER_TIMES* tb = &m_KERNEL_USER_TIMES[h.item()];;
  if ( fetch == Times )   {
#if defined(_WIN32) && WINVER>=0x0400     // Windows NT
    status = NtApi::NtQueryInformationProcess(h.handle(),
                                              ProcessTimes,
                                              tb,
                                              sizeof(KERNEL_USER_TIMES),
                                              0);
    status = (status==0) ? 1 : 0;
#elif defined(_WIN32)                     // Windows 95,98...
#elif defined(linux)                      // Linux
    // prc.startup is in ticks since system start :
    // need to offset for absolute time
    tms tmsb;
    static longlong prc_start = 0;
    //    static longlong offset = 100*longlong(time(0)) - longlong(times(0));
    static longlong offset = 100*longlong(time(0)) - longlong(times(&tmsb));
    if ( processID(pid) == s_myPid && prc_start == 0 ) {
      linux_proc prc;
      readProcStat( processID(pid), prc);
      prc_start = prc.starttime+offset;
    }

    if ( processID(pid) == s_myPid ) {
      struct rusage r;
      getrusage( RUSAGE_SELF, &r );
      const double utime = static_cast<double>(r.ru_utime.tv_sec )*1e6 +
	                   static_cast<double>(r.ru_utime.tv_usec);
      const double stime = static_cast<double>(r.ru_stime.tv_sec )*1e6 +
			   static_cast<double>(r.ru_stime.tv_usec);
      tb->UserTime   = static_cast<longlong>( utime * 10. );
      tb->KernelTime = static_cast<longlong>( stime * 10. );
      tb->CreateTime = prc_start;
    }
    else {
      linux_proc prc;
      readProcStat( processID(pid), prc );

      tms t;
      times(&t);
      tb->UserTime   = t.tms_utime * TICK_TO_100NSEC;
      tb->KernelTime = t.tms_stime * TICK_TO_100NSEC;
      tb->CreateTime = (prc.starttime+offset);
    }
    tb->CreateTime *= TICK_TO_100NSEC;
    tb->ExitTime    = 0;

    status = 1;

#elif defined(__APPLE__)
    // FIXME (MCl): Make an alternative function get timing on OSX
    // times() seems to cause a segmentation fault 
#else  // no /proc file system: assume sys_start for the first call
    tms tmsb;
    static clock_t sys_start = times(0);
    static longlong offset = 100*longlong(time(0)) - sys_start;
    clock_t now = times(&tmsb);
    tb->CreateTime    = offset + now;
    tb->UserTime      = tmsb.tms_utime;
    tb->KernelTime    = tmsb.tms_stime;
    tb->CreateTime   *= TICK_TO_100NSEC;
    tb->UserTime     *= TICK_TO_100NSEC;
    tb->KernelTime   *= TICK_TO_100NSEC;
    tb->ExitTime      = 0;
    status = 1;
#endif
  }
  if ( info ) *info = *tb;
  return status;
}
