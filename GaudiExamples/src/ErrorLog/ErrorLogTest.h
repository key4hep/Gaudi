// $Id: ErrorLogTest.h,v 1.1 2006/09/14 12:34:15 hmd Exp $
#ifndef GAUDIEXAMPLE_ERRORLOGTEST_H
#define GAUDIEXAMPLE_ERRORLOGTEST_H 1

// Include files
#include "GaudiKernel/Algorithm.h"

class ErrorLogTest : public Algorithm {

public:
  // Constructor of this form must be provided
  ErrorLogTest(const std::string& name, ISvcLocator* pSvcLocator); 

  // Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

private:


};


# endif    // 
