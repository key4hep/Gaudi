#ifndef IncidentSvc_IncidentSvc_H
#define IncidentSvc_IncidentSvc_H
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
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/ChronoEntity.h"
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

class IncidentSvc : public extends1<Service, IIncidentSvc>
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
  void fireIncident( const Incident& incident) override;
  //TODO: return by value instead...
  void getListeners (std::vector<IIncidentListener*>& lis,
                     const std::string& type = "") const override;

  // Standard Constructor.
  IncidentSvc( const std::string& name, ISvcLocator* svc );
  // Destructor.
  ~IncidentSvc() override;

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

};
// ============================================================================
// The END
// ============================================================================
#endif
// ============================================================================
