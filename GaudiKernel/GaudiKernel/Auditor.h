#ifndef GAUDIKERNEL_AUDITOR_H
#define GAUDIKERNEL_AUDITOR_H

// Include files
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/IAuditor.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h" /*used by service(..)*/
#include "GaudiKernel/PropertyFwd.h"
#include "GaudiKernel/PropertyHolder.h"
#include <Gaudi/PluginService.h>
#include <string>
#include <vector>

// Forward declarations
class IService;
class IMessageSvc;
class Algorithm;

/** @class Auditor Auditor.h GaudiKernel/Auditor.h

    Base class from which all concrete auditor classes should be derived.
    The only base class functionality which may be used in the
    constructor of a concrete auditor is the declaration of
    member variables as properties. All other functionality,
    i.e. the use of services, may be used only in
    initialize() and afterwards.

    @author David Quarrie
    @date   2000
    @author Marco Clemencic
    @date   2008-03
*/
class GAUDI_API Auditor : public PropertyHolder<CommonMessaging<implements<IAuditor, IProperty>>>
{
public:
#ifndef __REFLEX__
  typedef Gaudi::PluginService::Factory<IAuditor*, const std::string&, ISvcLocator*> Factory;
#endif

  /** Constructor
      @param name    The algorithm object's name
      @param svcloc  A pointer to a service location service */
  Auditor( const std::string& name, ISvcLocator* svcloc );

  /// Destructor
  ~Auditor() override = default;

  Auditor( const Auditor& a ) = delete;
  Auditor& operator=( const Auditor& rhs ) = delete;

  /** Initialization method invoked by the framework. This method is responsible
      for any bookkeeping of initialization required by the framework itself.
  */
  StatusCode sysInitialize() override;
  /** Finalization method invoked by the framework. This method is responsible
      for any bookkeeping of initialization required by the framework itself.
  */
  StatusCode sysFinalize() override;

  /// The following methods are meant to be implemented by the child class...

  void before( StandardEventType, INamedInterface* ) override;
  void before( StandardEventType, const std::string& ) override;

  void before( CustomEventTypeRef, INamedInterface* ) override;
  void before( CustomEventTypeRef, const std::string& ) override;

  void after( StandardEventType, INamedInterface*, const StatusCode& ) override;
  void after( StandardEventType, const std::string&, const StatusCode& ) override;

  void after( CustomEventTypeRef, INamedInterface*, const StatusCode& ) override;
  void after( CustomEventTypeRef, const std::string&, const StatusCode& ) override;

  // Obsolete methods

  void beforeInitialize( INamedInterface* ) override;
  void afterInitialize( INamedInterface* ) override;

  void beforeReinitialize( INamedInterface* ) override;
  void afterReinitialize( INamedInterface* ) override;

  void beforeExecute( INamedInterface* ) override;
  void afterExecute( INamedInterface*, const StatusCode& ) override;

  void beforeFinalize( INamedInterface* ) override;
  void afterFinalize( INamedInterface* ) override;

  void beforeBeginRun( INamedInterface* ) override;
  void afterBeginRun( INamedInterface* ) override;

  void beforeEndRun( INamedInterface* ) override;
  void afterEndRun( INamedInterface* ) override;

  virtual StatusCode initialize();
  virtual StatusCode finalize();

  const std::string& name() const override;

  bool isEnabled() const override;

  /** The standard service locator. Returns a pointer to the service locator service.
      This service may be used by an auditor to request any services it requires in
      addition to those provided by default.
  */
  SmartIF<ISvcLocator>& serviceLocator() const;

  /** Access a service by name, creating it if it doesn't already exist.
  */
  template <class T>
  StatusCode service( const std::string& name, T*& svc, bool createIf = false ) const
  {
    auto ptr = serviceLocator()->service<T>( name, createIf );
    if ( ptr ) {
      svc = ptr.get();
      svc->addRef();
      return StatusCode::SUCCESS;
    }
    svc = nullptr;
    return StatusCode::FAILURE;
  }

  template <class T = IService>
  SmartIF<T> service( const std::string& name, bool createIf = false ) const
  {
    return serviceLocator()->service<T>( name, createIf );
  }

  /** Set the auditor's properties. This method requests the job options service
      to set the values of any declared properties. The method is invoked from
      within sysInitialize() by the framework and does not need to be explicitly
      called by a concrete auditor.
  */
  StatusCode setProperties();

private:
  std::string m_name; ///< Auditor's name for identification

  mutable SmartIF<ISvcLocator> m_pSvcLocator; ///< Pointer to service locator service

  Gaudi::Property<int> m_outputLevel{this, "OutputLevel", MSG::NIL, "output level"};
  Gaudi::Property<bool> m_isEnabled{this, "Enable", true, "should the auditor be used or not"};

  bool m_isInitialized = false; ///< Auditor has been initialized flag
  bool m_isFinalized   = false; ///< Auditor has been finalized flag
};

#ifndef GAUDI_NEW_PLUGIN_SERVICE
template <class T>
class AudFactory
{
public:
#ifndef __REFLEX__
  template <typename S, typename... Args>
  static typename S::ReturnType create( Args&&... args )
  {
    return new T( std::forward<Args>( args )... );
  }
#endif
};

// Macros to declare component factories
#define DECLARE_AUDITOR_FACTORY( x ) DECLARE_FACTORY_WITH_CREATOR( x, AudFactory<x>, Auditor::Factory )
#define DECLARE_NAMESPACE_AUDITOR_FACTORY( n, x ) DECLARE_AUDITOR_FACTORY( n::x )

#else

// macros to declare factories
#define DECLARE_AUDITOR_FACTORY( x ) DECLARE_COMPONENT( x )
#define DECLARE_NAMESPACE_AUDITOR_FACTORY( n, x ) DECLARE_COMPONENT( n::x )

#endif

#endif // GAUDIKERNEL_AUDITOR_H
