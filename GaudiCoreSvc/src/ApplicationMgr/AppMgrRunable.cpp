#define  GAUDISVC_APPMGRRUNABLE_CPP

#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/ISvcLocator.h"

#include "AppMgrRunable.h"

// Instantiation of a static factory class used by clients to create instances of this service

DECLARE_COMPONENT(AppMgrRunable)

// IService implementation: initialize the service
StatusCode AppMgrRunable::initialize()   {
  StatusCode sc = Service::initialize();
  if ( sc.isSuccess() )     {
    sc = serviceLocator()->queryInterface(IAppMgrUI::interfaceID(), pp_cast<void>(&m_appMgrUI));
    // get property from application manager
    if ( m_evtMax == (int)0xFEEDBABE )   {
      auto props = serviceLocator()->as<IProperty>();
      setProperty(props->getProperty("EvtMax")).ignore();
    }
  }
  return sc;
}

// IService implementation: initialize the service
StatusCode AppMgrRunable::start()   {
  StatusCode sc = Service::start();
  return sc;
}

// IService implementation: initialize the service
StatusCode AppMgrRunable::stop()   {
  StatusCode sc = Service::stop();
  return sc;
}

// IService implementation: finalize the service
StatusCode AppMgrRunable::finalize()     {
  StatusCode sc = Service::finalize();
  if ( m_appMgrUI ) m_appMgrUI->release();
  m_appMgrUI = nullptr;
  return sc;
}

// IRunable implementation : Run the class implementation
StatusCode AppMgrRunable::run()   {
  if ( m_appMgrUI )    {
    // loop over the events
    return m_appMgrUI->nextEvent(m_evtMax);
  }
  return StatusCode::FAILURE;
}
