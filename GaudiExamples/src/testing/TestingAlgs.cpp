/*
 * TestingAlg1.cpp
 *
 *  Created on: Sep 7, 2009
 *      Author: Marco Clemencic
 */

#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Sleep.h"

#include <iostream>

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

}

#include "GaudiKernel/AlgFactory.h"

DECLARE_NAMESPACE_ALGORITHM_FACTORY(GaudiTesting, DestructorCheckAlg)
DECLARE_NAMESPACE_ALGORITHM_FACTORY(GaudiTesting, SleepyAlg)
