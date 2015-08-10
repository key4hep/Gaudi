#ifndef GAUDISVC_ServiceManager_H
#define GAUDISVC_ServiceManager_H

// Include files
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IStateful.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ComponentManager.h"
#include "GaudiKernel/Map.h"
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <mutex>

// Forward declarations
class IService;
class IMessageSvc;
class Property;

/** @class ServiceManager ServiceManager.h

    The ServiceManager class is in charge of the creation of concrete
    instances of Services. The ApplicationMgr delegates the creation and
    bookkeeping of services to the ServiceManager. In order to
    be able to create services from which it is not know the concrete
    type it requires that the services has been declared in one of
    3 possible ways: an abstract static creator function, a dynamic
    link library or an abstract factory reference.

    @author Pere Mato
*/
class ServiceManager : public extends2<ComponentManager, ISvcManager, ISvcLocator>{
public:

  struct ServiceItem final {
    ServiceItem(IService *s, long p = 0, bool act = false):
      service(s), priority(p), active(act) {}
    SmartIF<IService> service;
    long priority;
    bool active;
    inline bool operator==(const std::string &name) const {
      return service->name() == name;
    }
    inline bool operator==(const IService *ptr) const {
      return service.get() == ptr;
    }
    inline bool operator<(const ServiceItem& rhs) const {
      return priority < rhs.priority;
    }
  };

  // typedefs and classes
  typedef std::list<ServiceItem> ListSvc;
  typedef GaudiUtils::Map<std::string, std::string> MapType;

  /// default creator
  ServiceManager(IInterface* application);

  /// Function needed by CommonMessaging
  inline SmartIF<ISvcLocator>& serviceLocator() const {
    return m_svcLocator;
  }

  /// virtual destructor
  virtual ~ServiceManager();

  /// Return the list of Services
  virtual const std::list<IService*>& getServices() const;

  /// implementation of ISvcLocation::existsService
  virtual bool existsService(const std::string& name) const;

  /// implementation of ISvcManager::addService
  virtual StatusCode addService(IService* svc, int prio = DEFAULT_SVC_PRIORITY);
  /// implementation of ISvcManager::addService
  virtual StatusCode addService(const Gaudi::Utils::TypeNameString& typeName, int prio = DEFAULT_SVC_PRIORITY);
  /// implementation of ISvcManager::removeService
  virtual StatusCode removeService(IService* svc);
  /// implementation of ISvcManager::removeService
  virtual StatusCode removeService(const std::string& name);

  /// implementation of ISvcManager::declareSvcType
  virtual StatusCode declareSvcType(const std::string& svcname, const std::string& svctype);

  /// implementation of ISvcManager::createService 
  /// NOTE: as this returns a &, we must guarantee 
  ///       that once created, these SmartIF remain 
  ///       pinned in their location, thus constraining
  ///       the underlying implementation...
  virtual SmartIF<IService>& createService(const Gaudi::Utils::TypeNameString& nametype);

  /// Initialization (from CONFIGURED to INITIALIZED).
  virtual StatusCode initialize();
  /// Start (from INITIALIZED to RUNNING).
  virtual StatusCode start();
  /// Stop (from RUNNING to INITIALIZED).
  virtual StatusCode stop();
  /// Finalize (from INITIALIZED to CONFIGURED).
  virtual StatusCode finalize();

  /// Initialization (from INITIALIZED or RUNNING to INITIALIZED, via CONFIGURED).
  virtual StatusCode reinitialize();
  /// Initialization (from RUNNING to RUNNING, via INITIALIZED).
  virtual StatusCode restart();

  /// manage priorities of services
  virtual int getPriority(const std::string& name) const;
  virtual StatusCode setPriority(const std::string& name, int pri);

  /// Get the value of the initialization loop check flag.
  virtual bool loopCheckEnabled() const;
  /// Set the value of the initialization loop check flag.
  virtual void setLoopCheckEnabled(bool en);

  /// Return the name of the manager (implementation of INamedInterface)
  const std::string &name() const {
    static std::string _name = "ServiceManager";
    return _name;
  }

  /// Returns a smart pointer to a service.
  virtual SmartIF<IService> &service(const Gaudi::Utils::TypeNameString &typeName, const bool createIf = true);

#if !defined(GAUDI_V22_API) || defined(G22_NEW_SVCLOCATOR)
  using ISvcManager::createService;
  using ISvcManager::addService;
#endif

private:

  inline ListSvc::iterator find(const std::string &name) {
    return std::find(m_listsvc.begin(), m_listsvc.end(), name);
  }
  inline ListSvc::const_iterator find(const std::string &name) const {
    return std::find(m_listsvc.begin(), m_listsvc.end(), name);
  }
  inline ListSvc::iterator find(const IService *ptr) {
    return std::find(m_listsvc.begin(), m_listsvc.end(), ptr);
  }
  inline ListSvc::const_iterator find(const IService *ptr) const {
    return std::find(m_listsvc.begin(), m_listsvc.end(), ptr);
  }

private:
  ListSvc       m_listsvc;     ///< List of service maintained by ServiceManager
                               ///  This contains SmartIF<T> for all services -- 
                               ///  and because there can be SmartIF<T>& 'out there' that
                               ///  refer to these specific SmarIF<T>, we 
                               ///  *unfortunately* must guarantee that they _never_ move 
                               ///  after creation. Hence, we cannot use a plain std::vector
                               ///  here, as that may cause relocation and/or swapping of 
                               ///  SmartIF<T>'s, and then the already handed out references
                               ///  may refer to the wrong item.... Note that we could use
                               ///  an std::vector<std::unique_ptr<ServiceItem>> (sometimes known
                               ///  as 'stable vector') as then the individual ServiceItems 
                               ///  would stay pinned in their original location, but that 
                               ///  would put ServiceItem on the heap...
                               ///  And maybe I'm way too paranoid...
  MapType       m_maptype;     ///< Map of service name and service type
  bool          m_loopCheck = true;   ///< Check for service initialization loops

  /// Pointer to the application IService interface.
  SmartIF<IService> m_appSvc;

  /// List of pointers to the know services used to implement getServices()
  mutable std::list<IService*> m_listOfPtrs;

  GaudiUtils::Map<InterfaceID, SmartIF<IInterface> > m_defaultImplementations;

  /// Mutex to synchronize shared service initialization between threads
  std::recursive_mutex  m_svcinitmutex;

private:
  void dump() const;

};
#endif  // GAUDISVC_ServiceManager_H
