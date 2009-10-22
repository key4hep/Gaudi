// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IService.h,v 1.8 2008/06/02 14:20:38 marcocle Exp $
#ifndef GAUDIKERNEL_ISERVICE_H
#define GAUDIKERNEL_ISERVICE_H

// Include files
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/StateMachine.h"
#include <string>

/** @class IService IService.h GaudiKernel/IService.h

    General service interface definition

    @author Pere Mato
*/
class ISvcManager;
class ServiceManager;

class GAUDI_API IService: virtual public INamedInterface {
  friend class ServiceManager;
public:
  /// InterfaceID
  DeclareInterfaceID(IService,3,0);

  /// Initialize Service
  virtual StatusCode sysInitialize() = 0;
  /// Start Service
  virtual StatusCode sysStart() = 0;
  /// Stop Service
  virtual StatusCode sysStop() = 0;
  /// Finalize Service
  virtual StatusCode sysFinalize() = 0;
  /// Re-initialize the Service
  virtual StatusCode sysReinitialize() = 0;
  /// Re-start the Service
  virtual StatusCode sysRestart() = 0;

  // --- Methods from IStateful ---
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

protected:
  virtual void setServiceManager(ISvcManager*) = 0;
};

#endif  // GAUDIKERNEL_ISERVICE_H
