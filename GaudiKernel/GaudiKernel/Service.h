#ifndef GAUDIKERNEL_SERVICE_H
#define GAUDIKERNEL_SERVICE_H
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ServiceLocatorHelper.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IStateful.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/SmartIF.h"
#include <Gaudi/PluginService.h>
// ============================================================================
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
class GAUDI_API Service: public CommonMessaging<implements3<IService, IProperty, IStateful> > {
public:
  typedef Gaudi::PluginService::Factory2<IService*,
                                         const std::string&,
                                         ISvcLocator*> Factory;
  friend class ServiceManager;

  /// Release Interface instance.
  /// Specialized implementation because the default one is not enough.
  virtual unsigned long release();

  /** Retrieve name of the service               */
  virtual const std::string& name() const;

  // State machine implementation
  virtual StatusCode configure() { return StatusCode::SUCCESS; }
  virtual StatusCode initialize();
  virtual StatusCode start();
  virtual StatusCode stop();
  virtual StatusCode finalize();
  virtual StatusCode terminate() { return StatusCode::SUCCESS; }
  virtual Gaudi::StateMachine::State FSMState() const { return m_state; }
  virtual Gaudi::StateMachine::State targetFSMState() const { return m_targetState; }
  virtual StatusCode reinitialize();
  virtual StatusCode restart();

  /** Initialize Service                          */
  virtual StatusCode sysInitialize();
  /** Initialize Service                          */
  virtual StatusCode sysStart();
  /** Initialize Service                          */
  virtual StatusCode sysStop();
  /** Finalize Service                           */
  virtual StatusCode sysFinalize();
  /// Re-initialize the Service
  virtual StatusCode sysReinitialize();
  /// Re-initialize the Service
  virtual StatusCode sysRestart();

  // Default implementations for ISetProperty
  virtual StatusCode setProperty(const Property& p);
  virtual StatusCode setProperty( const std::string& s );
  virtual StatusCode setProperty( const std::string& n, const std::string& v);
  virtual StatusCode getProperty(Property* p) const;
  virtual const Property& getProperty( const std::string& name) const;
  virtual StatusCode getProperty( const std::string& n, std::string& v ) const;
  virtual const std::vector<Property*>& getProperties( ) const;

  /** set the property form the value
   *
   *  @code
   *
   *  std::vector<double> data = ... ;
   *  setProperty( "Data" , data ) ;
   *
   *  std::map<std::string,double> cuts = ... ;
   *  setProperty( "Cuts" , cuts ) ;
   *
   *  std::map<std::string,std::string> dict = ... ;
   *  setProperty( "Dictionary" , dict ) ;
   *
   *  @endcode
   *
   *  Note: the interface IProperty allows setting of the properties either
   *        directly from other properties or from strings only
   *
   *  This is very convenient in resetting of the default
   *  properties in the derived classes.
   *  E.g. without this method one needs to convert
   *  everything into strings to use IProperty::setProperty
   *
   *  @code
   *
   *    setProperty ( "OutputLevel" , "1"    ) ;
   *    setProperty ( "Enable"      , "True" ) ;
   *    setProperty ( "ErrorMax"    , "10"   ) ;
   *
   *  @endcode
   *
   *  For simple cases it is more or less ok, but for complicated properties
   *  it is just ugly..
   *
   *  @param name      name of the property
   *  @param value     value of the property
   *  @see Gaudi::Utils::setProperty
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date 2007-05-13
   */
  template <class TYPE>
  StatusCode setProperty
  ( const std::string& name  ,
    const TYPE&        value )
  { return Gaudi::Utils::setProperty ( m_propertyMgr , name , value ) ; }

  /** Standard Constructor                       */
  Service( const std::string& name, ISvcLocator* svcloc);
  /** Retrieve pointer to service locator        */
  SmartIF<ISvcLocator>& serviceLocator() const;

  /** Method for setting declared properties to the values
      specified for the job.
  */
  StatusCode setProperties();

  /** Access a service by name, creating it if it doesn't already exist.
  */
  template <class T>
  StatusCode service( const std::string& name, const T*& psvc, bool createIf = true ) const {
    ISvcLocator& svcLoc = *serviceLocator();
    SmartIF<T> ptr(
      ServiceLocatorHelper(svcLoc, *this).service(name, !createIf, // quiet
                                                  createIf));
    if (ptr.isValid()) {
      psvc = ptr.get();
      const_cast<T*>(psvc)->addRef();
      return StatusCode::SUCCESS;
    }
    // else
    psvc = 0;
    return StatusCode::FAILURE;
  }

  template <class T>
  StatusCode service( const std::string& name, T*& psvc, bool createIf = true ) const {
    ISvcLocator& svcLoc = *serviceLocator();
    SmartIF<T> ptr(
      ServiceLocatorHelper(svcLoc, *this).service(name, !createIf, // quiet
                                                  createIf));
    if (ptr.isValid()) {
      psvc = ptr.get();
      psvc->addRef();
      return StatusCode::SUCCESS;
    }
    // else
    psvc = 0;
    return StatusCode::FAILURE;
  }

