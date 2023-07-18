/***********************************************************************************\
* (c) Copyright 2012-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Property.h>
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/DataObjectHandle.h>

class GAUDI_API HiveTestAlgorithm : public Algorithm {
public:
  /**
   ** Constructor(s)
   **/
  using Algorithm::Algorithm;

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

  Gaudi::Property<std::vector<std::string>> m_inputs{ this, "Input", {}, "List of required inputs" };
  Gaudi::Property<std::vector<std::string>> m_outputs{ this, "Output", {}, "List of provided outputs" };

  std::vector<std::unique_ptr<DataObjectHandle<DataObject>>> m_inputHandles;
  std::vector<std::unique_ptr<DataObjectHandle<DataObject>>> m_outputHandles;
};
