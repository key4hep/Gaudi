#ifndef __INCIDENT_PROC_ALG_H
#define __INCIDENT_PROC_ALG_H

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Property.h"

class GAUDI_API IncidentProcAlg : public Algorithm {
public:
  /**
   ** Constructor(s)
   **/
  IncidentProcAlg( const std::string& name, ISvcLocator* pSvcLocator );

  /**
   ** Destructor
   **/
  virtual ~IncidentProcAlg(){};

  /*****************************
   ** Public Function Members **
   *****************************/

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  SmartIF<IIncidentSvc> m_incSvc;
  /**************************
   ** Private Data Members **
   **************************/
};

#endif
