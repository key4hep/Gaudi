/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef __INCIDENT_PROC_ALG_H
#define __INCIDENT_PROC_ALG_H

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/MsgStream.h"
#include <Gaudi/Property.h>

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
