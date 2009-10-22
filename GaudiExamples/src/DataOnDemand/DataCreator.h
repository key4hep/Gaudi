// $Id: DataCreator.h,v 1.1 2004/10/25 17:53:48 mato Exp $
#ifndef GAUDIEXAMPLE_DATACREATOR_H
#define GAUDIEXAMPLE_DATACREATOR_H 1

// Include files
#include "GaudiAlg/GaudiAlgorithm.h"

/** @class DataCreator
    Trivial Algorithm for test purposes
    @author nobody
*/
class DataCreator : public GaudiAlgorithm {
public:
  /// Constructor of this form must be provided
  DataCreator(const std::string& name, ISvcLocator* pSvcLocator); 

  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
private:
  std::string m_data;
};

#endif    // GAUDIEXAMPLE_HELLOWORLD_H
