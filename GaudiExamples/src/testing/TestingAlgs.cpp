/*
 * TestingAlg1.cpp
 *
 *  Created on: Sep 7, 2009
 *      Author: Marco Clemencic
 */

#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Sleep.h"
#include "GaudiKernel/IEventProcessor.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/IIncidentSvc.h"

#include <iostream>

#include <csignal>

namespace GaudiTesting {

  class DestructorCheckAlg: public GaudiAlgorithm {
  public:
    DestructorCheckAlg(const std::string& name, ISvcLocator *pSvcLocator):
      GaudiAlgorithm(name, pSvcLocator) {}
    virtual ~DestructorCheckAlg(){
      std::cout << "Destructor of " << name()<< std::endl;
    }
    StatusCode execute(){
      info() << "Executing " << name() << endmsg;
      return StatusCode::SUCCESS;
    }
  };

  class SleepyAlg: public GaudiAlgorithm {
  public:
    SleepyAlg(const std::string& name, ISvcLocator *pSvcLocator):
      GaudiAlgorithm(name, pSvcLocator), m_counter(0) {
      declareProperty("SleepTime", m_sleep = 10,
                      "Seconds to sleep during the execute");
    }
    StatusCode execute(){
      info() << "Executing event " << ++m_counter << endmsg;
      info() << "Sleeping for " << m_sleep << " seconds" << endmsg;
      Gaudi::Sleep(m_sleep);
      info() << "Back from sleep" << endmsg;
      return StatusCode::SUCCESS;
    }
  private:
    int m_sleep;
    int m_counter;
  };

  /**
   * Simple algorithm that raise a signal after N events.
   */
  class SignallingAlg: public GaudiAlgorithm {
    public:
    SignallingAlg(const std::string& name, ISvcLocator *pSvcLocator):
        GaudiAlgorithm(name, pSvcLocator){
      declareProperty("EventCount", m_eventCount = 3,
          "Number of events to let go before raising the signal");
      declareProperty("Signal", m_signal = SIGINT,
          "Signal to raise");
    }
    virtual ~SignallingAlg(){}
    StatusCode execute(){
      if (m_eventCount <= 0) {
        info() << "Raising signal now" << endmsg;
        std::raise(m_signal);
      } else {
        info() << m_eventCount << " events to go" << endmsg;
      }
      --m_eventCount;
      return StatusCode::SUCCESS;
    }
    private:
    /// Events to let go before the signal
    int m_eventCount;
    /// Signal (id) to raise
    int m_signal;
  };

  class StopLoopAlg: public GaudiAlgorithm {
  public:
    StopLoopAlg(const std::string& name, ISvcLocator *pSvcLocator):
      GaudiAlgorithm(name, pSvcLocator) {
      declareProperty("EventCount", m_eventCount = 3,
          "Number of events to let go before breaking the event loop");
      declareProperty("Mode", m_mode = "failure",
          "Type of interruption ['exception', 'stopRun', 'failure']");
    }
    virtual ~StopLoopAlg(){}
    StatusCode execute(){
      if (m_eventCount <= 0) {
        info() << "Stopping loop with " << m_mode << endmsg;
        if (m_mode == "exception") {
          Exception("Stopping loop");
        } else if (m_mode == "stopRun") {
          SmartIF<IEventProcessor> ep(serviceLocator());
          if (ep) ep->stopRun();
          else {
            error() << "Cannot get IEventProcessor" << endmsg;
            return StatusCode::FAILURE;
          }
        } else { // "failure"
          return StatusCode::FAILURE;
        }
      } else {
        info() << m_eventCount << " events to go" << endmsg;
      }
      --m_eventCount;
      return StatusCode::SUCCESS;
    }
  private:
    /// Events to let go before the signal
    int m_eventCount;
    /// Signal (id) to raise
    std::string m_mode;
  };

  class CustomIncidentAlg: public GaudiAlgorithm {
  public:
    CustomIncidentAlg(const std::string& name, ISvcLocator *pSvcLocator):
      GaudiAlgorithm(name, pSvcLocator) {
      declareProperty("EventCount", m_eventCount = 3,
          "Number of events to let go before firing the incident.");
      declareProperty("Incident", m_incident = "",
          "Type of incident to fire.");
    }
    virtual ~CustomIncidentAlg(){}
    StatusCode initialize() {
      StatusCode sc = GaudiAlgorithm::initialize();
      if (sc.isFailure()) return sc;

      if (m_incident.empty()) {
        error() << "The incident type (property Incident) must be declared." << endmsg;
        return StatusCode::FAILURE;
      }

      m_incidentSvc = service("IncidentSvc");
      if (!m_incidentSvc) return StatusCode::FAILURE;

      return StatusCode::SUCCESS;
    }
    StatusCode execute(){
      if (m_eventCount == 0) {
        info() << "Firing incident " << m_incident << endmsg;
        m_incidentSvc->fireIncident(Incident(name(), m_incident));
      } else if (m_eventCount > 0) {
        info() << m_eventCount << " events to go" << endmsg;
      } else {
        info() << "keep processing events..." << endmsg;
      }
      --m_eventCount;
      return StatusCode::SUCCESS;
    }
    StatusCode finalize() {
      m_incidentSvc.reset();
      return GaudiAlgorithm::finalize();
    }
  private:
    /// Events to let go before the signal
    int m_eventCount;
    /// Incident to fire.
    std::string m_incident;
    /// Incident service.
    SmartIF<IIncidentSvc> m_incidentSvc;
  };

