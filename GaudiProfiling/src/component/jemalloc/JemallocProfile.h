/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/Algorithm.h>

/** @class JemallocProfile JemallocProfile.h jemalloc/JemallocProfile.h
 *
 * Algorithm to enable/disable the profiling of the head by Jemalloc.
 * For this to run, you must set the env variables:
 * LD_PRELOAD=<jemalloc lib>
 * MALLOC_CONF=prof:true,prof_leak:true,prof_prefix:<dumpprefix>
 *
 *
 *  @author Ben Couturier
 *  @date   2015-06-09
 */
class JemallocProfile : public Algorithm {
public:
  /// Standard constructor
  using Algorithm::Algorithm;

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution

private:
  Gaudi::Property<int> m_nStartFromEvent{ this, "StartFromEventN", 1, "After what event we start profiling. " };
  Gaudi::Property<int> m_nStopAtEvent{
      this, "StopAtEventN", 0,
      "After what event we stop profiling. If 0 than we also profile finalization stage. Default = 0." };
  Gaudi::Property<int> m_dumpPeriod{ this, "DumpPeriod", 100, "Period for dumping head to a file. Default=100" };

  bool m_profiling   = false; // whether we are profiling...
  int  m_eventNumber = 0;     // Current event number
};
