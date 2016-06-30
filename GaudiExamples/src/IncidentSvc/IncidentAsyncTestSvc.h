#ifndef GAUDIEXAMPLES_INCIDENTASYNCTESTSVC_H_
#define GAUDIEXAMPLES_INCIDENTASYNCTESTSVC_H_

#include <mutex>
#include "tbb/concurrent_unordered_map.h"
#include "IIncidentAsyncTestSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/EventContextHash.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/Service.h"
class ISvcLocator;
class IMessageSvc;
class IIncidentSvc;

/** @class IncidentRegistryTestListener IncidentListenerTest.h
 *
 */
class  IncidentAsyncTestSvc: public extends<Service,
					    IIncidentListener,
					    IIncidentAsyncTestSvc> {

public:
  /// Constructor
	using extends::extends;

  /// Destructor
  ~IncidentAsyncTestSvc() override = default;
  StatusCode initialize() override;
  StatusCode finalize() override;

  /// Reimplements from IIncidentListener
  virtual void handle(const Incident& incident) final;
  virtual void getData(uint64_t* data,EventContext* ctx=0) const final override;

private:
  PropertyWithValue<uint64_t>  m_fileOffset {this, "FileOffset",  100000000 };
  PropertyWithValue<uint64_t>  m_eventMultiplier {this, "EventMultiplier",  1000 };
  StringArrayProperty  m_incidentNames {this, "IncidentNames"};
  LongProperty  m_prio {this, "Priority",  0};
  SmartIF<IMessageSvc> m_msgSvc;
  SmartIF<IIncidentSvc> m_incSvc;
  tbb::concurrent_unordered_map<EventContext,uint64_t,EventContextHash,EventContextHash> m_ctxData;
  std::mutex m_eraseMutex;
};

#endif /*GAUDIEXAMPLES_INCIDENREGISTRYTESTLISTENER_H_*/
