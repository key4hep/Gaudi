// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/IAppMgrUI.h,v 1.6 2008/06/02 14:20:38 marcocle Exp $
#ifndef GAUDIKERNEL_IAPPMGRUI_H
#define GAUDIKERNEL_IAPPMGRUI_H 1

// Include files
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/StateMachine.h"

#include <string>

/** @class IAppMgrUI IAppMgrUI.h GaudiKernel/IAppMgrUI.h

    Application Manager User Interface. This is the interface
    offered to the UI to control the JOB or Application.

    @author Pere Mato
    @date   30/10/98
*/

class GAUDI_API IAppMgrUI: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IAppMgrUI,3,0);
  /// Run the complete job (from initialize to terminate)
  virtual StatusCode run() = 0;
  /// Configure the job
  virtual StatusCode configure() = 0;
  /// Terminate the job
  virtual StatusCode terminate() = 0;
  /// Initialize the job
  virtual StatusCode initialize() = 0;
  /// Finalize the job
  virtual StatusCode finalize() = 0;
  /// Process the next maxevt events
  virtual StatusCode nextEvent(int maxevt) = 0;

  /// The identifying name of the AppMgrUI object.
  virtual const std::string& name() const = 0;

  /// Start (from INITIALIZED to RUNNING).
  virtual StatusCode start() = 0;

  /// Stop (from RUNNING to INITIALIZED).
  virtual StatusCode stop() = 0;

  /// Initialization (from INITIALIZED or RUNNING to INITIALIZED, via CONFIGURED).
  virtual StatusCode reinitialize() = 0;

  /// Initialization (from RUNNING to RUNNING, via INITIALIZED).
  virtual StatusCode restart() = 0;

  /// Get the current state.
  virtual Gaudi::StateMachine::State FSMState() const = 0;

};
#endif  // KERNEL_IAPPMGRUI_H
