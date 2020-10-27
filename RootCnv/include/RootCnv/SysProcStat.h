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
/*
 * SysProcStat.h
 *
 *  Created on: Jul 6, 2012
 *      Author: Ivan Valencik
 */

#ifndef SYSPROCSTAT_H_
#define SYSPROCSTAT_H_

namespace Gaudi {
  class SysProcStat {

  public:
    int           pid, ppid, pgrp, session, tty_nr, tpgid, exit_signal, processor;
    long unsigned flags, minflt, cminflt, majflt, cmajflt, utime, stime, startime, vsize, rlim, startcode, endcode,
        startstack, kstkesp, kstkeip, signal, blocked, sigingore, sigcatch, wchan, nswap, cnswap;
    long int cutime, cstime, priority, nice, num_threads, itrealvalue, rss;
    char     comm[128], state;

    long time;

    SysProcStat();
    virtual ~SysProcStat() = default;

    // updates the values by calling /proc/self/stat
    virtual int update();
  };
} // namespace Gaudi
#endif /* SYSPROCSTAT_H_ */
