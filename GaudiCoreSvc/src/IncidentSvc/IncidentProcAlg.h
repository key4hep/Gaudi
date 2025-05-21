/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <Gaudi/Algorithm.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/MsgStream.h>

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
