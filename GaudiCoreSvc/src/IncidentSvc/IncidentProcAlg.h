#ifndef __INCIDENT_PROC_ALG_H
#define __INCIDENT_PROC_ALG_H

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IIncidentSvc.h"

class GAUDI_API IncidentProcAlg: public Algorithm {
 public:

  /**
   ** Constructor(s)
   **/
  IncidentProcAlg( const std::string& name, ISvcLocator* pSvcLocator );

  /**
   ** Destructor
   **/
  virtual ~IncidentProcAlg( ){};

  /*****************************
   ** Public Function Members **
   *****************************/

  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();


 private:
  SmartIF<IIncidentSvc> m_incSvc;  
  /**************************
   ** Private Data Members **
   **************************/
};

#endif
