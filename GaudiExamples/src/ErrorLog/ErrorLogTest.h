#ifndef GAUDIEXAMPLE_ERRORLOGTEST_H
#define GAUDIEXAMPLE_ERRORLOGTEST_H 1

// Include files
#include "GaudiKernel/Algorithm.h"

class ErrorLogTest : public Algorithm
{

public:
  // Constructor of this form must be provided
  ErrorLogTest( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
};

#endif //
