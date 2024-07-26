/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/StateMachine.h>

namespace Gaudi {
  /**
   *  Interface defining a CallBack registration functionality based on the
   *  State Machine of Gaudi. Callbacks are std::function which can be registered
   *  for each transition. They will then be called automatically just after
   *  the transition took place in the order in which they have been registered.
   */
  struct IFSMCallbackHolder : virtual public IInterface {

  public:
    DeclareInterfaceID( IFSMCallbackHolder, 1, 0 );

    using CallBack                                                      = std::function<void()>;
    virtual void registerCallBack( StateMachine::Transition, CallBack ) = 0;
  };
} // namespace Gaudi
