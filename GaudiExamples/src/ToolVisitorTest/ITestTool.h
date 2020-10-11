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
#ifndef GAUDITESTING_ITESTTOOL
#define GAUDITESTING_ITESTTOOL 1
#include "GaudiKernel/IAlgTool.h"

namespace GaudiTesting {
  class ITestTool : virtual public IAlgTool {
  public:
    /// InterfaceID
    DeclareInterfaceID( ITestTool, 1, 0 );

    virtual void process() const = 0;
  };
} // namespace GaudiTesting
#endif
