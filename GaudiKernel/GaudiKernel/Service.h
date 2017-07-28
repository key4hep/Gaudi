#ifndef GAUDIKERNEL_SERVICE_H
#define GAUDIKERNEL_SERVICE_H
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/IStateful.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/PropertyHolder.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ToolHandle.h"
#include <Gaudi/PluginService.h>

// ============================================================================
#include <mutex>
#include <vector>
// ============================================================================
// Forward declarations
// ============================================================================
class IMessageSvc;
class ISvcManager;
class ServiceManager;
// ============================================================================
/** @class Service GaudiKernel/Service.h
 *
 *  Base class for all services. It implements the IService and IProperty interfaces.
 *
 *  @author Pere Mato
 *  @author Marco Clemencic
 */
class GAUDI_API Service : public PropertyHolder<CommonMessaging<implements<IService, IProperty, IStateful>>>
{
public:
#ifndef __REFLEX__
  typedef Gaudi::PluginService::Factory<IService*, const std::string&, ISvcLocator*> Factory;
#endif
  friend class ServiceManager;

  /** Retrieve name of the service               */
  const std::string& name() const override;

  // State machine implementation
  StatusCode configure() override { return StatusCode::SUCCESS; }
  StatusCode initialize() override;
  StatusCode start() override;
  StatusCode stop() override;
  StatusCode finalize() override;
  StatusCode terminate() override { return StatusCode::SUCCESS; }
  Gaudi::StateMachine::State FSMState() const override { return m_state; }
  Gaudi::StateMachine::State targetFSMState() const override { return m_targetState; }
  StatusCode reinitialize() override;
  StatusCode restart() override;

  /** Initialize Service                          */
  StatusCode sysInitialize() override;
  /** Initialize Service                          */
  StatusCode sysStart() override;
  /** Initialize Service                          */
  StatusCode sysStop() override;
  /** Finalize Service                           */
  StatusCode sysFinalize() override;
  /// Re-initialize the Service
  StatusCode sysReinitialize() override;
  /// Re-initialize the Service
  StatusCode sysRestart() override;

  /** Standard Constructor                       */
  Service( std::string name, ISvcLocator* svcloc );
  /** Retrieve pointer to service locator        */
  SmartIF<ISvcLocator>& serviceLocator() const override;

  /** Method for setting declared properties to the values
      specified for the job.
  */
  StatusCode setProperties();

  /** Access a service by name, creating it if it doesn't already exist.
  */
  template <class T>
  StatusCode service( const std::string& name, const T*& psvc, bool createIf = true ) const
  {
    ISvcLocator& svcLoc = *serviceLocator();
    auto ptr            = ServiceLocatorHelper( svcLoc, *this )
                   .service<T>( name, !createIf, // quiet
                                createIf );
    if ( ptr ) {
      psvc = ptr.get();
      const_cast<T*>( psvc )->addRef();
      return StatusCode::SUCCESS;
    }
    // else
    psvc = nullptr;
    return StatusCode::FAILURE;
  }

