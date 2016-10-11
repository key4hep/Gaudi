#ifndef GAUDIEXAMPLE_WRITEALG_H
#define GAUDIEXAMPLE_WRITEALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"

/** WriteAlg class for the RootIOExample

    @author Markus Frank
*/
class WriteAlg : public Algorithm
{
public:
  /// Constructor: A constructor of this form must be provided.
  WriteAlg( const std::string& name, ISvcLocator* pSvcLocator ) : Algorithm( name, pSvcLocator ) {}
  /// Standard Destructor
  ~WriteAlg() override {}
  /// Initialize
  StatusCode initialize() override;
  /// Finalize
  StatusCode finalize() override;
  /// Event callback
  StatusCode execute() override;
};

#endif // GAUDIEXAMPLE_WRITEALG_H
