#ifndef GAUDIEXAMPLE_MyDataAlgorithm_H
#define GAUDIEXAMPLE_MyDataAlgorithm_H 1

// Include files
#include "GaudiAlg/GaudiAlgorithm.h"

/** @class MyDataAlgorithm
    Trivial Algorithm for tutotial purposes
        @author nobody
*/

class MyDataAlgorithm : public GaudiAlgorithm
{
public:
  /// Constructor of this form must be provided
  MyDataAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
};

#endif // GAUDIEXAMPLE_MyDataAlgorithm_H
