// $Id: PythonScriptingSvc.h,v 1.2 2004/07/06 10:51:24 mato Exp $
#ifndef GAUDIPYTHON_PYTHONSCRIPTINGSVC_H
#define GAUDIPYTHON_PYTHONSCRIPTINGSVC_H

// Include Files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IRunable.h"

/** @class PythonScriptingSvc PythonScriptingSvc.h GaudiPython/PythonScriptingSvc.h
    This service handles scripting implemented using Python.

    @author Pere Mato
    @author David Quarrie
    @date   2001
*/
class PythonScriptingSvc : public extends1<Service, IRunable> {

public:
  /// Standard Constructor.
  PythonScriptingSvc( const std::string& name, ISvcLocator* svc );

  /// Initialize the service.  [IService::initialize()]
  virtual StatusCode initialize();

  /// Finalize the service. [IService::finalize()]
  virtual StatusCode finalize();

  /// Run the service by taking full control. [IRunable::run()]
  virtual StatusCode run();

protected:
  /// Destructor.
  virtual ~PythonScriptingSvc();
private:
  /// Startup script
  std::string m_startupScript;
};

#endif // GAUDIPYTHON_PYTHONSCRIPTINGSVC_H


