/*
 * TestingAlg1.cpp
 *
 *  Created on: Sep 7, 2009
 *      Author: Marco Clemencic
 */

#include "GaudiAlg/GaudiAlgorithm.h"

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

}

#include "GaudiKernel/AlgFactory.h"

DECLARE_NAMESPACE_ALGORITHM_FACTORY(GaudiTesting, DestructorCheckAlg);
