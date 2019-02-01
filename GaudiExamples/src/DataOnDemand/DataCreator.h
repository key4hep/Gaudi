#ifndef GAUDIEXAMPLE_DATACREATOR_H
#define GAUDIEXAMPLE_DATACREATOR_H 1

// Include files
#include "GaudiAlg/GaudiAlgorithm.h"

/** @class DataCreator
    Trivial Algorithm for test purposes
    @author nobody
*/
class DataCreator : public GaudiAlgorithm {
public:
  /// Constructor of this form must be provided
  using GaudiAlgorithm::GaudiAlgorithm;

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  Gaudi::Property<std::string> m_data{this, "Data", "/Event/Unknown"};
};

#endif // GAUDIEXAMPLE_HELLOWORLD_H
