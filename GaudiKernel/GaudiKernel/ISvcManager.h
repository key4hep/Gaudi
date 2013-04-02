// $Id: ISvcManager.h,v 1.7 2008/11/10 15:29:09 marcocle Exp $
#ifndef GAUDIKERNEL_ISVCMANAGER_H
#define GAUDIKERNEL_ISVCMANAGER_H

// Include files
#include "GaudiKernel/IComponentManager.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/TypeNameString.h"
#include <string>

// Forward class declaration
#if defined(GAUDI_V20_COMPAT) || (!defined(GAUDI_V22_API) || defined(G22_NEW_SVCLOCATOR))
class ISvcFactory;
#include "GaudiKernel/IService.h"
#else
class IService;
#endif
class ISvcLocator;

/** @class ISvcManager ISvcManager.h GaudiKernel/ISvcManager.h

    The ISvcManager is the interface implemented by the Service Factory in the
    Application Manager to support management functions. Clients use this
    interface to declare abstract service factories, and to create concrete
    instances of services.

    @author Pere Mato
*/
class GAUDI_API ISvcManager: virtual public IComponentManager {
public:
  /// InterfaceID
  DeclareInterfaceID(ISvcManager,4,0);

  /** Add a service to the "active" list of services of the factory
    * @param svc Pointer to the service
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode addService( IService* svc, int prio = 100) = 0;

#if !defined(GAUDI_V22_API) || defined(G22_NEW_SVCLOCATOR)
  /** Add a service to the "active" list of services of the factory
    * @param svc Pointer to the service
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode addService( const std::string& typ, const std::string& nam, int prio ){
    return addService(Gaudi::Utils::TypeNameString(nam,typ), prio);
  }
#endif

  /** Add a service to the "active" list of services of the factory
    * @param svc Pointer to the service
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode addService( const Gaudi::Utils::TypeNameString& nametype, int prio = 100) = 0;

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
  virtual StatusCode removeService( const std::string& nam ) = 0;

#if !defined(GAUDI_V22_API) || defined(G22_NEW_SVCLOCATOR)
  /** Declare an abstract factory for a given service type
    * @param factory Abstract factory reference
    * @param svctype Service type name
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode declareSvcFactory( const ISvcFactory& /*factory*/,
                                        const std::string& /*svctype*/ ) {
    // This function is never used.
    return StatusCode::FAILURE;
  }
#endif

  /** Declare the type of the service to be used when crating a given service name
    * @param svcname Service name
    * @param svctype Service type name
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode declareSvcType( const std::string& svcname,
                                     const std::string& svctype ) = 0;

  /** Creates and instance of a service type that has been declared beforehand and
    * assigns it a name. It returns a pointer to an IService.
    * @param nametype   name/type of the service to create
    *
    * @return   SmartIF to the created service.
    */
  virtual SmartIF<IService>& createService(const Gaudi::Utils::TypeNameString& nametype) = 0;

#if !defined(GAUDI_V22_API) || defined(G22_NEW_SVCLOCATOR)
  /** Creates and instance of a service type that has been declared beforehand and
    * assigns it a name. It returns a pointer to an IService.
    * @param svctype Service type name
    * @param svcname Service name to be set
    * @param svc Returned service pointer
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode createService( const std::string& svctype,
                                    const std::string& svcname,
                                    IService*& svc ){
    SmartIF<IService> s = createService(svctype + "/" + svcname);
    svc = s.get();
    if (svc) {
      svc->addRef(); // Needed to maintain the correct reference counting.
      return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
  }

  /** Access to service factory by name to create unmanaged services
    * @param  svc_type    [IN]      Name of the service type
    * @param  fac         [OUT]     Reference to store pointer to service factory
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode getFactory(    const std::string& /*svc_type*/,
                                    const ISvcFactory*& /*fac*/) const {
    // This function is never used.
    return StatusCode::FAILURE;
  }

  /** Initializes the list of "active" services
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode initializeServices() { return initialize(); }

  /** Starts the list of "active" services
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode startServices() { return start(); }

  /** Stops the list of "active" services
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode stopServices() { return stop(); }

  /** Finalizes the list of "active" services
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode finalizeServices() { return finalize(); }

  /** Reinitializes the list of "active" services
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode reinitializeServices() { return reinitialize(); }

  /** Restarts the list of "active" services
    *
    * @return StatusCode indicating success or failure.
    */
  virtual StatusCode restartServices() { return restart(); }
#endif

  virtual int getPriority(const std::string& name) const = 0;
  virtual StatusCode setPriority(const std::string& name, int pri) = 0;

  /// Get the value of the initialization loop check flag.
  virtual bool loopCheckEnabled() const = 0;
  /// Set the value of the initialization loop check flag.
  virtual void setLoopCheckEnabled(bool en = true) = 0;

};

#endif  // GAUDIKERNEL_ISVCMANAGER_H

