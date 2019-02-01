#ifndef PARTPROP_GAUDIPPS_H
#define PARTPROP_GAUDIPPS_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

namespace GaudiExamples {
  /** Small algorithm using Gaudi::ParticlePropertySvc.
   *
   *  @author Marco CLEMENCIC
   *  @date   2008-05-23
   */
  class GaudiPPS : public GaudiAlgorithm {
  public:
    /// Standard constructor
    using GaudiAlgorithm::GaudiAlgorithm;

    StatusCode initialize() override; ///< Algorithm initialization
    StatusCode execute() override;    ///< Algorithm execution
  };
} // namespace GaudiExamples
#endif // PARTPROP_GAUDIPPS_H
