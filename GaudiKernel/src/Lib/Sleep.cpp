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
#include "GaudiKernel/Sleep.h"
#include <chrono>
#include <thread>

namespace Gaudi {

  void Sleep( int sec ) { std::this_thread::sleep_for( std::chrono::seconds( sec ) ); }

  void NanoSleep( long long nsec ) { std::this_thread::sleep_for( std::chrono::nanoseconds( nsec ) ); }
} // namespace Gaudi
