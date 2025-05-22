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

#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/StateMachine.h>
#include <string>

/** @class IStateful IStateful.h GaudiKernel/IStateful.h
 *
 * Interface for classes that implement the Gaudi State Machine
 *
 * @author Marco Clemencic
 * @date   2008-05-15
 */
class GAUDI_API IStateful : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IStateful, 1, 0 );

  /** Configuration (from OFFLINE to CONFIGURED).
   */
  virtual StatusCode configure() = 0;

  /** Initialization (from CONFIGURED to INITIALIZED).
   */
  virtual StatusCode initialize() = 0;

  /** Start (from INITIALIZED to RUNNING).
   */
  virtual StatusCode start() = 0;

  /** Stop (from RUNNING to INITIALIZED).
   */
  virtual StatusCode stop() = 0;

  /** Finalize (from INITIALIZED to CONFIGURED).
   */
  virtual StatusCode finalize() = 0;

  /** Initialization (from CONFIGURED to OFFLINE).
   */
  virtual StatusCode terminate() = 0;

  /** Initialization (from INITIALIZED or RUNNING to INITIALIZED, via CONFIGURED).
   */
  virtual StatusCode reinitialize() = 0;

  /** Initialization (from RUNNING to RUNNING, via INITIALIZED).
   */
  virtual StatusCode restart() = 0;

  /** Get the current state.
   */
  virtual Gaudi::StateMachine::State FSMState() const = 0;

  /** When we are in the middle of a transition, get the state where the
   *  transition is leading us. Otherwise it returns the same state as state().
   */
  virtual Gaudi::StateMachine::State targetFSMState() const = 0;
};
