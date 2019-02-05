#ifndef HISTOGRAMS_GAUDIHISTOALGORITHM_H
#define HISTOGRAMS_GAUDIHISTOALGORITHM_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

// Event Model related classes
#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"

/** @class GaudiCommonTests GaudiCommonTests.h
 *
 *  Example algorithm for the GaudiCommon class, to test various features.
 *
 *  @author Chris Jones
 *  @date   2005-08-12
 */

class GaudiCommonTests : public GaudiAlgorithm {

public:
  /// Standard constructor
  GaudiCommonTests( const std::string& name, ISvcLocator* pSvcLocator );

  ~GaudiCommonTests() override; ///< Destructor

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization
};

#endif // HISTOGRAMS_GAUDIHISTOALGORITHM_H
