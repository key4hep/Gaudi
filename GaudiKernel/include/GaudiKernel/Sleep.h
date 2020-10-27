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
#ifndef GAUDIKERNEL_SLEEP_H_
#define GAUDIKERNEL_SLEEP_H_

#include "GaudiKernel/Kernel.h"

namespace Gaudi {

  /// Simple sleep function.
  /// @author Marco Clemencic
  GAUDI_API void Sleep( int sec );

  /// Small variation on the sleep function for nanoseconds sleep.
  /// @author Marco Clemencic
  GAUDI_API void NanoSleep( long long nsec );
} // namespace Gaudi

#endif /*GAUDIKERNEL_SLEEP_H_*/