  /**
   * Simple algorithm that creates dummy objects in the transient store.
   */
  class PutDataObjectAlg: public GaudiAlgorithm {
  public:
    PutDataObjectAlg(const std::string& name, ISvcLocator *pSvcLocator):
      GaudiAlgorithm(name, pSvcLocator){
      declareProperty("Paths", m_paths,
                      "List of paths in the transient store to load");
      declareProperty("DataSvc", m_dataSvc = "EventDataSvc",
                      "Name of the data service to use");
    }

    StatusCode initialize() {
      StatusCode sc = GaudiAlgorithm::initialize();
      if (sc.isFailure()) return sc;

      m_dataProvider = service(m_dataSvc);
      if (!m_dataProvider) return StatusCode::FAILURE;

      return StatusCode::SUCCESS;
    }

    StatusCode execute() {
      StatusCode sc = StatusCode::SUCCESS;
      info() << "Adding " << m_paths.size() << " objects to " << m_dataSvc << endmsg;
      for (auto& p: m_paths) {
        info() << "Adding '" << p << "'" << endmsg;
        DataObject *obj = new DataObject();
        sc = m_dataProvider->registerObject(p, obj);
        if (sc.isFailure())
          warning() << "Cannot register object '" << p << "'" << endmsg;
      }

      return sc;
    }

    StatusCode finalize() {
      m_dataProvider.reset();
      return GaudiAlgorithm::finalize();
    }
  private:
    std::vector<std::string> m_paths;
    std::string m_dataSvc;
    SmartIF<IDataProviderSvc> m_dataProvider;
  };


  /**
   * Simple algorithm that retrieves objects from the transient store.
   */
  class GetDataObjectAlg: public GaudiAlgorithm {
  public:
    GetDataObjectAlg(const std::string& name, ISvcLocator *pSvcLocator):
      GaudiAlgorithm(name, pSvcLocator){
      declareProperty("Paths", m_paths,
                      "List of paths in the transient store to load");
      declareProperty("DataSvc", m_dataSvc = "EventDataSvc",
                      "Name of the data service to use");
      declareProperty("IgnoreMissing", m_ignoreMissing=false,
                      "if True, missing objects will not beconsidered an error");
    }

    StatusCode initialize() {
      StatusCode sc = GaudiAlgorithm::initialize();
      if (sc.isFailure()) return sc;

      m_dataProvider = service(m_dataSvc);
      if (!m_dataProvider) return StatusCode::FAILURE;

      return StatusCode::SUCCESS;
    }

    StatusCode execute() {
      info() << "Getting " << m_paths.size() << " objects from " << m_dataSvc << endmsg;
      bool missing = false;
      for (auto& p: m_paths) {
        info() << "Getting '" << p << "'" << endmsg;
        DataObject *obj;
        StatusCode sc = m_dataProvider->retrieveObject(p, obj);
        if (sc.isFailure()) {
          warning() << "Cannot retrieve object '" << p << "'" << endmsg;
          missing = true;
        }
      }

      return (missing && ! m_ignoreMissing)
          ? StatusCode::FAILURE
          : StatusCode::SUCCESS;
    }

    StatusCode finalize() {
      m_dataProvider.reset();
      return GaudiAlgorithm::finalize();
    }
  private:
    std::vector<std::string> m_paths;
    std::string m_dataSvc;
    SmartIF<IDataProviderSvc> m_dataProvider;
    bool m_ignoreMissing;
  };

  class OddEventsFilter: public GaudiAlgorithm {
  public:
    OddEventsFilter(const std::string& name, ISvcLocator *pSvcLocator):
      GaudiAlgorithm(name, pSvcLocator), m_counter(0)
    {
    }
    StatusCode initialize() {
      m_counter = 0;
      return GaudiAlgorithm::initialize();
    }
    StatusCode execute() {
      setFilterPassed((++m_counter) % 2);
      return StatusCode::SUCCESS;
    }
  protected:
    int m_counter;
  };

  class EvenEventsFilter: public OddEventsFilter {
  public:
    EvenEventsFilter(const std::string& name, ISvcLocator *pSvcLocator):
      OddEventsFilter(name, pSvcLocator)
    {
    }
    StatusCode execute() {
      setFilterPassed(((++m_counter) % 2) == 0);
      return StatusCode::SUCCESS;
    }
  };

}

#include "GaudiKernel/AlgFactory.h"

namespace GaudiTesting {
  DECLARE_COMPONENT(DestructorCheckAlg)
  DECLARE_COMPONENT(SleepyAlg)
  DECLARE_COMPONENT(SignallingAlg)
  DECLARE_COMPONENT(StopLoopAlg)
  DECLARE_COMPONENT(CustomIncidentAlg)
  DECLARE_COMPONENT(PutDataObjectAlg)
  DECLARE_COMPONENT(GetDataObjectAlg)
  DECLARE_COMPONENT(OddEventsFilter)
  DECLARE_COMPONENT(EvenEventsFilter)
}
