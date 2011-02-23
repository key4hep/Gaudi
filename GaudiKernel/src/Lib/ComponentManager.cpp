/*
 * ComponentManager.cpp
 *
 *  Created on: Feb 20, 2009
 *      Author: Marco Clemencic
 */

#include "GaudiKernel/ComponentManager.h"

//------------------------------------------------------------------------------
// Constructor
ComponentManager::ComponentManager(IInterface *application, const InterfaceID &baseIID):
  m_application(application), m_stateful(application), m_basicInterfaceId(baseIID)
{
}

//------------------------------------------------------------------------------
// Basic interface id of the managed components.
const InterfaceID& ComponentManager::componentBaseInterface() const {
  return m_basicInterfaceId;
}

//------------------------------------------------------------------------------
// Virtual destructor
ComponentManager::~ComponentManager() {
}

//------------------------------------------------------------------------------
// queryInterface implementation that falls back on the owner.
StatusCode ComponentManager::queryInterface(const InterfaceID& iid, void** pinterface)
{
  // try local interfaces
  StatusCode sc = base_class::queryInterface(iid,pinterface);
  if (sc.isSuccess()) return sc;
  // fall back on the owner
  return m_application->queryInterface(iid, pinterface);
}
