/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef IncidentSvc_IncidentSvc_H
#define IncidentSvc_IncidentSvc_H
// ============================================================================
// Include Files
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
#include <list>
#include <map>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ChronoEntity.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/StringKey.h"
// ============================================================================
// TBB
// ============================================================================
#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_unordered_map.h"
// ============================================================================
/**
 * @class IncidentSvc
 * @brief Default implementation of the IIncidentSvc interface.
 *
 * This implementation is thread-safe with the following features:
 *  - Calls to fireIncident(), addListener() and removeListener() are
 *    synchronized across threads.
 *  - Calls to IIncidentListener::handle() are serialized, i.e. at any time
 *    there is at most one incident handler being executed across all threads.
 */

struct isSingleShot_t;

class IncidentSvc : public extends<Service, IIncidentSvc> {
private:
  // Typedefs
  // ListenerList should be a list rather than a vector because handing
  // a fired incident may result in a call to addListener.
  typedef std::list<IIncidentSvc::Listener>                                    ListenerList;
  typedef GaudiUtils::HashMap<Gaudi::StringKey, std::unique_ptr<ListenerList>> ListenerMap;

public:
  // Inherited Service overrides:
  StatusCode finalize() override;

  // IIncidentSvc interfaces overwrite
  //
  void addListener( IIncidentListener* lis, const std::string& type = "", long priority = 0, bool rethrow = false,
                    bool singleShot = false ) override;

  void removeListener( IIncidentListener* l, const std::string& type = "" ) override;
  void fireIncident( const Incident& incident ) override;
  void fireIncident( std::unique_ptr<Incident> incident ) override;
  // TODO: return by value instead...
  void getListeners( std::vector<IIncidentListener*>& lis, const std::string& type = "" ) const override;

  // Standard Constructor.
  IncidentSvc( const std::string& name, ISvcLocator* svc );
  // Destructor.
  ~IncidentSvc() override;
  IIncidentSvc::IncidentPack getIncidents( const EventContext* ctx ) override;

private:
  ListenerMap::iterator removeListenerFromList( ListenerMap::iterator, IIncidentListener* item, bool scheduleRemoval );
  // ==========================================================================
  /// Internal function to allow incidents listening to all events
  void i_fireIncident( const Incident& incident, const std::string& type );

  /// List of auditor names
  ListenerMap m_listenerMap;

  /// Incident being fired. It is used to know if we can safely remove a listener or
  /// we have to schedule its removal for later.
  const std::string* m_currentIncidentType = nullptr;

  /// Mutex to synchronize access to m_listenerMap
  mutable std::recursive_mutex m_listenerMapMutex;

  /// timer & it's lock
  mutable ChronoEntity m_timer;
  mutable bool         m_timerLock = false;
  // ==========================================================================
  // When TBB supports unique_ptrs in concurrent queue typedef should be changed
  // typedef tbb::concurrent_queue<std::unique_ptr<Incident>> IncQueue_t;
  typedef tbb::concurrent_queue<std::unique_ptr<Incident>>             IncQueue_t;
  tbb::concurrent_unordered_map<EventContext::ContextID_t, IncQueue_t> m_firedIncidents;

  /// Event ID for each slot
  tbb::concurrent_unordered_map<EventContext::ContextID_t, EventContext::ContextEvt_t> m_slotEvent;
};
// ============================================================================
// The END
// ============================================================================
#endif
