// $Id: ProcStats.cpp,v 1.4 2006/09/13 14:31:12 hmd Exp $
// Class: ProcStats
// Purpose:  To keep statistics on memory use
// Warning:  Only Linux implementation at the present time...

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
  unsigned int startcode; // %u
  unsigned int endcode; // %u
  unsigned int startstack; // %u
  unsigned int kstkesp; // %u
  unsigned int kstkeip; // %u
  int signal; // %d
  int blocked; // %d
  int sigignore; // %d
  int sigcatch; // %d
  unsigned int wchan; // %u
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
      "%d %s %c %d %d %d %d %d %u %u %u %u %u %d %d %d %d %d %d %u %u %d %u %u %u %u %u %u %u %u %d %d %d %d %u",
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
      &pinfo.startcode, // %u
      &pinfo.endcode, // %u
      &pinfo.startstack, // %u
      &pinfo.kstkesp, // %u
      &pinfo.kstkeip, // %u
      &pinfo.signal, // %d
      &pinfo.blocked, // %d
      &pinfo.sigignore, // %d
      &pinfo.sigcatch, // %d
      &pinfo.wchan // %u
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

