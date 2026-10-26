/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

// Framework includes
#include <GaudiKernel/IAppMgrUI.h>
#include <GaudiKernel/IRunable.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/Service.h>
#include <GaudiKernel/SmartIF.h>

class IAppMgrUI;

/** Class definition of AppMgrRunable.
    This is the default runnable of the application manager.
    The runable handles the actual run() implementation of the
    ApplicationMgr object.
    This implementation just calls ApplicationMgr::nextEvent(...).

    <B>History:</B>
    <PRE>
    +---------+----------------------------------------------+---------+
    |    Date |                 Comment                      | Who     |
    +---------+----------------------------------------------+---------+
    |13/12/00 | Initial version                              | M.Frank |
    +---------+----------------------------------------------+---------+
    </PRE>
   @author Markus Frank
   @version 1.0
*/
class AppMgrRunable : public extends<Service, IRunable> {
protected:
  /// Reference to application manager UI
  SmartIF<IAppMgrUI>   m_appMgrUI;
  Gaudi::Property<int> m_evtMax{ this, "EvtMax", 0xFEEDBABE, "number of events to be processed" };

public:
  /// inherit contructor
  using extends::extends;

  /// IService implementation: initialize the service
  StatusCode initialize() override;
  /// IService implementation: start the service
  StatusCode start() override;
  /// IService implementation: stop the service
  StatusCode stop() override;
  /// IService implementation: finalize the service
  StatusCode finalize() override;
  /// IRunable implementation : Run the class implementation
  StatusCode run() override;
};

// Instantiation of a static factory class used by clients to create instances of this service

DECLARE_COMPONENT( AppMgrRunable )

// IService implementation: initialize the service
StatusCode AppMgrRunable::initialize() {
  return base_class::initialize()
      .andThen( [this] {
        m_appMgrUI = serviceLocator();
        if ( !m_appMgrUI ) {
          error() << "Failed to get IAppMgrUI" << endmsg;
          return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
      } )
      .andThen( [this]() -> StatusCode {
        if ( m_evtMax == (int)0xFEEDBABE ) {
          auto props = serviceLocator()->as<IProperty>();
          return setProperty( props->getProperty( "EvtMax" ) );
        }
        return StatusCode::SUCCESS;
      } );
}

// IService implementation: initialize the service
StatusCode AppMgrRunable::start() {
  StatusCode sc = Service::start();
  return sc;
}

// IService implementation: initialize the service
StatusCode AppMgrRunable::stop() {
  StatusCode sc = Service::stop();
  return sc;
}

// IService implementation: finalize the service
StatusCode AppMgrRunable::finalize() {
  StatusCode sc = Service::finalize();
  if ( m_appMgrUI ) m_appMgrUI->release();
  m_appMgrUI = nullptr;
  return sc;
}

// IRunable implementation : Run the class implementation
StatusCode AppMgrRunable::run() {
  if ( m_appMgrUI ) {
    // loop over the events
    return m_appMgrUI->nextEvent( m_evtMax );
  }
  return StatusCode::FAILURE;
}
