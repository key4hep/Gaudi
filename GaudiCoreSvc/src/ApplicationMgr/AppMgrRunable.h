// $Id: AppMgrRunable.h,v 1.4 2008/06/02 14:21:35 marcocle Exp $
#ifndef GAUDISVC_APPMGRRUNABLE_H
#define GAUDISVC_APPMGRRUNABLE_H 1

// Framework includes
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IRunable.h"

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
class AppMgrRunable : public extends1<Service, IRunable> {
protected:
  /// Reference to application manager UI
  IAppMgrUI*    m_appMgrUI;
  /// Number of events to be processed
  int           m_evtMax;

public:
  /// Standard Constructor
  AppMgrRunable(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  virtual ~AppMgrRunable();

  /// IService implementation: initialize the service
  virtual StatusCode initialize();
  /// IService implementation: start the service
  virtual StatusCode start();
  /// IService implementation: stop the service
  virtual StatusCode stop();
  /// IService implementation: finalize the service
  virtual StatusCode finalize();
  /// IRunable implementation : Run the class implementation
  virtual StatusCode run();
};
#endif // GAUDISVC_APPMGRRUNABLE_H
