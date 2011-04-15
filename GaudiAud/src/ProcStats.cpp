// Class: ProcStats
// Purpose:  To keep statistics on memory use
// Warning:  Only Linux implementation at the present time...
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "X" to "Y" may lose significant bits
//   meant
#pragma warning(disable:2259)
#endif

#include "ProcStats.h"

#ifdef __linux
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/syscall.h>
#include <sys/procfs.h>
#include <cstdio>

using std::cerr;
using std::cout;
using std::endl;

/* Format of the Linux proc/stat (man 5 proc, kernel 2.6.35):
 pid %d      The process ID.

 comm %s     The  filename  of the executable, in parentheses.  This is visible
             whether or not the executable is swapped out.

 state %c    One character from the string "RSDZTW" where R is  running,  S  is
             sleeping in an interruptible wait, D is waiting in uninterruptible
             disk sleep, Z is zombie, T is traced or stopped (on a signal), and
             W is paging.

 ppid %d     The PID of the parent.

 pgrp %d     The process group ID of the process.

 session %d  The session ID of the process.

 tty_nr %d   The controlling terminal of the process.  (The minor device number
             is contained in the combination of bits 31 to 20 and 7 to  0;  the
             major device number is in bits 15 t0 8.)

 tpgid %d    The ID of the foreground process group of the controlling terminal
             of the process.

 flags %u (%lu before Linux 2.6.22)
             The kernel flags word of the process.  For bit meanings,  see  the
             PF_*  defines  in  <linux/sched.h>.   Details depend on the kernel
             version.

 minflt %lu  The number of minor faults the process has  made  which  have  not
             required loading a memory page from disk.

 cminflt %lu The  number of minor faults that the process's waited-for children
             have made.

 majflt %lu  The number of  major  faults  the  process  has  made  which  have
             required loading a memory page from disk.

 cmajflt %lu The  number of major faults that the process's waited-for children
             have made.

 utime %lu   Amount of time that this process has been scheduled in user  mode,
             measured  in  clock  ticks  (divide by sysconf(_SC_CLK_TCK).  This
             includes guest time, guest_time (time spent running a virtual CPU,
             see  below),  so that applications that are not aware of the guest
             time field do not lose that time from their calculations.

 stime %lu   Amount of time that this process  has  been  scheduled  in  kernel
             mode, measured in clock ticks (divide by sysconf(_SC_CLK_TCK).

 cutime %ld  Amount  of  time that this process's waited-for children have been
             scheduled in  user  mode,  measured  in  clock  ticks  (divide  by
             sysconf(_SC_CLK_TCK).   (See  also times(2).)  This includes guest
             time, cguest_time (time spent running a virtual CPU, see below).

 cstime %ld  Amount of time that this process's waited-for children  have  been
             scheduled  in  kernel  mode,  measured  in  clock ticks (divide by
             sysconf(_SC_CLK_TCK).

 priority %ld
             (Explanation for Linux 2.6)  For  processes  running  a  real-time
             scheduling  policy (policy below; see sched_setscheduler(2)), this
             is the negated scheduling priority, minus one; that is,  a  number
             in  the  range -2 to -100, corresponding to real-time priorities 1
             to 99.  For processes running  under  a  non-real-time  scheduling
             policy, this is the raw nice value (setpriority(2)) as represented
             in the kernel.  The kernel stores nice values as  numbers  in  the
             range 0 (high) to 39 (low), corresponding to the user-visible nice
             range of -20 to 19.

             Before Linux 2.6, this was a scaled value based on  the  scheduler
             weighting given to this process.

 nice %ld    The  nice value (see setpriority(2)), a value in the range 19 (low
             priority) to -20 (high priority).

 num_threads %ld
             Number of threads in this process (since Linux 2.6).  Before  ker‐
             nel  2.6,  this  field was hard coded to 0 as a placeholder for an
             earlier removed field.

 itrealvalue %ld
             The time in jiffies before the next SIGALRM is sent to the process
             due  to  an interval timer.  Since kernel 2.6.17, this field is no
             longer maintained, and is hard coded as 0.

 starttime %llu (was %lu before Linux 2.6)
             The time in jiffies the process started after system boot.

 vsize %lu   Virtual memory size in bytes.

 rss %ld     Resident Set Size: number of pages the process has in real memory.
             This  is  just  the pages which count towards text, data, or stack
             space.  This does not include pages which have  not  been  demand-
             loaded in, or which are swapped out.

 rsslim %lu  Current  soft  limit  in  bytes on the rss of the process; see the
             description of RLIMIT_RSS in getpriority(2).

 startcode %lu
             The address above which program text can run.

 endcode %lu The address below which program text can run.

 startstack %lu
             The address of the start (i.e., bottom) of the stack.

 kstkesp %lu The current value of ESP (stack pointer), as found in  the  kernel
             stack page for the process.

 kstkeip %lu The current EIP (instruction pointer).

 signal %lu  The  bitmap  of  pending  signals,  displayed as a decimal number.
             Obsolete, because it does not  provide  information  on  real-time
             signals; use /proc/[pid]/status instead.

 blocked %lu The  bitmap  of  blocked  signals,  displayed as a decimal number.
             Obsolete, because it does not  provide  information  on  real-time
             signals; use /proc/[pid]/status instead.

 sigignore %lu
             The  bitmap  of  ignored  signals,  displayed as a decimal number.
             Obsolete, because it does not  provide  information  on  real-time
             signals; use /proc/[pid]/status instead.

 sigcatch %lu
             The  bitmap  of  caught  signals,  displayed  as a decimal number.
             Obsolete, because it does not  provide  information  on  real-time
             signals; use /proc/[pid]/status instead.

 wchan %lu   This  is the "channel" in which the process is waiting.  It is the
             address of a system call, and can be looked up in  a  namelist  if
             you   need   a   textual   name.    (If  you  have  an  up-to-date
             /etc/psdatabase, then try ps -l to see the WCHAN field in action.)

 nswap %lu   Number of pages swapped (not maintained).

 cnswap %lu  Cumulative nswap for child processes (not maintained).

 exit_signal %d (since Linux 2.1.22)
             Signal to be sent to parent when we die.

 processor %d (since Linux 2.2.8)
             CPU number last executed on.

 rt_priority %u (since Linux 2.5.19; was %lu before Linux 2.6.22)
             Real-time scheduling priority, a number in the range 1 to  99  for
             processes  scheduled under a real-time policy, or 0, for non-real-
             time processes (see sched_setscheduler(2)).

 policy %u (since Linux 2.5.19; was %lu before Linux 2.6.22)
             Scheduling policy (see sched_setscheduler(2)).  Decode  using  the
             SCHED_* constants in linux/sched.h.

 delayacct_blkio_ticks %llu (since Linux 2.6.18)
             Aggregated  block  I/O  delays, measured in clock ticks (centisec‐
             onds).

 guest_time %lu (since Linux 2.6.24)
             Guest time of the process (time spent running a virtual CPU for  a
             guest  operating  system),  measured  in  clock  ticks  (divide by
             sysconf(_SC_CLK_TCK).

 cguest_time %ld (since Linux 2.6.24)
             Guest time of the process's  children,  measured  in  clock  ticks
             (divide by sysconf(_SC_CLK_TCK).
*/
struct linux_proc {
  int pid;
  char comm[400];
  char state;
  int ppid;
  int pgrp;
  int session;
  int tty;
  int tpgid;
  unsigned long flags;
  unsigned long minflt;
  unsigned long cminflt;
  unsigned long majflt;
  unsigned long cmajflt;
  unsigned long utime;
  unsigned long stime;
  long cutime;
  long cstime;
  long priority;
  long nice;
  long num_threads;
  long itrealvalue;
  unsigned long long starttime;
  unsigned long vsize;
  long rss;
  unsigned long rlim;
  unsigned long startcode;
  unsigned long endcode;
  unsigned long startstack;
  unsigned long kstkesp;
  unsigned long kstkeip;
  unsigned long signal;
  unsigned long blocked;
  unsigned long sigignore;
  unsigned long sigcatch;
  unsigned long wchan;
};
#endif // __linux

