#ifndef IncidentRegistry_IncidentRegistry_H
#define IncidentRegistry_IncidentRegistry_H
// ============================================================================
// Include Files
// ============================================================================
// STD & STL
// ============================================================================
#include <map>
#include <list>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IIncidentRegistrySvc.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/ChronoEntity.h"
#include "GaudiKernel/EventContextHash.h"
// ============================================================================
// TBB
// ============================================================================
#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_unordered_map.h"
// ============================================================================
/**
 * @class IncidentRegistry
 * @brief Default implementation of the IIncidentRegistrySvc interface.
 *
 * Modified IncidentSvc to match multithreaded task based incident processing.
 * @author Sami Kama
 */

struct isSingleShot_t;

class IncidentRegistry : public extends<Service,
                                   IIncidentRegistrySvc>
{
    public:
  struct Listener final
  {
    IIncidentListener* iListener;
    long priority;
    bool rethrow;
    bool singleShot;

    Listener(IIncidentListener* il, long pri, bool thr=false, bool single=false):
      iListener(il), priority(pri), rethrow(thr), singleShot(single){}
  };
    private:

  // Typedefs
  typedef std::vector<Listener> ListenerList;
  typedef GaudiUtils::HashMap<Gaudi::StringKey, std::unique_ptr<ListenerList>> ListenerMap;

public:
  // Inherited Service overrides:
  //
  StatusCode initialize() override;
  StatusCode finalize() override;

  // IIncidentSvc interfaces overwrite
  //
  void addListener( IIncidentListener* lis       ,
                    const std::string& type = "" ,
                    long priority   = 0          ,
                    bool rethrow    = false      ,
                    bool singleShot = false      ) override;

  void removeListener( IIncidentListener* l, const std::string& type = "" ) override;
  void fireIncident( std::unique_ptr<Incident> incident,const EventContext* ctx) override;
  //TODO: return by value instead...
  void getListeners (std::vector<IIncidentListener*>& lis,
                     const std::string& type = "",
		     const EventContext *ctx = 0
		     ) const override;

  // Standard Constructor.
  IncidentRegistry( const std::string& name, ISvcLocator* svc );
  // Destructor.
  ~IncidentRegistry() override;

  IIncidentRegistrySvc::IncidentPack getNextIncident(const EventContext* ctx);

private:
  ListenerMap::iterator removeListenerFromList(ListenerMap::iterator, 
                                               IIncidentListener* item, 
                                               bool scheduleRemoval);
  // ==========================================================================
  /// Internal function to allow incidents listening to all events
  void i_fireIncident(const Incident& incident, const std::string& type);

  /// List of auditor names
  ListenerMap  m_listenerMap;

  /// Incident being fired. It is used to know if we can safely remove a listener or
  /// we have to schedule its removal for later.
  const std::string *m_currentIncidentType = nullptr;

  /// Mutex to synchronize access to m_listenerMap
  mutable std::recursive_mutex m_listenerMapMutex;

  /// timer & it's lock
  mutable ChronoEntity m_timer     ;
  mutable bool         m_timerLock = false ;
  // ==========================================================================
  typedef tbb::concurrent_queue<std::unique_ptr<Incident>> IncQueue_t;
  tbb::concurrent_unordered_map<EventContext,IncQueue_t,EventContextHash,EventContextHash> m_firedIncidents;
  
};
// ============================================================================
// The END
// ============================================================================
#endif
// ============================================================================
