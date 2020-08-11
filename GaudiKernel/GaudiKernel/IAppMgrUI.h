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
#ifndef GAUDIKERNEL_IAPPMGRUI_H
#define GAUDIKERNEL_IAPPMGRUI_H 1

// Include files
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/StateMachine.h"
#include "GaudiKernel/StatusCode.h"

#include <string>

namespace Gaudi::Monitoring {
  struct Hub;
}

/** @class IAppMgrUI IAppMgrUI.h GaudiKernel/IAppMgrUI.h

    Application Manager User Interface. This is the interface
    offered to the UI to control the JOB or Application.

    @author Pere Mato
    @date   30/10/98
*/

class GAUDI_API IAppMgrUI : virtual public INamedInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IAppMgrUI, 4, 1 );
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
  virtual StatusCode nextEvent( int maxevt ) = 0;

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

  /// Function to call to update the outputLevel of the components (after a change in MessageSvc).
  virtual void outputLevelUpdate() = 0;

  virtual Gaudi::Monitoring::Hub& monitoringHub() {
    throw GaudiException( "IAppMgrUI", "montorHub access not implemented", StatusCode::FAILURE );
  }
};
#endif // KERNEL_IAPPMGRUI_H
