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

// Include files
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/ServiceHandle.h>

class TH1F;
class ITHistSvc;
class TDirectory;

class THistWrite : public Algorithm {

public:
  // Constructor of this form must be provided
  THistWrite( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

private:
  ServiceHandle<ITHistSvc> m_ths{ this, "HistSvc", "THistSvc" };

  void listKeys( TDirectory* td );
};
