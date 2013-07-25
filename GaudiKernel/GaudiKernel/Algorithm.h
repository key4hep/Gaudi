#ifndef GAUDIKERNEL_ALGORITHM_H
#define GAUDIKERNEL_ALGORITHM_H
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IStateful.h"

#include <string>
#include <vector>

// Extra include files (forward declarations should be sufficient)
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IMonitorSvc.h"
#include "GaudiKernel/IExceptionSvc.h"
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/Property.h"
#include <Gaudi/PluginService.h>

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "unknown"
#endif

/** @class Algorithm Algorithm.h GaudiKernel/Algorithm.h
 *
 *  Base class from which all concrete algorithm classes should
 *  be derived.
 *
 *  In order for a concrete algorithm class to do anything
 *  useful the methods initialize(), execute() and finalize()
 *  should be overridden.
 *
 *  The base class provides utility methods for accessing
 *  standard services (event data service etc.); for declaring
 *  properties which may be configured by the job options
 *  service; and for creating sub algorithms.
 *  The only base class functionality which may be used in the
 *  constructor of a concrete algorithm is the declaration of
 *  member variables as properties. All other functionality,
 *  i.e. the use of services and the creation of sub-algorithms,
 *  may be used only in initialize() and afterwards (see the
 *  Gaudi user guide).
 *
 *  @author Paul Maley
 *  @author Pere Mato
 *  @author David Quarrie
 *  @date   1998
 */
class GAUDI_API Algorithm: public implements3<IAlgorithm, IProperty, IStateful> {
public:
  typedef Gaudi::PluginService::Factory2<IAlgorithm*,
                                         const std::string&,
                                         ISvcLocator*> Factory;

  /** Constructor
   *  @param name    The algorithm object's name
   *  @param svcloc  A pointer to a service location service
   */
  Algorithm( const std::string& name, ISvcLocator *svcloc,
             const std::string& version=PACKAGE_VERSION );
  /// Destructor
  virtual ~Algorithm();

  /** Reinitialization method invoked by the framework. This method is responsible
   *  for any reinitialization required by the framework itself.
   *  It will in turn invoke the reinitialize() method of the derived algorithm,
   * and of any sub-algorithms which it creates.
   */
  virtual StatusCode sysStart();

  /** Initialization method invoked by the framework. This method is responsible
   *  for any bookkeeping of initialization required by the framework itself.
   *  It will in turn invoke the initialize() method of the derived algorithm,
   * and of any sub-algorithms which it creates.
   */
  virtual StatusCode sysInitialize();


  /** Reinitialization method invoked by the framework. This method is responsible
   *  for any reinitialization required by the framework itself.
   *  It will in turn invoke the reinitialize() method of the derived algorithm,
   * and of any sub-algorithms which it creates.
   */
  virtual StatusCode sysReinitialize();

  /** Restart method invoked by the framework.
      It will in turn invoke the restart() method of the derived algorithm,
      and of any sub-algorithms which it creates.
  */
  virtual StatusCode sysRestart();

  /** The actions to be performed by the algorithm on an event. This method is
   * invoked once per event for top level algorithms by the application
   *  manager.
   *  This method invokes execute() method.
   *  For sub-algorithms either the sysExecute() method or execute() method
   *  must be EXPLICITLY invoked by  the parent algorithm.
   */
  virtual StatusCode sysExecute();

  /** System stop. This method invokes the stop() method of a concrete
      algorithm and the stop() methods of all of that algorithm's sub algorithms.
  */
  virtual StatusCode sysStop();

  /** System finalization. This method invokes the finalize() method of a
   *  concrete algorithm and the finalize() methods of all of that algorithm's
   *  sub algorithms.
   */
  virtual StatusCode sysFinalize();

  /** beginRun method invoked by the framework. This method is responsible
      for any beginRun actions required by the framework itself.
      It will in turn invoke the beginRun() method of the derived algorithm,
      and of any sub-algorithms which it creates.
  */
  virtual StatusCode sysBeginRun( );

  /** endRun method invoked by the framework. This method is responsible
      for any endRun actions required by the framework itself.
      It will in turn invoke the endRun() method of the derived algorithm,
      and of any sub-algorithms which it creates.
  */
  virtual StatusCode sysEndRun( );

