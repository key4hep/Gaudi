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
#pragma once

#include <Gaudi/Interfaces/IFSMCallbackHolder.h>
#include <GaudiKernel/StateMachine.h>
#include <GaudiKernel/extends.h>
#include <algorithm>
#include <map>
#include <vector>

namespace Gaudi {

  /** Helper class to implement the IFSMCallbackHolder interface.
   *
   *  FSMCallbackHolder is used by components base classes (Algorithm, Tool,
   *  etc.) to provide a default implementation the IFSMCallbackHolder interface.
   *
   *  When needing to implement the IFSMCallbackHolder interface in a class, it is
   *  enough to wrap the base of the class with FSMCallbackHolder, as in
   *
   *  \code{.cpp}
   *  class MyClass : public FSMCallbackHolder<BaseClass> {
   *    // ...
   *  };
   *  \endcode
   */
  template <class BASE>
  class FSMCallbackHolder : public extends<BASE, IFSMCallbackHolder> {

  public:
    using Parent = extends<BASE, IFSMCallbackHolder>;
    using extends<BASE, IFSMCallbackHolder>::extends;

    void registerCallBack( StateMachine::Transition s, IFSMCallbackHolder::CallBack c ) override {
      m_callbacks[s].push_back( std::move( c ) );
    }

    StatusCode sysInitialize() override {
      return Parent::sysInitialize().andThen( [&]() { handleCallBacks( StateMachine::INITIALIZE ); } );
    }
    StatusCode sysStart() override {
      return Parent::sysStart().andThen( [&]() { handleCallBacks( StateMachine::START ); } );
    }
    StatusCode sysStop() override {
      return Parent::sysStop().andThen( [&]() { handleCallBacks( StateMachine::STOP ); } );
    }
    StatusCode sysFinalize() override {
      return Parent::sysFinalize().andThen( [&]() { handleCallBacks( StateMachine::FINALIZE ); } );
    }

  private:
    void handleCallBacks( StateMachine::Transition state ) {
      std::for_each( m_callbacks[state].begin(), m_callbacks[state].end(),
                     []( IFSMCallbackHolder::CallBack const& c ) { c(); } );
    }

    std::map<StateMachine::Transition, std::vector<IFSMCallbackHolder::CallBack>> m_callbacks;
  };

} // namespace Gaudi
