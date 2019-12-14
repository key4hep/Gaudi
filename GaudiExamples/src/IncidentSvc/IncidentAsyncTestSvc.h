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
#ifndef GAUDIEXAMPLES_INCIDENTASYNCTESTSVC_H_
#define GAUDIEXAMPLES_INCIDENTASYNCTESTSVC_H_

#include "GaudiKernel/EventContextHash.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SmartIF.h"
#include "IIncidentAsyncTestSvc.h"
#include "tbb/concurrent_unordered_map.h"
#include <mutex>
class ISvcLocator;
class IMessageSvc;
class IIncidentSvc;

/** @class IncidentRegistryTestListener IncidentListenerTest.h
 *
 */
class IncidentAsyncTestSvc final : public extends<Service, IIncidentListener, IIncidentAsyncTestSvc> {

public:
  /// Constructor
  using extends::extends;

  StatusCode initialize() override;
  StatusCode finalize() override;

  /// Reimplements from IIncidentListener
  void handle( const Incident& incident ) override;
  void getData( uint64_t* data, EventContext* ctx = 0 ) const override;

private:
  Gaudi::Property<uint64_t>                 m_fileOffset{this, "FileOffset", 100000000};
  Gaudi::Property<uint64_t>                 m_eventMultiplier{this, "EventMultiplier", 1000};
  Gaudi::Property<std::vector<std::string>> m_incidentNames{this, "IncidentNames"};
  Gaudi::Property<long>                     m_prio{this, "Priority", 0};
  SmartIF<IMessageSvc>                      m_msgSvc;
  SmartIF<IIncidentSvc>                     m_incSvc;
  tbb::concurrent_unordered_map<EventContext, uint64_t, EventContextHash, EventContextHash> m_ctxData;
  std::mutex                                                                                m_eraseMutex;
};

#endif /*GAUDIEXAMPLES_INCIDENREGISTRYTESTLISTENER_H_*/
