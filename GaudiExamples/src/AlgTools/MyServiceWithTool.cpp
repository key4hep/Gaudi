#include "GaudiKernel/Service.h"

#include "MyTool.h"


namespace GaudiTesting {

  /** Special service that issue a failure in one of the transitions (for testing).
   */
  class SvcWithTool: public Service {
  public:
    /// Standard Constructor
    SvcWithTool(const std::string& name, ISvcLocator *pSvcLocator):
      Service(name, pSvcLocator) {

    	declareTool(m_tool);

    }
    virtual ~SvcWithTool(){}

    StatusCode initialize() {
      return Service::initialize();
    }
    StatusCode start() {
      return Service::start();
    }
    StatusCode stop() {
      return Service::stop();
    }
    StatusCode finalize() {
      return Service::finalize();
    }

  private:

    ToolHandle<IMyTool> m_tool;

  };

  DECLARE_COMPONENT(SvcWithTool)
}
