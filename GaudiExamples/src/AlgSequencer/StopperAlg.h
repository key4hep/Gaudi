#ifndef GAUDIEXAMPLE_STOPPERALG_H
#define GAUDIEXAMPLE_STOPPERALG_H 1

// Include files
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"

/** @class ParentAlg
    Trivial Algorithm for tutotial purposes
    
    @author nobody
*/
class StopperAlg : public GaudiAlgorithm {
public:
  /// Constructor of this form must be provided
  StopperAlg(const std::string& name, ISvcLocator* pSvcLocator); 

  /// Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;
private:
  int  m_stopcount;
};

#endif    // GAUDIEXAMPLE_STOPPERALG_H
