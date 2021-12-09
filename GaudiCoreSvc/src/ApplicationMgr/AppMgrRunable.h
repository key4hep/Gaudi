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
#ifndef GAUDISVC_APPMGRRUNABLE_H
#define GAUDISVC_APPMGRRUNABLE_H 1

// Framework includes
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/Service.h"

// Forward declarations
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
  IAppMgrUI*           m_appMgrUI = nullptr;
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
#endif // GAUDISVC_APPMGRRUNABLE_H
