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
    ~SvcWithTool() override = default;

    StatusCode initialize() override {
      return Service::initialize();
    }
    StatusCode start() override {
      return Service::start();
    }
    StatusCode stop() override {
      return Service::stop();
    }
    StatusCode finalize() override {
      return Service::finalize();
    }

  private:

    ToolHandle<IMyTool> m_tool;

  };

  DECLARE_COMPONENT(SvcWithTool)
}
