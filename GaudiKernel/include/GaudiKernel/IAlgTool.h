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
#ifndef GAUDIKERNEL_IALGTOOL_H
#define GAUDIKERNEL_IALGTOOL_H

// Include files
#include "GaudiKernel/INamedInterface.h"
#include "GaudiKernel/IStateful.h"
#include <string>

// Forward declarations
class IAlgorithm;

/** @class IAlgTool IAlgTool.h GaudiKernel/IAlgTool.h

    The interface implemented by the AlgTool base class.
    Concrete tools, derived from the AlgTool based class are
    controlled via this interface.

    @author Gloria Corti
    @author Pere Mato
    @date 15/11/01 version 2 introduced
*/

class GAUDI_API IAlgTool : virtual public INamedInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IAlgTool, 4, 0 );

  /// The type of an AlgTool, meaning the concrete AlgTool class.
  virtual const std::string& type() const = 0;

  /** The parent of the concrete AlgTool. It can be an Algorithm
      or a Service. A common AlgTool has the ToolSvc as parent.
  */
  virtual const IInterface* parent() const = 0;

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

  /** Initialization of the Tool. This method is called typically
   *  by the ToolSvc. It allows to complete  the initialization that
   *  could not be done with a simply a constructor.
   */
  virtual StatusCode sysInitialize() = 0;

  /** Initialization of the Tool. This method is called typically
   *  by the ToolSvc. It allows to complete  the initialization that
   *  could not be done with a simply a constructor.
   */
  virtual StatusCode sysStart() = 0;

  /** Initialization of the Tool. This method is called typically
   *  by the ToolSvc. It allows to complete  the initialization that
   *  could not be done with a simply a constructor.
   */
  virtual StatusCode sysStop() = 0;

  /** Finalization of the Tool. This method is called typically
   *  by the ToolSvc. For completeless this method is called before
   *  the tool is destructed.
   */
  virtual StatusCode sysFinalize() = 0;

  /** Initialization of the Tool. This method is called typically
   *  by the ToolSvc. It allows to complete  the initialization that
   *  could not be done with a simply a constructor.
   */
  virtual StatusCode sysReinitialize() = 0;

  /** Initialization of the Tool. This method is called typically
   *  by the ToolSvc. It allows to complete  the initialization that
   *  could not be done with a simply a constructor.
   */
  virtual StatusCode sysRestart() = 0;
};

#endif // GAUDIKERNEL_IALGTOOL_H