  /** The identifying name of the algorithm object. This is the name of a
   *  particular instantiation of an algorithm object as opposed to the name
   *  of the algorithm itself, e.g. "LinearTrackFit" may be the name of a
   *  concrete algorithm class,
   *  whereas "ApproxTrackFit" and "BestTrackFit" may be two instantiations
   *  of the class configured to find tracks with different fit criteria.
   */
  virtual const std::string& name() const;

  virtual const std::string& version() const;

  /// Dummy implementation of IStateful::configure() method
  virtual StatusCode configure () { return StatusCode::SUCCESS ; }
  /// Dummy implementation of IStateful::terminate() method
  virtual StatusCode terminate () { return StatusCode::SUCCESS ; }

  /// the default (empty) implementation of IStateful::initialize() method
  virtual StatusCode initialize () { return StatusCode::SUCCESS ; }
  /// the default (empty) implementation of IStateful::start() method
  virtual StatusCode start () { return StatusCode::SUCCESS ; }
  /// the default (empty) implementation of IStateful::stop() method
  virtual StatusCode stop () { return StatusCode::SUCCESS ; }
  /// the default (empty) implementation of IStateful::finalize() method
  virtual StatusCode finalize   () { return StatusCode::SUCCESS ; }

  /// the default (empty) implementation of IStateful::reinitialize() method
  virtual StatusCode reinitialize ();
  /// the default (empty) implementation of IStateful::restart() method
  virtual StatusCode restart ();

  /// Has this algorithm been executed since the last reset?
  virtual bool isExecuted( ) const;

  /// Set the executed flag to the specified state
  virtual void setExecuted( bool state );

  /** Reset the executed state of the Algorithm for the duration
   *  of the current event.
   */
  virtual void resetExecuted( );

  /** Algorithm begin run. This method is called at the beginning
   *  of the event loop.
   */
  virtual StatusCode beginRun();

  /// Algorithm end run. This method is called at the end of the event loop
  virtual StatusCode endRun();

  /// returns the current state of the algorithm
  virtual Gaudi::StateMachine::State FSMState() const { return m_state; }

  /// returns the state the algorithm will be in after the ongoing transition
  virtual Gaudi::StateMachine::State targetFSMState() const { return m_targetState; }

  /// Is this algorithm enabled or disabled?
  virtual bool isEnabled( ) const;

  /// Did this algorithm pass or fail its filter criterion for the last event?
  virtual bool filterPassed( ) const;

  /// Set the filter passed flag to the specified state
  virtual void setFilterPassed( bool state );

  /// Access a service by name, creating it if it doesn't already exist.
  template <class T>
  StatusCode service
  ( const std::string& name, T*& psvc, bool createIf = true ) const {
    return service_i(name, createIf, T::interfaceID(), (void**)&psvc);
  }

  /// Access a service by name and type, creating it if it doesn't already exist.
  template <class T>
  StatusCode service( const std::string& svcType, const std::string& svcName,
                      T*& psvc) const {
    return service_i(svcType, svcName, T::interfaceID(), (void**)&psvc);
  }

  /// Return a pointer to the service identified by name (or "type/name")
  SmartIF<IService> service(const std::string& name, const bool createIf = true, const bool quiet = false) const;

  /// Set the output level for current algorithm
  void setOutputLevel( int level );

  /** The standard auditor service.May not be invoked before sysInitialize()
   *  has been invoked.
   */
  SmartIF<IAuditorSvc>& auditorSvc() const;

  /** The standard Chrono & Stat service,
   *  Return a pointer to the service if present
   */
  SmartIF<IChronoStatSvc>& chronoSvc() const;
  /// Obsoleted name, kept due to the backwards compatibility
  SmartIF<IChronoStatSvc>& chronoStatService() const;

  /** The standard detector data service.
   *  May not be invoked before sysInitialize() has been invoked.
   */
  SmartIF<IDataProviderSvc>& detSvc() const;

  /// Obsoleted name, kept due to the backwards compatibility
  SmartIF<IDataProviderSvc>& detDataService() const;

  /** The standard detector data persistency conversion service.
   *  May not be invoked before sysInitialize() has been invoked.
   */
  SmartIF<IConversionSvc>& detCnvSvc() const;

  /// Obsoleted name, kept due to the backwards compatibility
  SmartIF<IConversionSvc>& detDataCnvService() const;

  /** The standard event data service.
   *  May not be invoked before sysInitialize() has been invoked.
   */
  SmartIF<IDataProviderSvc>& eventSvc() const;
  /// shortcut for  method eventSvc
  SmartIF<IDataProviderSvc>& evtSvc  () const { return eventSvc() ; }
  /// Obsoleted name, kept due to the backwards compatibility
  SmartIF<IDataProviderSvc>& eventDataService() const;

