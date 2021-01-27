/***********************************************************************************\
* (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations *
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

#include "Gaudi/Algorithm.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/MsgStream.h"
#include <Gaudi/Property.h>

class GAUDI_API IncidentProcAlg : public Gaudi::Algorithm {
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
  StatusCode execute( const EventContext& ctx ) const override;

private:
  SmartIF<IIncidentSvc> m_incSvc;
  /**************************
   ** Private Data Members **
   **************************/
};

#endif
