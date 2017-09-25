#ifndef PARTPROP_GAUDIPPS_H
#define PARTPROP_GAUDIPPS_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

namespace GaudiExamples
{
  /** Small algorithm using Gaudi::ParticlePropertySvc.
   *
   *  @author Marco CLEMENCIC
   *  @date   2008-05-23
   */
  class GaudiPPS : public GaudiAlgorithm
  {
  public:
    /// Standard constructor
    GaudiPPS( const std::string& name, ISvcLocator* pSvcLocator );

    ~GaudiPPS() override; ///< Destructor

    StatusCode initialize() override; ///< Algorithm initialization
    StatusCode execute() override;    ///< Algorithm execution
    StatusCode finalize() override;   ///< Algorithm finalization
  };
}
#endif // PARTPROP_GAUDIPPS_H
