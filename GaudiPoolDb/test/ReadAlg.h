//$Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/test/ReadAlg.h,v 1.1.1.1 2004/01/16 14:05:04 mato Exp $	//
#ifndef GAUDIEXAMPLES_READALG_H
#define GAUDIEXAMPLES_READALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"  // Required for inheritance

/** @class ReadAlg ReadAlg.h

    ReadAlg class for the RootIOExample

    @author Markus Frank
*/

class ReadAlg : public Algorithm {
public:
	/// Constructor: A constructor of this form must be provided.
	ReadAlg(const std::string& name, ISvcLocator* pSvcLocator)  
    : Algorithm(name, pSvcLocator) { }
  /// Standard Destructor
  virtual ~ReadAlg() { }
  /// Initialize
  virtual StatusCode initialize();
  /// Finalize
  virtual StatusCode finalize() {
    return StatusCode::SUCCESS;
  }
  /// Event callback
  virtual StatusCode execute();
};

#endif // GAUDIEXAMPLES_READALG_H
