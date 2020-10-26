//
//  HiveTestAlgorithm.h
//
//
//  Created by Benedikt Hegner on 7/21/12.
//  Copyright (c) 2012 __CERN__. All rights reserved.
//

#include "GaudiAlg/GaudiAlgorithm.h"
#include <Gaudi/Property.h>

class GAUDI_API HiveTestAlgorithm : public GaudiAlgorithm {
public:
  /**
   ** Constructor(s)
   **/
  using GaudiAlgorithm::GaudiAlgorithm;

  /*****************************
   ** Public Function Members **
   *****************************/

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

  // BH: concurrency additions
  virtual const std::vector<std::string> get_inputs();
  virtual const std::vector<std::string> get_outputs();

private:
  /**************************
   ** Private Data Members **
   **************************/

  /**
   ** The total events seen.
   **/
  int m_total = 0;

  Gaudi::Property<std::vector<std::string>> m_inputs{this, "Input", {}, "List of required inputs"};
  Gaudi::Property<std::vector<std::string>> m_outputs{this, "Output", {}, "List of provided outputs"};

  std::vector<DataObjectHandle<DataObject>*> m_inputHandles;
  std::vector<DataObjectHandle<DataObject>*> m_outputHandles;
};
