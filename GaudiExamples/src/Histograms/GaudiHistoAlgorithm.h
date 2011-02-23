// $Id: GaudiHistoAlgorithm.h,v 1.1 2005/10/03 14:15:30 hmd Exp $
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

  virtual ~GaudiHistoAlgorithm( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

};

#endif // HISTOGRAMS_GAUDIHISTOALGORITHM_H