  /** Access a service by name and type, creating it if it doesn't already exist.
  */
  template <class T>
  StatusCode service( const std::string& svcType, const std::string& svcName,
		      T*& psvc) const {
    return service(svcType + "/" + svcName, psvc);
  }
  // ==========================================================================
  /** Declare the named property
   *
   *  @code
   *
   *  MySvc ( const std::string& name ,
   *          ISvcLocator*       pSvc )
   *     : Service ( name , pSvc )
   *     , m_property1   ( ... )
   *     , m_property2   ( ... )
   *   {
   *     // declare the property
   *     declareProperty( "Property1" , m_property1 , "Doc for property #1" ) ;
   *
   *     // declare the property and attach the handler to it
   *     declareProperty( "Property2" , m_property2 , "Doc for property #2" )
   *        -> declareUpdateHandler( &MySvc::handler_2 ) ;
   *
   *   }
   *  @endcode
   *
   *  @see PropertyMgr
   *  @see PropertyMgr::declareProperty
   *
   *  @param name the property name
   *  @param property the property itself,
   *  @param doc      the documentation string
   *  @return the actual property objects
   */
  template <class T>
  Property* declareProperty
  ( const std::string& name              ,
    T&                 property          ,
    const std::string& doc      = "none" ) const
  {
    return m_propertyMgr -> declareProperty ( name , property , doc ) ;
  }
  // ==========================================================================
  /** Declare remote named properties. This is used to declare as a local property
      a property of another services or algorithm. The only needed information is
      IProperty interface of the remote service and the its property name if different
      of the current one.
      @param name       Local property name
      @param rsvc       Remote IProperty interface
      @param rname      Name of the property at remote service
  */
  Property* declareRemoteProperty
  ( const std::string& name       ,
    IProperty*         rsvc       ,
    const std::string& rname = "" ) const
  {
  	return m_propertyMgr -> declareRemoteProperty ( name , rsvc , rname ) ;
  }
  // ==========================================================================
  /** The standard auditor service.May not be invoked before sysInitialize()
   *  has been invoked.
   */
  SmartIF<IAuditorSvc>& auditorSvc() const;

protected:
  /** Standard Destructor                        */
  virtual ~Service();
  /** Service output level                       */
  IntegerProperty m_outputLevel;
  /** Service state                              */
  Gaudi::StateMachine::State    m_state;
  /** Service state                              */
  Gaudi::StateMachine::State    m_targetState;

  /// get the @c Service's output level
  int  outputLevel() const { return m_outputLevel.value(); }

private:
  /** Service Name  */
  std::string   m_name;
  /** Service Locator reference                  */
  mutable SmartIF<ISvcLocator> m_svcLocator;
  SmartIF<ISvcManager>  m_svcManager;
  /** Property Manager                           */
  PropertyMgr*  m_propertyMgr;

  void setServiceManager(ISvcManager* ism);

  /** Auditor Service                            */
  mutable SmartIF<IAuditorSvc>  m_pAuditorSvc;
  BooleanProperty       m_auditInit;
  bool                  m_auditorInitialize;
  bool                  m_auditorStart;
  bool                  m_auditorStop;
  bool                  m_auditorFinalize;
  bool                  m_auditorReinitialize;
  bool                  m_auditorRestart;

  /** callback for output level property */
  void initOutputLevel(Property& prop);
};

#ifndef GAUDI_NEW_PLUGIN_SERVICE
template <class T>
class SvcFactory {
public:
  template <typename S>
  static typename S::ReturnType create(typename S::Arg1Type a1,
                                       typename S::Arg2Type a2) {
    return new T(a1, a2);
  }
};

// Macros to declare component factories
#define DECLARE_SERVICE_FACTORY(x) \
  DECLARE_FACTORY_WITH_CREATOR(x, SvcFactory< x >, Service::Factory)
#define DECLARE_NAMED_SERVICE_FACTORY(x, n) \
  DECLARE_FACTORY_WITH_CREATOR_AND_ID(x, SvcFactory< x >, #n, Service::Factory)
#define DECLARE_NAMESPACE_SERVICE_FACTORY(n, x) \
  DECLARE_SERVICE_FACTORY(n::x)

#else

// macros to declare factories
#define DECLARE_SERVICE_FACTORY(x)              DECLARE_COMPONENT(x)
#define DECLARE_NAMED_SERVICE_FACTORY(x, n)     DECLARE_COMPONENT_WITH_ID(x, #n)
#define DECLARE_NAMESPACE_SERVICE_FACTORY(n, x) DECLARE_COMPONENT(n::x)

#endif

#endif // GAUDIKERNEL_SERVICE_H
