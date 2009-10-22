// $Id: IStateful.h,v 1.1 2008/06/02 14:20:38 marcocle Exp $
#ifndef GAUDIKERNEL_ISTATEFUL_H
#define GAUDIKERNEL_ISTATEFUL_H

// Include Files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/StateMachine.h"
#include <string>

/** @class IStateful IStateful.h GaudiKernel/IStateful.h
 *
 * Interface for classes that implement the Gaudi State Machine
 *
 * @author Marco Clemencic
 * @date   2008-05-15
 */
class GAUDI_API IStateful: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IStateful,1,0);

  virtual ~IStateful(){} //< Virtual destructor.

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

#endif // GAUDIKERNEL_ISTATEFUL_H
