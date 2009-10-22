// $Id: ProcStats.h,v 1.2 2004/06/08 13:40:07 mato Exp $
#ifndef GAUDIAUD_PROCSTATS_H
#define GAUDIAUD_PROCSTATS_H

// Class: ProcStats
// Description:  Keeps statistics on memory usage
// Author: Jim Kowalkowski (FNAL), modified by M. Shapiro (LBNL)

#include <string>
#include <vector>

struct procInfo
{
  procInfo() : vsize(0), rss(0) {}
  procInfo(double sz, double rss_sz): vsize(sz),rss(rss_sz) {}

  bool operator==(const procInfo& p) const
    { return vsize==p.vsize && rss==p.rss; }

  // see proc(4) man pages for units and a description
  double vsize;  // in MB (used to be in pages?)
  double rss;    // in MB (used to be in pages?)
};

class ProcStats
{
public:
  static ProcStats* instance();

  bool fetch(procInfo& fill_me);
  double pageSize() const { return pg_size; }

private:
  ProcStats();
  ~ProcStats();

  struct cleanup
  {
    cleanup() { }
    ~cleanup();
  };

  friend struct cleanup;

  int fd;
  double pg_size;
  procInfo curr;
  std::string fname;
  char buf[500];
  bool valid;

  static ProcStats* inst;
};

#endif

