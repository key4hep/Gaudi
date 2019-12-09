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
#ifndef GAUDISVC_ServiceManager_H
#define GAUDISVC_ServiceManager_H

// Include files
#include "GaudiKernel/ComponentManager.h"
#include "GaudiKernel/IStateful.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/Map.h"
#include "GaudiKernel/SmartIF.h"

#include "boost/thread.hpp"
#include <algorithm>
#include <list>
#include <memory>
#include <string>
#include <vector>

// Forward declarations
class IService;
class IMessageSvc;

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
class ServiceManager : public extends<ComponentManager, ISvcManager, ISvcLocator> {
public:
  struct ServiceItem final {
    ServiceItem( IService* s, long p = 0, bool act = false ) : service( s ), priority( p ), active( act ) {}
    SmartIF<IService> service;
    long              priority;
    bool              active;
    bool       operator==( std::string_view name ) const { return service->name() == name; }
    bool       operator==( const IService* ptr ) const { return service.get() == ptr; }
    bool       operator<( const ServiceItem& rhs ) const { return priority < rhs.priority; }
  };

  // typedefs and classes
  typedef std::list<ServiceItem>                    ListSvc;
  typedef GaudiUtils::Map<std::string, std::string> MapType;

  /// default creator
  ServiceManager( IInterface* application );

  /// Function needed by CommonMessaging
  inline SmartIF<ISvcLocator>& serviceLocator() const override { return m_svcLocator; }

  /// virtual destructor
  ~ServiceManager() override;

  /// Return the list of Services
  const std::list<IService*>& getServices() const override;

  /// implementation of ISvcLocation::existsService
  bool existsService( std::string_view name ) const override;

  /// implementation of ISvcManager::addService
  StatusCode addService( IService* svc, int prio = DEFAULT_SVC_PRIORITY ) override;
  /// implementation of ISvcManager::addService
  StatusCode addService( const Gaudi::Utils::TypeNameString& typeName, int prio = DEFAULT_SVC_PRIORITY ) override;
  /// implementation of ISvcManager::removeService
  StatusCode removeService( IService* svc ) override;
  /// implementation of ISvcManager::removeService
  StatusCode removeService( std::string_view name ) override;

  /// implementation of ISvcManager::declareSvcType
  StatusCode declareSvcType( const std::string& svcname, const std::string& svctype ) override;

  /// implementation of ISvcManager::createService
  /// NOTE: as this returns a &, we must guarantee
  ///       that once created, these SmartIF remain
  ///       pinned in their location, thus constraining
  ///       the underlying implementation...
  SmartIF<IService>& createService( const Gaudi::Utils::TypeNameString& nametype ) override;

  /// Initialization (from CONFIGURED to INITIALIZED).
  StatusCode initialize() override;
  /// Start (from INITIALIZED to RUNNING).
  StatusCode start() override;
  /// Stop (from RUNNING to INITIALIZED).
  StatusCode stop() override;
  /// Finalize (from INITIALIZED to CONFIGURED).
  StatusCode finalize() override;

  /// Initialization (from INITIALIZED or RUNNING to INITIALIZED, via CONFIGURED).
  StatusCode reinitialize() override;
  /// Initialization (from RUNNING to RUNNING, via INITIALIZED).
  StatusCode restart() override;

  /// manage priorities of services
  int        getPriority( const std::string& name ) const override;
  StatusCode setPriority( const std::string& name, int pri ) override;

  /// Get the value of the initialization loop check flag.
  bool loopCheckEnabled() const override;
  /// Set the value of the initialization loop check flag.
  void setLoopCheckEnabled( bool en ) override;

  /// Return the name of the manager (implementation of INamedInterface)
  const std::string& name() const override {
    static std::string _name = "ServiceManager";
    return _name;
  }

  /// Returns a smart pointer to a service.
  SmartIF<IService>& service( const Gaudi::Utils::TypeNameString& typeName, const bool createIf = true ) override;

  /// Returns a smart pointer to the requested interface of a service.
  template <typename T>
  inline SmartIF<T> service( const Gaudi::Utils::TypeNameString& typeName, const bool createIf = true ) {
    return SmartIF<T>{service( typeName, createIf )};
  }

#if !defined( GAUDI_V22_API ) || defined( G22_NEW_SVCLOCATOR )
  using ISvcManager::addService;
  using ISvcManager::createService;
#endif

  /// Function to call to update the outputLevel of the components (after a change in MessageSvc).
  void outputLevelUpdate() override;

private:
  ListSvc::iterator find( std::string_view name ) {
    boost::lock_guard<boost::recursive_mutex> lck( m_gLock );
    return std::find( m_listsvc.begin(), m_listsvc.end(), name );
  }
  ListSvc::const_iterator find( std::string_view name ) const {
    boost::lock_guard<boost::recursive_mutex> lck( m_gLock );
    return std::find( m_listsvc.begin(), m_listsvc.end(), name );
  }
  ListSvc::iterator find( const IService* ptr ) {
    boost::lock_guard<boost::recursive_mutex> lck( m_gLock );
    return std::find( m_listsvc.begin(), m_listsvc.end(), ptr );
  }
  ListSvc::const_iterator find( const IService* ptr ) const {
    boost::lock_guard<boost::recursive_mutex> lck( m_gLock );
    return std::find( m_listsvc.begin(), m_listsvc.end(), ptr );
  }

private:
  ListSvc m_listsvc;          ///< List of service maintained by ServiceManager
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
  MapType m_maptype;          ///< Map of service name and service type
  bool    m_loopCheck = true; ///< Check for service initialization loops

  /// Pointer to the application IService interface.
  SmartIF<IService> m_appSvc;

  /// List of pointers to the know services used to implement getServices()
  mutable std::list<IService*> m_listOfPtrs;

  GaudiUtils::Map<InterfaceID, SmartIF<IInterface>> m_defaultImplementations;

  /// Mutex to synchronize shared service initialization between threads
  typedef boost::recursive_mutex     Mutex_t;
  typedef boost::lock_guard<Mutex_t> LockGuard_t;

  mutable Mutex_t                        m_gLock;
  mutable std::map<std::string, Mutex_t> m_lockMap;

private:
  void dump() const;
};
#endif // GAUDISVC_ServiceManager_H
