#ifndef GAUDIEXAMPLE_STOPPERALG_H
#define GAUDIEXAMPLE_STOPPERALG_H 1

// Include files
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Property.h"

/** @class ParentAlg
    Trivial Algorithm for tutotial purposes

    @author nobody
*/
class StopperAlg : public GaudiAlgorithm {
public:
  /// Constructor of this form must be provided
  using GaudiAlgorithm::GaudiAlgorithm;

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  Gaudi::Property<int> m_stopcount{this, "StopCount", 3};
};

#endif // GAUDIEXAMPLE_STOPPERALG_H
