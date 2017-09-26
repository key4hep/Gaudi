#ifndef HISTOGRAMS_GAUDIHISTOALGORITHM_H
#define HISTOGRAMS_GAUDIHISTOALGORITHM_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiHistoAlg.h"

/** @class GaudiHistoAlgorithm GaudiHistoAlgorithm.h Histograms/GaudiHistoAlgorithm.h
 *
 *  Example algorithm for the GaudiHistoAlg class
 *  The same functionality is available in GaudiHistoTool
 *
 *  @author Chris Jones
 *  @date   2005-08-12
 */

class GaudiHistoAlgorithm : public GaudiHistoAlg
{

public:
  /// Standard constructor
  GaudiHistoAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

  ~GaudiHistoAlgorithm() override; ///< Destructor

  StatusCode initialize() override; ///< Algorithm initialization
  StatusCode execute() override;    ///< Algorithm execution
  StatusCode finalize() override;   ///< Algorithm finalization
};

#endif // HISTOGRAMS_GAUDIHISTOALGORITHM_H