  /** The standard event data persistency conversion service.
   *  May not be invoked before sysInitialize() has been invoked.
   */
  SmartIF<IConversionSvc>&   eventCnvSvc() const;
  /// Obsoleted name, kept due to the backwards compatibility
  SmartIF<IConversionSvc>&   eventDataCnvService() const;

  /** The standard histogram service.
   *  May not be invoked before sysInitialize() has been invoked.
   */
  SmartIF<IHistogramSvc>& histoSvc() const;
  /// Obsoleted name, kept due to the backwards compatibility
  SmartIF<IHistogramSvc>& histogramDataService() const;

  /** The standard message service.
   *  Returns a pointer to the standard message service.
   *  May not be invoked before sysInitialize() has been invoked.
   */
  SmartIF<IMessageSvc>&      msgSvc() const;

  /// Obsoleted name, kept due to the backwards compatibility
  SmartIF<IMessageSvc>&      messageService() const;

  /** The standard N tuple service.
   *  Returns a pointer to the N tuple service if present.
   */
  SmartIF<INTupleSvc>& ntupleSvc() const;

  /// Obsoleted name, kept due to the backwards compatibility
  SmartIF<INTupleSvc>& ntupleService() const;

  /** AIDA-based NTuple service
   *  Returns a pointer to the AIDATuple service if present.
   */
  // SmartIF<IAIDATupleSvc>& atupleSvc() const;


  /** The standard RandomGen service,
   *  Return a pointer to the service if present
   */
  SmartIF<IRndmGenSvc>& randSvc() const;

  /// The standard ToolSvc service, Return a pointer to the service if present
  SmartIF<IToolSvc>& toolSvc() const;

  /// Get the exception Service
  SmartIF<IExceptionSvc>& exceptionSvc() const;

  /// get Algorithm Context Service
  SmartIF<IAlgContextSvc>& contextSvc() const ;

  /** The standard service locator.
   *  Returns a pointer to the service locator service.
   *  This service may be used by an algorithm to request
   *  any services it requires in addition to those provided by default.
   */
  SmartIF<ISvcLocator>& serviceLocator() const;
  /// shortcut for method serviceLocator
  SmartIF<ISvcLocator>& svcLoc        () const { return serviceLocator() ; }

  /// register for Algorithm Context Service?
  bool registerContext() const { return m_registerContext ; }

  /** Create a sub algorithm.
   *  A call to this method creates a child algorithm object.
   *  Note that the returned pointer is to Algorithm
   *  (as opposed to IAlgorithm), and thus the methods of IProperty
   *  are also available for the direct setting of the sub-algorithm's
   *  properties. Using this mechanism instead of creating daughter
   *  algorithms directly via the new operator is prefered since then
   *  the framework may take care of all of the necessary book-keeping.
   *  @param type The concrete algorithm class of the sub algorithm
   *  @param name The name to be given to the sub algorithm
   *  @param pSubAlg Set to point to the newly created algorithm object
   */
  StatusCode createSubAlgorithm( const std::string& type,
                                 const std::string& name, Algorithm*& pSubAlg );

  /// List of sub-algorithms. Returns a pointer to a vector of (sub) Algorithms
  std::vector<Algorithm*>* subAlgorithms() const;

  /// Implementation of IProperty::setProperty
  virtual StatusCode setProperty( const Property& p );
  /// Implementation of IProperty::setProperty
  virtual StatusCode setProperty( const std::string& s );
  /// Implementation of IProperty::setProperty
  virtual StatusCode setProperty( const std::string& n, const std::string& v);
  /// Implementation of IProperty::getProperty
  virtual StatusCode getProperty(Property* p) const;
  /// Implementation of IProperty::getProperty
  virtual const Property& getProperty( const std::string& name) const;
  /// Implementation of IProperty::getProperty
  virtual StatusCode getProperty( const std::string& n, std::string& v ) const;
  /// Implementation of IProperty::getProperties
  virtual const std::vector<Property*>& getProperties( ) const;

