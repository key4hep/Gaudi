#ifndef GAUDIPYTHON_PYTHONSCRIPTINGSVC_H
#define GAUDIPYTHON_PYTHONSCRIPTINGSVC_H

// Include Files
#include "GaudiKernel/IRunable.h"
#include "GaudiKernel/Service.h"

/** @class PythonScriptingSvc PythonScriptingSvc.h GaudiPython/PythonScriptingSvc.h
    This service handles scripting implemented using Python.

    @author Pere Mato
    @author David Quarrie
    @date   2001
*/
class PythonScriptingSvc : public extends<Service, IRunable>
{

public:
  /// Standard Constructor.
  PythonScriptingSvc( const std::string& name, ISvcLocator* svc );

  /// Initialize the service.  [IService::initialize()]
  StatusCode initialize() override;

  /// Finalize the service. [IService::finalize()]
  StatusCode finalize() override;

  /// Run the service by taking full control. [IRunable::run()]
  StatusCode run() override;

  /// Destructor.
  ~PythonScriptingSvc() override;

private:
  /// Startup script
  std::string m_startupScript;
};

#endif // GAUDIPYTHON_PYTHONSCRIPTINGSVC_H
