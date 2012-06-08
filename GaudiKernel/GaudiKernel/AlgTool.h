#ifndef GAUDIKERNEL_ALGTOOL_H
#define GAUDIKERNEL_ALGTOOL_H
// ============================================================================
// Include files
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/IMonitorSvc.h"
#include "GaudiKernel/IStateful.h"

#include <vector>
#include <list>

// Forward declarations

/** @class AlgTool AlgTool.h GaudiKernel/AlgTool.h
 *
 *  Base class from which all the concrete tool classes
 *  should be derived. Specific methods for doing something
 *  useful should be implemented in the concrete tools.
 *  Sub-types of this class could implement an additional
 *  interface for behavior common to sets of concrete tools
 *  (for example vertexers).
 *
 *  @author Gloria Corti
 *  @author Pere Mato
 */
class GAUDI_API AlgTool: public implements3<IAlgTool, IProperty, IStateful> {
  friend class ToolSvc;
public:

  /// Query for a given interface
  virtual StatusCode queryInterface(const InterfaceID& riid, void** ppvUnknown);

  /// Retrieve full identifying name of the concrete tool object.
  virtual const std::string& name() const;

  /// Retrieve type (concrete class) of the sub-algtool.
  virtual const std::string& type() const;

  /// Retrieve parent of the sub-algtool.
  virtual const IInterface*  parent() const;

  // State machine implementation
  virtual StatusCode configure() { return StatusCode::SUCCESS; }
  virtual StatusCode initialize();
  virtual StatusCode start();
  virtual StatusCode stop();
  virtual StatusCode finalize();
  virtual StatusCode terminate() { return StatusCode::SUCCESS; }
  virtual StatusCode reinitialize();
  virtual StatusCode restart();
  virtual Gaudi::StateMachine::State FSMState() const { return m_state; }
  virtual Gaudi::StateMachine::State targetFSMState() const { return m_targetState; }

  /// Initialize AlgTool
  virtual StatusCode sysInitialize();

  /// Start AlgTool
  virtual StatusCode sysStart();

  /// Stop AlgTool
  virtual StatusCode sysStop();

  /// Finalize AlgTool
  virtual StatusCode sysFinalize();

  /// Initialize AlgTool
  virtual StatusCode sysReinitialize();

  /// Start AlgTool
  virtual StatusCode sysRestart();

  /// Default implementations for IProperty interface.
  virtual StatusCode setProperty( const Property&    p );
  virtual StatusCode setProperty( const std::string& s );
  virtual StatusCode setProperty( const std::string& n, const std::string& v);
  virtual StatusCode getProperty(Property* p) const;
  virtual const Property& getProperty( const std::string& name) const;
  virtual StatusCode getProperty( const std::string& n, std::string& v ) const;
  virtual const std::vector<Property*>& getProperties( ) const;

  inline PropertyMgr * getPropertyMgr() { return m_propertyMgr; }

public:

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


  /** Standard Constructor.
   *  @param type the concrete class of the sub-algtool
   *  @param name the full name of the concrete sub-algtool
   *  @param parent the parent of the concrete sub-algtool
   */
  AlgTool( const std::string& type,
           const std::string& name,
           const IInterface* parent);

  /// Retrieve pointer to service locator.
  ISvcLocator* serviceLocator()  const;

  /// shortcut for the method service locator
  ISvcLocator* svcLoc()  const { return serviceLocator() ; }

  /// Retrieve pointer to message service.
  IMessageSvc* msgSvc() const;

  /// The standard ToolSvc service, Return a pointer to the service if present
  IToolSvc* toolSvc() const;

  /** Method for setting declared properties to the values specified in the
   *  jobOptions via the job option service. This method is called by the
   *  ToolSvc after creating the concrete tool, before passing it to the
   *  requesting parent and does not need to be called explicitly.
   */
  StatusCode setProperties();

  /** Access a service by name,
   *  creating it if it doesn't already exist.
   */
  template <class T>
  StatusCode service
  ( const std::string& name, T*& svc, bool createIf = true ) const {
    return service_i(name, createIf, T::interfaceID(), (void**)&svc);
  }

  /** Access a service by name, type creating it if it doesn't already exist.
   */
  template <class T>
  StatusCode service( const std::string& type, const std::string& name, T*& svc) const {
    return service_i(type, name, T::interfaceID(), (void**)&svc);
  }

  /// Return a pointer to the service identified by name (or "type/name")
  SmartIF<IService> service(const std::string& name, const bool createIf = true, const bool quiet = false) const;

  /// declare interface
  void declInterface( const InterfaceID&, void*);

