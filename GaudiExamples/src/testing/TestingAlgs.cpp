/*
 * TestingAlg1.cpp
 *
 *  Created on: Sep 7, 2009
 *      Author: Marco Clemencic
 */

#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Sleep.h"

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
}

#include "GaudiKernel/AlgFactory.h"

DECLARE_NAMESPACE_ALGORITHM_FACTORY(GaudiTesting, DestructorCheckAlg)
DECLARE_NAMESPACE_ALGORITHM_FACTORY(GaudiTesting, SleepyAlg)
DECLARE_NAMESPACE_ALGORITHM_FACTORY(GaudiTesting, SignallingAlg)
