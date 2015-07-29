#ifndef COMPONENTMANAGER_H_
#define COMPONENTMANAGER_H_

#include "GaudiKernel/IComponentManager.h"
#include "GaudiKernel/CommonMessaging.h"

/** @class ComponentManager ComponentManager.h
 *
 * Base class for a component manager.
 *
 * @author Marco Clemencic
 */
class GAUDI_API ComponentManager: public CommonMessaging<implements1<IComponentManager> > {
public:

  /// Constructor.
  /// @param application    the manager of managers
  ComponentManager(IInterface *application, const InterfaceID &baseIID);

  /// Basic interface id of the managed components.
  const InterfaceID& componentBaseInterface() const override;

  /// Virtual destructor
  ~ComponentManager() override = default;

  /// Specialized queryInterface implementation.
  /// If an interface is not fount in the implemented ones, we fall back on the
  /// owner.
  StatusCode queryInterface(const InterfaceID& iid, void** pinterface);

  SmartIF<ISvcLocator>& serviceLocator() const override {
    if (!m_svcLocator) m_svcLocator = m_application;
    return m_svcLocator;
  }

  /// Configuration (from OFFLINE to CONFIGURED).
  StatusCode configure() override {return StatusCode::SUCCESS;}

  /// Initialization (from CONFIGURED to INITIALIZED).
  StatusCode initialize() override {return StatusCode::SUCCESS;}

  /// Start (from INITIALIZED to RUNNING).
  StatusCode start() override {return StatusCode::SUCCESS;}

  /// Stop (from RUNNING to INITIALIZED).
  StatusCode stop() override {return StatusCode::SUCCESS;}

  /// Finalize (from INITIALIZED to CONFIGURED).
  StatusCode finalize() override {return StatusCode::SUCCESS;}

  /// Initialization (from CONFIGURED to OFFLINE).
  StatusCode terminate() override {return StatusCode::SUCCESS;}


  /// Initialization (from INITIALIZED or RUNNING to INITIALIZED, via CONFIGURED).
  StatusCode reinitialize() override {return StatusCode::SUCCESS;}

  /// Initialization (from RUNNING to RUNNING, via INITIALIZED).
  StatusCode restart() override {return StatusCode::SUCCESS;}

  /// Get the current state.
  Gaudi::StateMachine::State FSMState() const override {return m_stateful->FSMState();}

  /// When we are in the middle of a transition, get the state where the
  /// transition is leading us. Otherwise it returns the same state as state().
  Gaudi::StateMachine::State targetFSMState() const override {return m_stateful->targetFSMState();}

protected:

  /// Pointer to the owner of the manager
  SmartIF<IInterface> m_application;

  /// Pointer to the IStateful interface of the owner
  SmartIF<IStateful> m_stateful;

  /// Basic interface id of the managed components.
  InterfaceID m_basicInterfaceId;

  /// Service locator (needed to access the MessageSvc)
  mutable SmartIF<ISvcLocator> m_svcLocator;

};


#endif /* COMPONENTMANAGER_H_ */
