// $Id: GaudiCommonTests.h,v 1.1 2007/05/16 19:22:25 hmd Exp $
#ifndef HISTOGRAMS_GAUDIHISTOALGORITHM_H 
#define HISTOGRAMS_GAUDIHISTOALGORITHM_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

// Event Model related classes
#include "Event.h"
#include "MyTrack.h"

/** @class GaudiCommonTests GaudiCommonTests.h
 *  
 *  Example algorithm for the GaudiCommon class, to test various features.
 *
 *  @author Chris Jones
 *  @date   2005-08-12
 */

class GaudiCommonTests : public GaudiAlgorithm
{

public: 

  /// Standard constructor
  GaudiCommonTests( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~GaudiCommonTests( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

};

#endif // HISTOGRAMS_GAUDIHISTOALGORITHM_H
