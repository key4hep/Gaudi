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
#ifndef GAUDIKERNEL_ISVCMANAGER_H
#define GAUDIKERNEL_ISVCMANAGER_H

// Include files
#include <GaudiKernel/IComponentManager.h>
#include <GaudiKernel/IService.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/TypeNameString.h>
#include <string>

class ISvcLocator;

/** @class ISvcManager ISvcManager.h GaudiKernel/ISvcManager.h

    The ISvcManager is the interface implemented by the Service Factory in the
    Application Manager to support management functions. Clients use this
    interface to declare abstract service factories, and to create concrete
    instances of services.

    @author Pere Mato
*/
class GAUDI_API ISvcManager : virtual public IComponentManager {
public:
  /// InterfaceID
  DeclareInterfaceID( ISvcManager, 4, 0 );

  static const int DEFAULT_SVC_PRIORITY = 100;

  /** Add a service to the "active" list of services of the factory
   * @param svc Pointer to the service
   *
   * @return StatusCode indicating success or failure.
   */
  virtual StatusCode addService( IService* svc, int prio = DEFAULT_SVC_PRIORITY ) = 0;

  /** Add a service to the "active" list of services of the factory
   * @param svc Pointer to the service
   *
   * @return StatusCode indicating success or failure.
   */
  virtual StatusCode addService( const Gaudi::Utils::TypeNameString& nametype, int prio = DEFAULT_SVC_PRIORITY ) = 0;

  /** Remove a service from the "active" list of services of the factory
   * @param svc Pointer to the service
   *
   * @return StatusCode indicating success or failure.
   */
  virtual StatusCode removeService( IService* svc ) = 0;

  /** Remove a service from the "active" list of services of the factory
   * @param svc Pointer to the service
   *
   * @return StatusCode indicating success or failure.
   */
  virtual StatusCode removeService( std::string_view nam ) = 0;

  /** Declare the type of the service to be used when crating a given service name
   * @param svcname Service name
   * @param svctype Service type name
   *
   * @return StatusCode indicating success or failure.
   */
  virtual StatusCode declareSvcType( std::string svcname, std::string svctype ) = 0;

  /** Creates and instance of a service type that has been declared beforehand and
   * assigns it a name. It returns a pointer to an IService.
   * @param nametype   name/type of the service to create
   *
   * @return   SmartIF& to the created service.
   *
   * NOTE: as this returns a &, the underlying implementation
   *       must guarantee that once created, these SmartIF remain
   *       pinned in their location, thus constraining
   *       the underlying implementation (i.e. one cannot use
   *       something like std::vector<SmartIF<IService>>).
   *       If this interface had used value-semantics, and returned
   *       just plain SmartIF<IService> (i.e. WITHOUT the &) then
   *       the underlying implementation would have much more freedom)
   */
  virtual SmartIF<IService>& createService( const Gaudi::Utils::TypeNameString& nametype ) = 0;

  virtual int        getPriority( std::string_view name ) const    = 0;
  virtual StatusCode setPriority( std::string_view name, int pri ) = 0;

  /// Get the value of the initialization loop check flag.
  virtual bool loopCheckEnabled() const = 0;
  /// Set the value of the initialization loop check flag.
  virtual void setLoopCheckEnabled( bool en = true ) = 0;
};

#endif // GAUDIKERNEL_ISVCMANAGER_H
