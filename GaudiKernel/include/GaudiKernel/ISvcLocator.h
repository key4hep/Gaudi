/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_ISVCLOCATOR_H
#define GAUDIKERNEL_ISVCLOCATOR_H 1

// Include files
#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/ISvcManager.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/TypeNameString.h>

#include <list>
#include <string>

// Forward class declaration
class IService;
namespace Gaudi::Monitoring {
  struct Hub;
}

namespace Gaudi {
  namespace Interfaces {
    struct IOptionsSvc;
  }
} // namespace Gaudi
#define GAUDI_HAS_IOPTIONS_SVC

/** @class ISvcLocator ISvcLocator.h GaudiKernel/ISvcLocator.h
    The ISvcLocator is the interface implemented by the Service Factory in the
    Application Manager to locate services in the framework. Clients use this
    interface to locate references to interfaces of services existing in the
    application. This operation needs to be done before the service can be used
    by the client. Typically "locating the services" is done at the initialization
    phase of the clients.

    @author Pere Mato
*/
class GAUDI_API ISvcLocator : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( ISvcLocator, 3, 0 );

#if !defined( GAUDI_V22_API ) || defined( G22_NEW_SVCLOCATOR )
  /** Get a reference to the service given a service name
      @param name Service name
      @param svc Returned service pointer
  */
  [[deprecated( "use ISvcLocator::service<T>(type_name, createIf) -> SmartIF<T>" )]] virtual StatusCode
  getService( const Gaudi::Utils::TypeNameString& typeName, IService*& svc, const bool createIf = true ) {
    SmartIF<IService>& s = service( typeName, createIf );
    svc                  = s.get();
    if ( svc ) {
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
  [[deprecated( "use ISvcLocator::service<T>(type_name, createIf) -> SmartIF<T>" )]] virtual StatusCode
  getService( const Gaudi::Utils::TypeNameString& typeName, const InterfaceID& iid, IInterface*& pinterface ) {
    auto svc = service( typeName, false );
    return svc ? svc->queryInterface( iid, (void**)&pinterface ) : StatusCode::FAILURE;
  }

/** Get a reference to a service and create it if it does not exists
    @param name Service name
    @param svc Returned service pointer
    @param createIf flag to control the creation
*/
// virtual StatusCode getService( const Gaudi::Utils::TypeNameString& name,
//                               IService*& svc,
//                               bool createIf ) = 0;
#endif

  /// Return the list of Services
  virtual const std::list<IService*>& getServices() const = 0;

  /// Check the existence of a service given a service name
  virtual bool existsService( std::string_view name ) const = 0;

#if !defined( GAUDI_V22_API ) || defined( G22_NEW_SVCLOCATOR )
  /// Templated method to access a service by name.
  template <class T>
  [[deprecated( "use ISvcLocator::service<T>(type_name, createIf) -> SmartIF<T>" )]] StatusCode
  service( const Gaudi::Utils::TypeNameString& name, T*& svc, bool createIf = true ) {
    if ( createIf ) {
      IService*  s;
      StatusCode sc = getService( name, s, true );
      if ( !sc.isSuccess() ) return sc; // Must check if initialization was OK!
    }
    return getService( name, T::interfaceID(), (IInterface*&)svc );
  }

  /// Templated method to access a service by type and name.
  template <class T>
  [[deprecated( "use ISvcLocator::service<T>(type_name, createIf) -> SmartIF<T>" )]] StatusCode
  service( std::string_view type, std::string_view name, T*& svc, bool createIf = true ) {
    return service( std::string{ type }.append( "/" ).append( name ), svc, createIf );
  }
#endif

  /// Returns a smart pointer to a service.
  virtual SmartIF<IService>& service( const Gaudi::Utils::TypeNameString& typeName, const bool createIf = true ) = 0;

  /// Returns a smart pointer to the requested interface of a service.
  template <typename T>
  inline SmartIF<T> service( const Gaudi::Utils::TypeNameString& typeName, const bool createIf = true ) {
    return SmartIF<T>{ service( typeName, createIf ) };
  }

  // try to access a different interface of  the _current_ serviceLocator...
  template <typename IFace>
  SmartIF<IFace> as() {
    return SmartIF<IFace>{ this };
  }

  /// Direct access to Gaudi::Interfaces::IOptionsSvc implementation.
  Gaudi::Interfaces::IOptionsSvc& getOptsSvc();

  Gaudi::Monitoring::Hub& monitoringHub();
};

#endif // GAUDI_ISVCLOCATOR_H