  template <class I> class declareInterface {
  public:
    template <class T> declareInterface(T* tool) {
      tool->declInterface( I::interfaceID(), (I*)tool);
    }
  };
  // ==========================================================================
  /** Declare the named property
   *
   *  @code
   *
   *  MyTool ( const std::string& type   ,
   *           const std::string& name   ,
   *           const IInterface*  parent )
   *     : AlgTool  ( type , name , pSvc )
   *     , m_property1   ( ... )
   *     , m_property2   ( ... )
   *   {
   *     // declare the property
   *     declareProperty( "Property1" , m_property1 , "Doc for property #1" ) ;
   *
   *     // declare the property and attach the handler to it
   *     declareProperty( "Property2" , m_property2 , "Doc for property #2" )
   *        -> declareUpdateHandler( &MyTool::handler_2 ) ;
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
  ( const std::string& name         ,
    T&                 property     ,
    const std::string& doc = "none" ) const
  {
    return m_propertyMgr -> declareProperty ( name , property , doc ) ;
  }
  /// Declare remote named properties
  Property* declareRemoteProperty
  ( const std::string& name       ,
    IProperty*         rsvc       ,
    const std::string& rname = "" ) const
  {
    return m_propertyMgr-> declareRemoteProperty ( name , rsvc , rname ) ;
  }
  // ==========================================================================
  /// Access the auditor service
  IAuditorSvc* auditorSvc() const;

  /** @brief Access the monitor service
   *
   *   @attention Note that this method will return a NULL pointer if no monitor service is
   *              configured to be present. You must take this possibility into account when
   *              using the pointer
   *   @return Pointer to the Monitor service
   *   @retval NULL No monitor service is present
   *   @retval non-NULL A monitor service is present and available to be used
   */
  inline IMonitorSvc* monitorSvc() const
  {
    // If not already located try to locate it without forcing a creation
    if ( !m_pMonitorSvc ){
      service_i( m_monitorSvcName, false,
                 IMonitorSvc::interfaceID(), pp_cast<void>(&m_pMonitorSvc) ).ignore();
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
    IMonitorSvc* mS = monitorSvc();
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
    IMonitorSvc* mS = monitorSvc();
    if ( mS ) mS->declareInfo(name, format, var, size, desc, this);
  }

protected:

  /// get tool's output level
  int           outputLevel () const { return (int)m_outputLevel ; }

  /// Accessor for the Message level property
  IntegerProperty & outputLevelProperty() { return m_outputLevel; }

  /** callback for output level property */
  void initOutputLevel(Property& prop);



protected:

  // Standard destructor.
  virtual ~AlgTool();

private:
  typedef std::list<std::pair<InterfaceID,void*> >  InterfaceList;
  IntegerProperty      m_outputLevel;      ///< AlgTool output level
  std::string          m_type;             ///< AlgTool type (concrete class name)
  const std::string    m_name;             ///< AlgTool full name
  const IInterface*    m_parent;           ///< AlgTool parent
  mutable ISvcLocator* m_svcLocator;       ///< Pointer to Service Locator service
  mutable IMessageSvc* m_messageSvc;       ///< Message service
  mutable IToolSvc*    m_ptoolSvc;         ///< Tool service
  mutable IMonitorSvc* m_pMonitorSvc;      ///< Online Monitoring Service
  std::string          m_monitorSvcName;   ///< Name to use for Monitor Service
  PropertyMgr*         m_propertyMgr;      ///< Property Manager
  InterfaceList        m_interfaceList;    ///< Interface list
  std::string          m_threadID;         ///< Thread Id for Alg Tool

  /** implementation of service method */
  StatusCode service_i(const std::string& algName,
                       bool createIf,
                       const InterfaceID& iid,
                       void** ppSvc) const;
  StatusCode service_i(const std::string& svcType,
                       const std::string& svcName,
                       const InterfaceID& iid,
                       void** ppS) const;

  mutable IAuditorSvc*      m_pAuditorSvc; ///< Auditor Service

  BooleanProperty m_auditInit;
  bool         m_auditorInitialize;///< flag for auditors in "initialize()"
  bool         m_auditorStart;     ///< flag for auditors in "start()"
  bool         m_auditorStop;      ///< flag for auditors in "stop()"
  bool         m_auditorFinalize;  ///< flag for auditors in "finalize()"
  bool         m_auditorReinitialize;///< flag for auditors in "reinitialize()"
  bool         m_auditorRestart;     ///< flag for auditors in "restart()"

  Gaudi::StateMachine::State m_state;            ///< state of the Tool
  Gaudi::StateMachine::State m_targetState;      ///< state of the Tool
};

#endif // GAUDIKERNEL_ALGTOOL_H