  /** Set the algorithm's properties.
   *  This method requests the job options service
   *  to set the values of any declared properties.
   *  The method is invoked from within sysInitialize() by the framework
   *  and does not need to be explicitly
   *  called by a concrete algorithm.
   */
  StatusCode setProperties();
  // ==========================================================================
  /** Declare the named property
   *
   *
   *  @code
   *
   *  MyAlg ( const std::string& name ,
   *          ISvcLocator*       pSvc )
   *     : Algorithm ( name , pSvc )
   *     , m_property1   ( ... )
   *     , m_property2   ( ... )
   *   {
   *     // declare the property
   *     declareProperty( "Property1" , m_property1 , "Doc for property #1" ) ;
   *
   *     // declare the property and attach the handler to it
   *     declareProperty( "Property2" , m_property2 , "Doc for property #2" )
   *        -> declareUpdateHandler( &MyAlg::handler_2 ) ;
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
    return m_propertyMgr->declareProperty(name, property, doc);
  }
  // ==========================================================================
  /// Declare remote named properties
  Property* declareRemoteProperty
  ( const std::string& name       ,
    IProperty*         rsvc       ,
    const std::string& rname = "" ) const
  {
    return m_propertyMgr -> declareRemoteProperty ( name , rsvc , rname );
  }
  // ==========================================================================
  /** @brief Access the monitor service
   *
   *   @attention Note that this method will return a NULL pointer if no monitor service is
   *              configured to be present. You must take this possibility into account when
   *              using the pointer
   *   @return Pointer to the Monitor service
   *   @retval NULL No monitor service is present
   *   @retval non-NULL A monitor service is present and available to be used
   */
  inline SmartIF<IMonitorSvc>& monitorSvc() const
  {
    // If not already located try to locate it without forcing a creation
    if ( !m_pMonitorSvc.isValid() ){
      m_pMonitorSvc = service(m_monitorSvcName, false, true); // do not create and be quiet
    }
    return m_pMonitorSvc;
  }

  /** Declare monitoring information
      @param name Monitoring information name known to the external system
      @param var  Monitoring Listener address (the item to monitor...)
      @param desc Textual description of the information being monitored
  */
  template <class T>
  void declareInfo( const std::string& name,
                    const T& var,
                    const std::string& desc ) const
  {
    IMonitorSvc* mS = monitorSvc().get();
    if ( mS ) mS->declareInfo(name, var, desc, this);
  }

  /** Declare monitoring information (special case)
      @param name Monitoring information name known to the external system
      @param format Format information
      @param var  Monitoring Listener address
      @param size Monitoring Listener address size
      @param desc Textual description of the information being monitored
  */
  void declareInfo( const std::string& name,
                    const std::string& format,
                    const void* var,
                    int size,
                    const std::string& desc ) const
  {
    IMonitorSvc* mS = monitorSvc().get();
    if ( mS ) mS->declareInfo(name, format, var, size, desc, this);
  }

