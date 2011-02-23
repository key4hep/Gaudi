// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/GaudiKernel/ISvcLocator.h,v 1.15 2006/12/14 12:55:44 hmd Exp $
#ifndef GAUDIKERNEL_ISVCLOCATOR_H
#define GAUDIKERNEL_ISVCLOCATOR_H 1

// Include files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/SmartIF.h"
#include <string>
#include <list>

// Forward class declaration
class IService;

/** @class ISvcLocator ISvcLocator.h GaudiKernel/ISvcLocator.h
    The ISvcLocator is the interface implemented by the Service Factory in the
    Application Manager to locate services in the framework. Clients use this
    interface to locate references to interfaces of services existing in the
    application. This operation needs to be done before the service can be used
    by the client. Typically "locating the services" is done at the initialization
    phase of the clients.

    @author Pere Mato
*/
class GAUDI_API ISvcLocator: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(ISvcLocator,3,0);

#if !defined(GAUDI_V22_API) || defined(G22_NEW_SVCLOCATOR)
  /** Get a reference to the service given a service name
      @param name Service name
      @param svc Returned service pointer
  */
  virtual StatusCode getService( const Gaudi::Utils::TypeNameString& typeName,
                                 IService*&  svc, const bool createIf = true) {
    SmartIF<IService> &s = service(typeName, createIf);
    svc = s.get();
    if (svc) {
      svc->addRef(); // Needed to maintain the correct reference counting.
      return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
  }

  /** Get a specific interface pointer given a service name and interface id
      @param name Service name
      @param iid Interface ID
      @param pinterface Returned pointer to the requested interface
  */
  virtual StatusCode getService( const Gaudi::Utils::TypeNameString& typeName,
                                 const InterfaceID& iid,
                                 IInterface*& pinterface ) {
    SmartIF<IService> svc = service(typeName, false);
    if (svc.isValid()) {
      // Service found. So now get the right interface
      return svc->queryInterface(iid, (void**)&pinterface);
    }
    return StatusCode::FAILURE;
  }

  /** Get a reference to a service and create it if it does not exists
      @param name Service name
      @param svc Returned service pointer
      @param createIf flag to control the creation
  */
  //virtual StatusCode getService( const Gaudi::Utils::TypeNameString& name,
  //                               IService*& svc,
  //                               bool createIf ) = 0;
#endif

  /// Return the list of Services
  virtual const std::list<IService*> &getServices() const = 0;

  /// Check the existence of a service given a service name
  virtual bool existsService(const std::string& name) const = 0;

#if !defined(GAUDI_V22_API) || defined(G22_NEW_SVCLOCATOR)
  /// Templated method to access a service by name.
  template <class T>
  StatusCode service( const Gaudi::Utils::TypeNameString& name, T*& svc, bool createIf = true ) {
    if( createIf ) {
      IService* s;
      StatusCode sc = getService( name, s, true);
      if ( !sc.isSuccess() ) return sc;  // Must check if initialization was OK!
    }
    return getService( name, T::interfaceID(), (IInterface*&)svc );
  }

  /// Templated method to access a service by type and name.
  template <class T>
  StatusCode service( const std::string& type, const std::string& name,
		      T*& svc, bool createIf = true ) {
    return service(type + "/" + name, svc, createIf);
  }
#endif

  /// Returns a smart pointer to a service.
  virtual SmartIF<IService> &service(const Gaudi::Utils::TypeNameString &typeName, const bool createIf = true) = 0;

  /// Returns a smart pointer to the requested interface of a service.
  template <typename T>
  inline SmartIF<T> service(const Gaudi::Utils::TypeNameString &typeName, const bool createIf = true) {
    return SmartIF<T>(service(typeName, createIf));
  }

};


#endif  // GAUDI_ISVCLOCATOR_H
