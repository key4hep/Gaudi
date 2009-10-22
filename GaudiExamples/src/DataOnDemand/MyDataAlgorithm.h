// $Id: MyDataAlgorithm.h,v 1.1 2005/01/18 17:28:39 mato Exp $
#ifndef GAUDIEXAMPLE_MyDataAlgorithm_H
#define GAUDIEXAMPLE_MyDataAlgorithm_H 1

// Include files
#include "GaudiAlg/GaudiAlgorithm.h"

/** @class MyDataAlgorithm
    Trivial Algorithm for tutotial purposes
        @author nobody
*/

class MyDataAlgorithm : public GaudiAlgorithm {
public:
  /// Constructor of this form must be provided
  MyDataAlgorithm(const std::string& name, ISvcLocator* pSvcLocator); 

  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
private:
};

#endif    // GAUDIEXAMPLE_MyDataAlgorithm_H
