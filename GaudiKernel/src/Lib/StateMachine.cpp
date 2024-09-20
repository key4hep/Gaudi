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
#include <GaudiKernel/StateMachine.h>

namespace Gaudi {
  namespace StateMachine {
    /**
     * Function to get the new state according to the required transition, checking
     * if the transition is allowed.
     */
    State ChangeState( const Transition transition, const State state ) {
      switch ( transition ) {
      case CONFIGURE:
        if ( OFFLINE == state ) { return CONFIGURED; }
        break;
      case INITIALIZE:
        if ( CONFIGURED == state ) { return INITIALIZED; }
        break;
      case START:
        if ( INITIALIZED == state ) { return RUNNING; }
        break;
      case STOP:
        if ( RUNNING == state ) { return INITIALIZED; }
        break;
      case FINALIZE:
        if ( INITIALIZED == state ) { return CONFIGURED; }
        break;
      case TERMINATE:
        if ( CONFIGURED == state ) { return OFFLINE; }
        break;
      default:
        break;
      }
      std::stringstream msg;
      msg << "Invalid transition '" << transition << "' from state '" << state << "'";
      throw GaudiException( msg.str(), "Gaudi::StateMachine::ChangeState", StatusCode::FAILURE );
      return OFFLINE; // never reached, but maked the compiler happy
    }
  } // namespace StateMachine
} // namespace Gaudi
