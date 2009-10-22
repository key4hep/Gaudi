#ifndef COMPONENTMANAGER_H_
#define COMPONENTMANAGER_H_

#include "GaudiKernel/IComponentManager.h"
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/HashMap.h"

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
  virtual const InterfaceID& componentBaseInterface() const;

  /// Virtual destructor
  virtual ~ComponentManager();

  /// Specialized queryInterface implementation.
  /// If an interface is not fount in the implemented ones, we fall back on the
  /// owner.
  StatusCode queryInterface(const InterfaceID& iid, void** pinterface);

  virtual SmartIF<ISvcLocator>& serviceLocator() const {
    if (!m_svcLocator) m_svcLocator = m_application;
    return m_svcLocator;
  }

  /// Configuration (from OFFLINE to CONFIGURED).
  virtual StatusCode configure() {return StatusCode::SUCCESS;}

  /// Initialization (from CONFIGURED to INITIALIZED).
  virtual StatusCode initialize() {return StatusCode::SUCCESS;}

  /// Start (from INITIALIZED to RUNNING).
  virtual StatusCode start() {return StatusCode::SUCCESS;}

  /// Stop (from RUNNING to INITIALIZED).
  virtual StatusCode stop() {return StatusCode::SUCCESS;}

  /// Finalize (from INITIALIZED to CONFIGURED).
  virtual StatusCode finalize() {return StatusCode::SUCCESS;}

  /// Initialization (from CONFIGURED to OFFLINE).
  virtual StatusCode terminate() {return StatusCode::SUCCESS;}


  /// Initialization (from INITIALIZED or RUNNING to INITIALIZED, via CONFIGURED).
  virtual StatusCode reinitialize() {return StatusCode::SUCCESS;}

  /// Initialization (from RUNNING to RUNNING, via INITIALIZED).
  virtual StatusCode restart() {return StatusCode::SUCCESS;}

  /// Get the current state.
  virtual Gaudi::StateMachine::State FSMState() const {return m_stateful->FSMState();}

  /// When we are in the middle of a transition, get the state where the
  /// transition is leading us. Otherwise it returns the same state as state().
  virtual Gaudi::StateMachine::State targetFSMState() const {return m_stateful->targetFSMState();}

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
