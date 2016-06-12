#ifndef GAUDIKERNEL_AUDITOR_H
#define GAUDIKERNEL_AUDITOR_H

// Include files
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IAuditor.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"  /*used by service(..)*/
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/CommonMessaging.h"
#include <Gaudi/PluginService.h>
#include <string>
#include <vector>

// Forward declarations
class IService;
class IMessageSvc;
class Property;
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
class GAUDI_API Auditor : public CommonMessaging<implements<IAuditor,
                                                            IProperty>> {
public:
#ifndef __REFLEX__
  typedef Gaudi::PluginService::Factory<IAuditor*,
                                        const std::string&,
                                        ISvcLocator*> Factory;
#endif

  /** Constructor
      @param name    The algorithm object's name
      @param svcloc  A pointer to a service location service */
  Auditor( const std::string& name, ISvcLocator *svcloc );
  /// Destructor
  ~Auditor() override = default;

  /** Initialization method invoked by the framework. This method is responsible
      for any bookkeeping of initialization required by the framework itself.
  */
  StatusCode sysInitialize() override;
  /** Finalization method invoked by the framework. This method is responsible
      for any bookkeeping of initialization required by the framework itself.
  */
  StatusCode sysFinalize() override;

  /// The following methods are meant to be implemented by the child class...

  void before(StandardEventType, INamedInterface*) override;
  void before(StandardEventType, const std::string&) override;

  void before(CustomEventTypeRef, INamedInterface*) override;
  void before(CustomEventTypeRef, const std::string&) override;

  void after(StandardEventType, INamedInterface*, const StatusCode&) override;
  void after(StandardEventType, const std::string&, const StatusCode&) override;

  void after(CustomEventTypeRef, INamedInterface*, const StatusCode&) override;
  void after(CustomEventTypeRef, const std::string&, const StatusCode&) override;

  // Obsolete methods

  void beforeInitialize(INamedInterface* )  override;
  void afterInitialize(INamedInterface* )  override;

  void beforeReinitialize(INamedInterface* )  override;
  void afterReinitialize(INamedInterface* )  override;

  void beforeExecute(INamedInterface* ) override;
  void afterExecute(INamedInterface*, const StatusCode& ) override;

  void beforeFinalize(INamedInterface* )  override;
  void afterFinalize(INamedInterface* )  override;

  void beforeBeginRun(INamedInterface* ) override;
  void afterBeginRun(INamedInterface* ) override;

  void beforeEndRun(INamedInterface* ) override;
  void afterEndRun(INamedInterface* ) override;

  virtual StatusCode initialize();
  virtual StatusCode finalize();

  const std::string&  name() const  override;

  bool isEnabled() const  override;

  /** The standard service locator. Returns a pointer to the service locator service.
      This service may be used by an auditor to request any services it requires in
      addition to those provided by default.
  */
  SmartIF<ISvcLocator>& serviceLocator() const;

  /** Access a service by name, creating it if it doesn't already exist.
  */
  template <class T>
  StatusCode service( const std::string& name, T*& svc, bool createIf = false ) const {
    auto ptr = serviceLocator()->service<T>(name, createIf);
    if (ptr) {
      svc = ptr.get();
      svc->addRef();
      return StatusCode::SUCCESS;
    }
    svc = nullptr;
    return StatusCode::FAILURE;
  }

  template <class T = IService>
  SmartIF<T> service( const std::string& name, bool createIf = false ) const {
    return serviceLocator()->service<T>(name, createIf);
  }

  /// Set a value of a property of an auditor.
  StatusCode setProperty(const Property& p) override;

  /// Implementation of IProperty::setProperty
  StatusCode setProperty( const std::string& s ) override;

  /// Implementation of IProperty::setProperty
  StatusCode setProperty( const std::string& n, const std::string& v) override;

  /// Get the value of a property.
  StatusCode getProperty(Property* p) const override;

  /// Get the property by name.
  const Property& getProperty( const std::string& name) const override;

  /// Implementation of IProperty::getProperty
  StatusCode getProperty( const std::string& n, std::string& v ) const override;

  /// Get all properties.
  const std::vector<Property*>& getProperties( ) const override;

  /// Implementation of IProperty::hasProperty
  bool hasProperty(const std::string& name) const override;

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
  { return Gaudi::Utils::setProperty ( m_PropertyMgr.get() , name , value ) ; }

  /** Set the auditor's properties. This method requests the job options service
      to set the values of any declared properties. The method is invoked from
      within sysInitialize() by the framework and does not need to be explicitly
      called by a concrete auditor.
  */
  StatusCode setProperties();

  // ==========================================================================
  /** Declare the named property
   *
   *  @code
   *
   *  MyAuditor( ... )
   *     : Auditor ( ...  )
   *     , m_property1   ( ... )
   *     , m_property2   ( ... )
   *   {
   *     // declare the property
   *     declareProperty( "Property1" , m_property1 , "Doc for property #1" ) ;
   *
   *     // declare the property and attach the handler to it
   *     declareProperty( "Property2" , m_property2 , "Doc for property #2" )
   *        -> declareUpdateHandler( &MyAuditor::handler_2 ) ;
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
  Property* declareProperty( const std::string& name, T& property,
			     const std::string& doc = "none") const {
	return m_PropertyMgr->declareProperty(name, property, doc);
  }

  // ==========================================================================
  /** Declare a property
   *
   *
   *  @code
   *
   *  MyAlg ( const std::string& name ,
   *          ISvcLocator*       pSvc )
   *     : Algorithm ( name , pSvc )
   *     , m_property1   ( ... )
   *   {
   *     // declare the property
   *     declareProperty(m_property1);
   *   }
   *  @endcode
   *
   *  @see PropertyMgr
   *  @see PropertyMgr::declareProperty
   *
   *  @param property the property itself,
   *  @return pointer to the property object passed as argument
   */
  Property* declareProperty(Property &property) const {
    return m_PropertyMgr->declareProperty(property);
  }

 private:

  std::string m_name;	          ///< Auditor's name for identification

  mutable SmartIF<ISvcLocator> m_pSvcLocator;   ///< Pointer to service locator service
  SmartIF<PropertyMgr> m_PropertyMgr;   ///< For management of properties
  int          m_outputLevel;   ///< Auditor output level
  bool         m_isEnabled;     ///< Auditor is enabled flag
  bool         m_isInitialized; ///< Auditor has been initialized flag
  bool         m_isFinalized;   ///< Auditor has been finalized flag

  // Private Copy constructor: NO COPY ALLOWED
  Auditor(const Auditor& a);

  // Private assignment operator: NO ASSIGNMENT ALLOWED
  Auditor& operator=(const Auditor& rhs);
};

#ifndef GAUDI_NEW_PLUGIN_SERVICE
template <class T>
class AudFactory {
public:
#ifndef __REFLEX__
  template <typename S, typename... Args>
  static typename S::ReturnType create(Args&&... args) {
    return new T(std::forward<Args>(args)...);
  }
#endif
};

// Macros to declare component factories
#define DECLARE_AUDITOR_FACTORY(x) \
  DECLARE_FACTORY_WITH_CREATOR(x, AudFactory< x >, Auditor::Factory)
#define DECLARE_NAMESPACE_AUDITOR_FACTORY(n, x) \
    DECLARE_AUDITOR_FACTORY(n::x)

#else

// macros to declare factories
#define DECLARE_AUDITOR_FACTORY(x)              DECLARE_COMPONENT(x)
#define DECLARE_NAMESPACE_AUDITOR_FACTORY(n, x) DECLARE_COMPONENT(n::x)

#endif

#endif //GAUDIKERNEL_AUDITOR_H
