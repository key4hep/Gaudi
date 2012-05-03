#include "GaudiKernel/Service.h"

namespace GaudiTesting {

  /** Special service that issue a failure in one of the transitions (for testing).
   */
  class FailingSvc: public Service {
  public:
    /// Standard Constructor
    FailingSvc(const std::string& name, ISvcLocator *pSvcLocator):
      Service(name, pSvcLocator) {
      declareProperty("Transition", m_transition = "",
          "In which transition to fail ['initialize', 'start', 'stop', 'finalize']");
      declareProperty("Mode", m_mode = "failure",
          "Type of failure ['failure', 'exception']");
    }
    virtual ~FailingSvc(){}

    StatusCode initialize() {
      StatusCode sc = Service::initialize();
      if (sc.isFailure()) {
        error() << "failed to initialize base class" << endmsg;
        return sc;
      }
      return handle("initialize");
    }
    StatusCode start() {
      StatusCode sc = Service::start();
      if (sc.isFailure()) {
        error() << "failed to start base class" << endmsg;
        return sc;
      }
      return handle("start");
    }
    StatusCode stop() {
      StatusCode sc = handle("stop");
      if (sc.isFailure()) return sc;
      return Service::stop();
    }
    StatusCode finalize() {
      StatusCode sc = handle("finalize");
      if (sc.isFailure()) return sc;
      return Service::finalize();
    }

  private:
    /// when the service should fail: initialize, start, stop, finalize
    std::string m_transition;
    /// how to fail: failure, exception
    std::string m_mode;
    inline StatusCode handle(const std::string &transition) {
      if (m_transition == transition) {
        if (m_mode == "exception") {
          throw GaudiException("forced failure in " + transition, name(), StatusCode::FAILURE);
        } else if (m_mode == "failure") {
          return StatusCode::FAILURE;
        } else {
          warning() << "Unknown type of failure '" << m_mode
                    << "', use 'failure' or 'exception'" << endmsg;
        }
      }
      return StatusCode::SUCCESS;
    }
  };

}


#include "GaudiKernel/SvcFactory.h"

DECLARE_NAMESPACE_SERVICE_FACTORY(GaudiTesting, FailingSvc)
