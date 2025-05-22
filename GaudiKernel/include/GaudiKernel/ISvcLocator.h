/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/IInterface.h>
#include <GaudiKernel/ISvcManager.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/TypeNameString.h>
#include <list>
#include <string>

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

  /// Return the list of Services
  virtual const std::list<IService*>& getServices() const = 0;

  /// Check the existence of a service given a service name
  virtual bool existsService( std::string_view name ) const = 0;

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