  template <class T>
  StatusCode service( const std::string& name, T*& psvc, bool createIf = true ) const
  {
    auto ptr = service<T>( name, createIf );
    psvc     = ( ptr ? ptr.get() : nullptr );
    if ( psvc ) {
      psvc->addRef();
      return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
  }

  template <typename IFace = IService>
  SmartIF<IFace> service( const std::string& name, bool createIf = true ) const
  {
    return ServiceLocatorHelper( *serviceLocator(), *this )
        .service<IFace>( name, !createIf, // quiet
                         createIf );
  }

  /** Access a service by name and type, creating it if it doesn't already exist.
  */
  template <class T>
  StatusCode service( const std::string& svcType, const std::string& svcName, T*& psvc ) const
  {
    return service( svcType + "/" + svcName, psvc );
  }

  /** Declare used tool
   *
   *  @param handle ToolHandle<T>
   *  @param toolTypeAndName
   *  @param parent, default public tool
   *  @param create if necessary, default true
   */
  template <class T>
  StatusCode declareTool( ToolHandle<T>& handle, std::string toolTypeAndName = "", bool createIf = true )
  {

    if ( toolTypeAndName == "" ) toolTypeAndName = handle.typeAndName();

    StatusCode sc = handle.initialize( toolTypeAndName, handle.isPublic() ? nullptr : this, createIf );
    if ( UNLIKELY( !sc ) ) {
      throw GaudiException{std::string{"Cannot create handle for "} + ( handle.isPublic() ? "public" : "private" ) +
                               " tool " + toolTypeAndName,
                           name(), sc};
    }

    return sc;
  }

  // ==========================================================================
  /** The standard auditor service.May not be invoked before sysInitialize()
   *  has been invoked.
   */
  SmartIF<IAuditorSvc>& auditorSvc() const;

protected:
  /** Standard Destructor                        */
  ~Service() override;
  /** Service state                              */
  Gaudi::StateMachine::State m_state = Gaudi::StateMachine::OFFLINE;
  /** Service state                              */
  Gaudi::StateMachine::State m_targetState = Gaudi::StateMachine::OFFLINE;

  /// get the @c Service's output level
  int outputLevel() const { return m_outputLevel.value(); }

private:
  void sysInitialize_imp();
  StatusCode m_initSC;
  std::once_flag m_initFlag;

  /** Service Name  */
  std::string m_name;
  /** Service Locator reference                  */
  mutable SmartIF<ISvcLocator> m_svcLocator;
  SmartIF<ISvcManager> m_svcManager;

  void setServiceManager( ISvcManager* ism ) override;

protected:
  // Properties

  Gaudi::Property<int> m_outputLevel{this, "OutputLevel", MSG::NIL, "output level"};
  Gaudi::Property<bool> m_auditInit{this, "AuditServices", false, "[[deprecated]] unused"};
  Gaudi::Property<bool> m_auditorInitialize{this, "AuditInitialize", false, "trigger auditor on initialize()"};
  Gaudi::Property<bool> m_auditorStart{this, "AuditStart", false, "trigger auditor on start()"};
  Gaudi::Property<bool> m_auditorStop{this, "AuditStop", false, "trigger auditor on stop()"};
  Gaudi::Property<bool> m_auditorFinalize{this, "AuditFinalize", false, "trigger auditor on finalize()"};
  Gaudi::Property<bool> m_auditorReinitialize{this, "AuditReinitialize", false, "trigger auditor on reinitialize()"};
  Gaudi::Property<bool> m_auditorRestart{this, "AuditRestart", false, "trigger auditor on restart()"};

  /** Auditor Service                            */
  mutable SmartIF<IAuditorSvc> m_pAuditorSvc;
};

#ifndef GAUDI_NEW_PLUGIN_SERVICE
template <class T>
class SvcFactory
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
#define DECLARE_SERVICE_FACTORY( x ) DECLARE_FACTORY_WITH_CREATOR( x, SvcFactory<x>, Service::Factory )
#define DECLARE_NAMED_SERVICE_FACTORY( x, n )                                                                          \
  DECLARE_FACTORY_WITH_CREATOR_AND_ID( x, SvcFactory<x>, #n, Service::Factory )
#define DECLARE_NAMESPACE_SERVICE_FACTORY( n, x ) DECLARE_SERVICE_FACTORY( n::x )

#else

// macros to declare factories
#define DECLARE_SERVICE_FACTORY( x ) DECLARE_COMPONENT( x )
#define DECLARE_NAMED_SERVICE_FACTORY( x, n ) DECLARE_COMPONENT_WITH_ID( x, #n )
#define DECLARE_NAMESPACE_SERVICE_FACTORY( n, x ) DECLARE_COMPONENT( n::x )

#endif

#endif // GAUDIKERNEL_SERVICE_H
