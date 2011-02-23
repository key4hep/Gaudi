#ifndef GAUDIKERNEL_STATEMACHINE_H_
#define GAUDIKERNEL_STATEMACHINE_H_

#include "GaudiKernel/GaudiException.h"

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
  RUNNING//,
  //FINALIZED = CONFIGURED
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
State GAUDI_API ChangeState(const Transition transition, const State state);

} // namespace Gaudi
} // namespace StateMachine

/// Pretty print of states.
inline std::ostream & operator << (std::ostream & s, const Gaudi::StateMachine::State &st) {
  switch (st) {
  case Gaudi::StateMachine::OFFLINE     : return s << "OFFLINE";
  case Gaudi::StateMachine::CONFIGURED  : return s << "CONFIGURED";
  case Gaudi::StateMachine::INITIALIZED : return s << "INITIALIZED";
  case Gaudi::StateMachine::RUNNING     : return s << "RUNNING";
  }
  return s; // cannot be reached, but make the compiler happy
}

/// Pretty print of transitions.
inline std::ostream & operator << (std::ostream & s, const Gaudi::StateMachine::Transition &t) {
  switch (t) {
  case Gaudi::StateMachine::CONFIGURE  : return s << "CONFIGURE";
  case Gaudi::StateMachine::INITIALIZE : return s << "INITIALIZE";
  case Gaudi::StateMachine::START      : return s << "START";
  case Gaudi::StateMachine::STOP       : return s << "STOP";
  case Gaudi::StateMachine::FINALIZE   : return s << "FINALIZE";
  case Gaudi::StateMachine::TERMINATE  : return s << "TERMINATE";
  }
  return s; // cannot be reached, but make the compiler happy
}

#endif /*GAUDIKERNEL_STATEMACHINE_H_*/
