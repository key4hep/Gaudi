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
#ifndef GAUDIEXAMPLES_INCIDENTASYNCTESTALG_H_
#define GAUDIEXAMPLES_INCIDENTASYNCTESTALG_H_

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/DataObjectHandle.h"
#include <memory>

class IIncidentSvc;
class IncidentListener;
class IIncidentAsyncTestSvc;

class IncidentAsyncTestAlg : public Algorithm {
public:
  using Algorithm::Algorithm;

  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

  bool isClonable() const override { return true; }

  static std::string& incident();

private:
  Gaudi::Property<std::string>                               m_serviceName{this, "ServiceName", "IncTestSvc"};
  Gaudi::Property<std::vector<std::string>>                  m_inpKeys{this, "inpKeys"};
  Gaudi::Property<std::vector<std::string>>                  m_outKeys{this, "outKeys"};
  SmartIF<IIncidentAsyncTestSvc>                             m_service;
  std::vector<std::unique_ptr<DataObjectHandle<DataObject>>> m_inputObjHandles;
  std::vector<std::unique_ptr<DataObjectHandle<DataObject>>> m_outputObjHandles;
};

#endif /*GAUDIEXAMPLES_INCIDENTREGISTRYTESTALG_H_*/
