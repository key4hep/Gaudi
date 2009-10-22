// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiSvc/src/IncidentSvc/IncidentSvc.h,v 1.7 2008/11/10 16:00:23 marcocle Exp $
#ifndef IncidentSvc_IncidentSvc_H
#define IncidentSvc_IncidentSvc_H

// Include Files
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IIncidentSvc.h"

#include <map>
#include <list>

#include <boost/thread/recursive_mutex.hpp>

// Forward declarations

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

class IncidentSvc : public extends1<Service, IIncidentSvc> {

public:

  struct Listener {
    IIncidentListener* iListener;
    long priority;
    bool rethrow;
    bool singleShot;

    Listener(IIncidentListener* il, long pri, bool thr=false, bool single=false):
      iListener(il), priority(pri), rethrow(thr), singleShot(single){}

  };

// Typedefs
  typedef std::list<Listener> ListenerList;
  typedef std::map<std::string, ListenerList*> ListenerMap;

// Inherited Service overrides:
//
  virtual StatusCode initialize();
  virtual StatusCode finalize();

// IIncidentSvc interfaces overwrite
//
  virtual void addListener(IIncidentListener* lis, const std::string& type = "",
                           long priority = 0, bool rethrow=false, bool singleShot=false);

  virtual void removeListener(IIncidentListener* lis, const std::string& type = "");
  virtual void fireIncident( const Incident& incident );

  // Standard Constructor.
  IncidentSvc( const std::string& name, ISvcLocator* svc );
  // Destructor.
  virtual ~IncidentSvc();

private:
  /// Internal function to allow incidents listening to all events
  void i_fireIncident(const Incident& incident, const std::string& type);

  /// List of auditor names
  ListenerMap  m_listenerMap;

  /// Incident being fired. It is used to know if we can safely remove a listener or
  /// we have to schedule its removal for later.
  const std::string *m_currentIncidentType;

  /// Internal MsgStream
  MsgStream m_log;

  /// Mutex to synchronize access to m_listenerMap
  boost::recursive_mutex m_listenerMapMutex;
};

#endif


