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
#ifndef COMPONENTMANAGER_H_
#define COMPONENTMANAGER_H_

#include <GaudiKernel/CommonMessaging.h>
#include <GaudiKernel/IComponentManager.h>

class ApplicationMgr;

/** @class ComponentManager ComponentManager.h
 *
 * Base class for a component manager.
 *
 * @author Marco Clemencic
 */
class GAUDI_API ComponentManager : public CommonMessaging<implements<IComponentManager>> {
public:
  /// Constructor.
  /// @param application    the manager of managers
  ComponentManager( IInterface* application, const InterfaceID& baseIID );

  /// Basic interface id of the managed components.
  const InterfaceID& componentBaseInterface() const override;

  /// Specialized queryInterface implementation.
  /// If an interface is not found in the implemented ones, we fall back on the
  /// owner.
  StatusCode queryInterface( const InterfaceID& iid, void** pinterface ) override;
  /// Specialized i_cast implementation.
  /// If an interface is not found in the implemented ones, we fall back on the
  /// owner.
  void const* i_cast( const InterfaceID& iid ) const override;

  SmartIF<ISvcLocator>& serviceLocator() const override {
    if ( !m_svcLocator ) m_svcLocator = m_application;
    return m_svcLocator;
  }

  /// Configuration (from OFFLINE to CONFIGURED).
  StatusCode configure() override { return StatusCode::SUCCESS; }

  /// Initialization (from CONFIGURED to INITIALIZED).
  StatusCode initialize() override { return StatusCode::SUCCESS; }

  /// Start (from INITIALIZED to RUNNING).
  StatusCode start() override { return StatusCode::SUCCESS; }

  /// Stop (from RUNNING to INITIALIZED).
  StatusCode stop() override { return StatusCode::SUCCESS; }

  /// Finalize (from INITIALIZED to CONFIGURED).
  StatusCode finalize() override { return StatusCode::SUCCESS; }

  /// Initialization (from CONFIGURED to OFFLINE).
  StatusCode terminate() override { return StatusCode::SUCCESS; }

  /// Initialization (from INITIALIZED or RUNNING to INITIALIZED, via CONFIGURED).
  StatusCode reinitialize() override { return StatusCode::SUCCESS; }

  /// Initialization (from RUNNING to RUNNING, via INITIALIZED).
  StatusCode restart() override { return StatusCode::SUCCESS; }

  /// Get the current state.
  Gaudi::StateMachine::State FSMState() const override { return m_stateful->FSMState(); }

  /// When we are in the middle of a transition, get the state where the
  /// transition is leading us. Otherwise it returns the same state as state().
  Gaudi::StateMachine::State targetFSMState() const override { return m_stateful->targetFSMState(); }

protected:
  /// Pointer to the owner of the manager
  SmartIF<IInterface> m_application;

  /// Pointer to the IStateful interface of the owner
  SmartIF<IStateful> m_stateful;

  /// Basic interface id of the managed components.
  InterfaceID m_basicInterfaceId;

  /// Service locator (needed to access the MessageSvc)
  mutable SmartIF<ISvcLocator> m_svcLocator;

  friend ApplicationMgr;
};

#endif /* COMPONENTMANAGER_H_ */
