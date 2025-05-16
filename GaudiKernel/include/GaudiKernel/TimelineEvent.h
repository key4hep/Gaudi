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

#include <chrono>
#include <pthread.h>
#include <string>

struct TimelineEvent final {
  using Clock      = std::chrono::high_resolution_clock;
  using time_point = Clock::time_point;

  pthread_t thread;
  size_t    slot;
  size_t    event;

  std::string algorithm;

  time_point start;
  time_point end;
};
