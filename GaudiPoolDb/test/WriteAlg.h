//$Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/test/WriteAlg.h,v 1.1.1.1 2004/01/16 14:05:04 mato Exp $	//
#ifndef GAUDIEXAMPLE_WRITEALG_H
#define GAUDIEXAMPLE_WRITEALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h" 

/** @class WriteAlg WriteAlg.h

    WriteAlg class for the RootIOExample

    @author Markus Frank
*/

class WriteAlg : public Algorithm {
public:
  /// Constructor: A constructor of this form must be provided.
  WriteAlg(const std::string& name, ISvcLocator* pSvcLocator)
  :	Algorithm(name, pSvcLocator) { }
  /// Standard Destructor
  virtual ~WriteAlg() { }
  /// Initialize
  virtual StatusCode initialize();
  /// Finalize
  virtual StatusCode finalize();
  /// Event callback
  virtual StatusCode execute();
};

#endif // GAUDIEXAMPLE_WRITEALG_H