  // ==========================================================================
public:
  // ==========================================================================
  /** set the property form the value
   *
   *  @code
   *
   *  std::vector<double> data = ... ;
   *
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
  // ==========================================================================
protected:

  /// Has the Algorithm already been initialized?
  bool isInitialized( ) const { return Gaudi::StateMachine::INITIALIZED == m_state; }

  /// Has the Algorithm already been finalized?
  bool isFinalized( ) const { return Gaudi::StateMachine::CONFIGURED == m_state; }

  /// retrieve the Algorithm output level
  int  outputLevel() const { return (int)m_outputLevel ; }

  /// Accessor for the Message level property
  IntegerProperty & outputLevelProperty() { return m_outputLevel; }

  /// callback for output level property
  void initOutputLevel(Property& prop);


private:

  std::string m_name;            ///< Algorithm's name for identification
  std::string m_version;         ///< Algorithm's version
  std::vector<Algorithm *>* m_subAlgms; ///< Sub algorithms

  mutable SmartIF<IMessageSvc>      m_MS;       ///< Message service
  mutable SmartIF<IDataProviderSvc> m_EDS;      ///< Event data service
  mutable SmartIF<IConversionSvc>   m_ECS;      ///< Event conversion service
  mutable SmartIF<IDataProviderSvc> m_DDS;      ///< Detector data service
  mutable SmartIF<IConversionSvc>   m_DCS;      ///< Detector conversion service
  mutable SmartIF<IHistogramSvc>    m_HDS;      ///< Histogram data service
  mutable SmartIF<INTupleSvc>       m_NTS;      ///< N tuple service
  //  mutable SmartIF<IAIDATupleSvc>    m_ATS;      ///< AIDA tuple service
  mutable SmartIF<IChronoStatSvc>   m_CSS;      ///< Chrono & Stat Service
  mutable SmartIF<IRndmGenSvc>      m_RGS;      ///< Random Number Generator Service
  mutable SmartIF<IExceptionSvc>    m_EXS;      ///< Exception Handler Service
  mutable SmartIF<IAuditorSvc>      m_pAuditorSvc; ///< Auditor Service
  mutable SmartIF<IToolSvc>         m_ptoolSvc;    ///< ToolSvc Service
  mutable SmartIF<IMonitorSvc>      m_pMonitorSvc; ///< Online Monitoring Service
  mutable SmartIF<IAlgContextSvc>   m_contextSvc ; ///< Algorithm Context Service
  bool  m_registerContext ; ///< flag to register for Algorithm Context Service
  std::string               m_monitorSvcName; ///< Name to use for Monitor Service
  SmartIF<ISvcLocator>  m_pSvcLocator;      ///< Pointer to service locator service
  PropertyMgr* m_propertyMgr;      ///< For management of properties
  IntegerProperty m_outputLevel;   ///< Algorithm output level
  int          m_errorMax;         ///< Algorithm Max number of errors
  int          m_errorCount;       ///< Algorithm error counter
  BooleanProperty m_auditInit;     ///< global flag for auditors
  bool         m_auditorInitialize;///< flag for auditors in "initialize()"
  bool         m_auditorReinitialize;///< flag for auditors in "Reinitialize()"
  bool         m_auditorRestart;   ///< flag for auditors in "Restart()"
  bool         m_auditorExecute;   ///< flag for auditors in "execute()"
  bool         m_auditorFinalize;  ///< flag for auditors in "finalize()"
  bool         m_auditorBeginRun;  ///< flag for auditors in "beginRun()"
  bool         m_auditorEndRun;    ///< flag for auditors in "endRun()"
  bool         m_auditorStart;///< flag for auditors in "initialize()"
  bool         m_auditorStop;///< flag for auditors in "Reinitialize()"
  bool         m_filterPassed;     ///< Filter passed flag
  bool         m_isEnabled;        ///< Algorithm is enabled flag
  bool         m_isExecuted;       ///< Algorithm is executed flag
  Gaudi::StateMachine::State m_state;            ///< Algorithm has been initialized flag
  Gaudi::StateMachine::State m_targetState;      ///< Algorithm has been initialized flag
  bool         m_isFinalized;      ///< Algorithm has been finalized flag

  /// implementation of service method
  StatusCode service_i(const std::string& svcName,
                       bool createIf,
                       const InterfaceID& iid,
                       void** ppSvc) const;
  StatusCode service_i(const std::string& svcType,
                       const std::string& svcName,
                       const InterfaceID& iid,
                       void** ppSvc) const;

  /// Private Copy constructor: NO COPY ALLOWED
  Algorithm(const Algorithm& a);

  /// Private assignment operator: NO ASSIGNMENT ALLOWED
  Algorithm& operator=(const Algorithm& rhs);
};

#ifndef GAUDI_NEW_PLUGIN_SERVICE
template <class T>
class AlgFactory {
public:
  template <typename S>
  static typename S::ReturnType create(typename S::Arg1Type a1,
                                       typename S::Arg2Type a2) {
    return new T(a1, a2);
  }
};

// Macros to declare component factories
#define DECLARE_ALGORITHM_FACTORY(x) \
  DECLARE_FACTORY_WITH_CREATOR(x, AlgFactory< x >, Algorithm::Factory)
#define DECLARE_NAMED_ALGORITHM_FACTORY(x, n) \
  DECLARE_FACTORY_WITH_CREATOR_AND_ID(x, AlgFactory< x >, \
                                      #n, Algorithm::Factory)
#define DECLARE_NAMESPACE_ALGORITHM_FACTORY(n, x) \
  DECLARE_ALGORITHM_FACTORY(n::x)

#else

// Macros to declare component factories
#define DECLARE_ALGORITHM_FACTORY(x)              DECLARE_COMPONENT(x)
#define DECLARE_NAMED_ALGORITHM_FACTORY(x, n)     DECLARE_COMPONENT_WITH_ID(x, #n)
#define DECLARE_NAMESPACE_ALGORITHM_FACTORY(n, x) DECLARE_COMPONENT(n::x)

#endif

#endif //GAUDIKERNEL_ALGORITHM_H
