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
#ifndef GAUDIKERNEL_STATEMACHINE_H_
#define GAUDIKERNEL_STATEMACHINE_H_

#include <GaudiKernel/GaudiException.h>

namespace Gaudi {
  namespace StateMachine {
    /**
     * Allowed states for classes implementing the state machine (ApplicationMgr,
     * Algorithm, Service, AlgTool).
     */
    enum State {
      OFFLINE,
      CONFIGURED,
      INITIALIZED,
      RUNNING //,
      // FINALIZED = CONFIGURED
    };

    /**
     * Allowed transitions between states.
     */
    enum Transition {
      CONFIGURE,  // OFFLINE     -> CONFIGURED
      INITIALIZE, // CONFIGURED  -> INITIALIZED
      START,      // INITIALIZED -> RUNNING
      STOP,       // RUNNING     -> INITIALIZED
      FINALIZE,   // INITIALIZED -> CONFIGURED
      TERMINATE   // CONFIGURED  -> OFFLINE
    };

    /**
     * Function to get the new state according to the required transition, checking
     * if the transition is allowed.
     */
    State GAUDI_API ChangeState( const Transition transition, const State state );

    /// Pretty print of states.
    inline std::ostream& operator<<( std::ostream& s, const Gaudi::StateMachine::State& st ) {
      switch ( st ) {
      case Gaudi::StateMachine::OFFLINE:
        return s << "OFFLINE";
      case Gaudi::StateMachine::CONFIGURED:
        return s << "CONFIGURED";
      case Gaudi::StateMachine::INITIALIZED:
        return s << "INITIALIZED";
      case Gaudi::StateMachine::RUNNING:
        return s << "RUNNING";
      default:
        return s;
      }
    }

    /// Pretty print of transitions.
    inline std::ostream& operator<<( std::ostream& s, const Gaudi::StateMachine::Transition& t ) {
      switch ( t ) {
      case Gaudi::StateMachine::CONFIGURE:
        return s << "CONFIGURE";
      case Gaudi::StateMachine::INITIALIZE:
        return s << "INITIALIZE";
      case Gaudi::StateMachine::START:
        return s << "START";
      case Gaudi::StateMachine::STOP:
        return s << "STOP";
      case Gaudi::StateMachine::FINALIZE:
        return s << "FINALIZE";
      case Gaudi::StateMachine::TERMINATE:
        return s << "TERMINATE";
      default:
        return s;
      }
    }

  } // namespace StateMachine
} // namespace Gaudi

#endif /*GAUDIKERNEL_STATEMACHINE_H_*/
