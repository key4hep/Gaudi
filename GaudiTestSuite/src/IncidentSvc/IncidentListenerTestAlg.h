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

#include <GaudiKernel/Algorithm.h>

#include <memory>

class IIncidentSvc;
class IncidentListenerTest;

class IncidentListenerTestAlg : public Algorithm {
public:
  using Algorithm::Algorithm;

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

  static std::string& incident();

private:
  static std::string                                   s_incidentType;
  SmartIF<IIncidentSvc>                                m_incSvc;
  std::array<std::unique_ptr<IncidentListenerTest>, 6> m_listener;
};