ProcStats::cleanup::~cleanup() {
  if(ProcStats::inst!=0)  {
    delete ProcStats::inst;
    ProcStats::inst=0;
  }
}

ProcStats* ProcStats::instance() {
  static cleanup c;
  if(inst==0)
    inst = new ProcStats;
  return inst;
}

ProcStats* ProcStats::inst = 0;

ProcStats::ProcStats():valid(false)
{
#ifdef __linux
  pg_size = sysconf(_SC_PAGESIZE); // getpagesize();
  std::ostringstream ost;

  ost << "/proc/" << getpid() << "/stat";
  fname = ost.str();
  if((fd=open(fname.c_str(),O_RDONLY))<0)
  {
    cerr << "Failed to open " << ost.str() << endl;
    return;
  }
#endif
  valid=true;
}

ProcStats::~ProcStats()
{
#ifdef __linux
  close(fd);
#endif
}

bool ProcStats::fetch(procInfo& f)
{
  if( valid == false ) return false;

#ifdef __linux
  double pr_size, pr_rssize;
  linux_proc pinfo;
  int cnt;

  lseek(fd,0,SEEK_SET);

  if((cnt=read(fd,buf,sizeof(buf)))<0)
  {
    cout << "LINUX Read of Proc file failed:" << endl;
    return false;
  }

  if(cnt>0)
  {
    buf[cnt]='\0';

    sscanf(buf,
        //1  2  3  4  5  6  7  8  9  10  1   2   3   4   5   6   7   8   9   20  1   2   3   4   5   6   7   8   9   30  1   2   3   4   5
        "%d %s %c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %llu %lu %ld %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
        &pinfo.pid,
        pinfo.comm,
        &pinfo.state,
        &pinfo.ppid,
        &pinfo.pgrp,
        &pinfo.session,
        &pinfo.tty,
        &pinfo.tpgid,
        &pinfo.flags,
        &pinfo.minflt,
        &pinfo.cminflt,
        &pinfo.majflt,
        &pinfo.cmajflt,
        &pinfo.utime,
        &pinfo.stime,
        &pinfo.cutime,
        &pinfo.cstime,
        &pinfo.priority,
        &pinfo.nice,
        &pinfo.num_threads,
        &pinfo.itrealvalue,
        &pinfo.starttime,
        &pinfo.vsize,
        &pinfo.rss,
        &pinfo.rlim,
        &pinfo.startcode,
        &pinfo.endcode,
        &pinfo.startstack,
        &pinfo.kstkesp,
        &pinfo.kstkeip,
        &pinfo.signal,
        &pinfo.blocked,
        &pinfo.sigignore,
        &pinfo.sigcatch,
        &pinfo.wchan
        );

      // resident set size in pages
    pr_size = (double)pinfo.vsize;
    pr_rssize = (double)pinfo.rss;

    f.vsize = pr_size   / (1024*1024);
    f.rss   = pr_rssize * pg_size / (1024*1024);
  }

#else
  f.vsize = 0;
  f.rss   = 0;
#endif

  bool rc = (curr==f)?false:true;

  curr.rss=f.rss;
  curr.vsize=f.vsize;

  return rc;
}

