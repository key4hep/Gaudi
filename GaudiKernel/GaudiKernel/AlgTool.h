#ifndef GAUDIKERNEL_ALGTOOL_H
#define GAUDIKERNEL_ALGTOOL_H
// ============================================================================
// Include files
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/PropertyMgr.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/IMonitorSvc.h"
#include "GaudiKernel/IStateful.h"
#include <Gaudi/PluginService.h>
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/CommonMessaging.h"

#include "GaudiKernel/MinimalDataObjectHandle.h"
#include "GaudiKernel/DataObjectDescriptor.h"

template<class T>
class DataObjectHandle;

class ToolHandleInfo;

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
class GAUDI_API AlgTool: public CommonMessaging<implements3<IAlgTool, IProperty, IStateful>> {
//  friend class ToolSvc;
public:
#ifndef __REFLEX__
  typedef Gaudi::PluginService::Factory<IAlgTool*,
                                        const std::string&,
                                        const std::string&,
                                        const IInterface*> Factory;
#endif

  /// Query for a given interface
  StatusCode queryInterface(const InterfaceID& riid, void** ppvUnknown) override;

  /// Retrieve full identifying name of the concrete tool object.
  const std::string& name() const override;

  /// Retrieve type (concrete class) of the sub-algtool.
  const std::string& type() const override;

  /// Retrieve parent of the sub-algtool.
  const IInterface*  parent() const override;

  // State machine implementation
  StatusCode configure() override { return StatusCode::SUCCESS; }
  StatusCode initialize() override;
  StatusCode start() override;
  StatusCode stop() override;
  StatusCode finalize() override;
  StatusCode terminate() override { return StatusCode::SUCCESS; }
  StatusCode reinitialize() override;
  StatusCode restart() override;
  Gaudi::StateMachine::State FSMState() const override { return m_state; }
  Gaudi::StateMachine::State targetFSMState() const override { return m_targetState; }

  /// Initialize AlgTool
  StatusCode sysInitialize() override;

  /// Start AlgTool
  StatusCode sysStart() override;

  /// Stop AlgTool
  StatusCode sysStop() override;

  /// Finalize AlgTool
  StatusCode sysFinalize() override;

  /// Initialize AlgTool
  StatusCode sysReinitialize() override;

  /// Start AlgTool
  StatusCode sysRestart() override;

  /// Default implementations for IProperty interface.
  StatusCode setProperty( const Property&    p ) override;
  StatusCode setProperty( const std::string& s ) override;
  StatusCode setProperty( const std::string& n, const std::string& v) override;
  StatusCode getProperty(Property* p) const override;
  const Property& getProperty( const std::string& name) const override;
  StatusCode getProperty( const std::string& n, std::string& v ) const override;
  const std::vector<Property*>& getProperties( ) const override;
  bool hasProperty(const std::string& name) const override;

  inline PropertyMgr * getPropertyMgr() { return m_propertyMgr.get(); }

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
  { return Gaudi::Utils::setProperty ( m_propertyMgr.get() , name , value ) ; }


  /** Standard Constructor.
   *  @param type the concrete class of the sub-algtool
   *  @param name the full name of the concrete sub-algtool
   *  @param parent the parent of the concrete sub-algtool
   */
  AlgTool( const std::string& type,
           const std::string& name,
           const IInterface* parent);

  /// Retrieve pointer to service locator.
  SmartIF<ISvcLocator>& serviceLocator()  const override;

  /// shortcut for the method service locator
  ISvcLocator* svcLoc()  const { return serviceLocator() ; }

  /** accessor to event service  service
   *  @return pointer to detector service
   */
  IDataProviderSvc*    evtSvc    () const ;

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

  template <typename T>
  SmartIF<T> service(const std::string& name, const bool createIf = true, const bool quiet = false) const
  { return SmartIF<T>( service(name,createIf,quiet) ); }

protected:
  template <typename I>
  void declareInterface( I* i ) { m_interfaceList.emplace_back( I::interfaceID(), i ); }
public:
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

	/** Declare input data object
	 *
	 *  @param propertyName to identify input object in python config
	 *  @param handle data handle
	 *  @param address relative or absolute address in TES
	 *  @param optional optional input
	 *  @param accessType read, write or update
	 */

	template<class T>
	StatusCode declareInput(const std::string& propertyName, DataObjectHandle<T> & handle,
			const std::string& address = DataObjectDescriptor::NULL_,
			bool optional = false, MinimalDataObjectHandle::AccessType accessType =
					MinimalDataObjectHandle::READ) {

		bool res = m_inputDataObjects.insert(propertyName, &handle);

		handle.descriptor()->setTag(propertyName);
		handle.descriptor()->setAddress(address);
		handle.descriptor()->setAccessType(accessType);
		handle.descriptor()->setOptional(optional);

		handle.setOwner(this);


		if (LIKELY(res)) {
			debug() << "Handle for " << propertyName << " ("
					<< address << ")" << " successfully created and stored."
					<< endmsg;
		} else {
			error() << "Handle for " << propertyName << " ("
					<< address << ")" << " could not be created." << endmsg;
		}

		return res;

	}

	/** Declare input data object
	 *
	 *  @param propertyName to identify input object in python config
     *  @param handle data handle
	 *  @param addresses relative or absolute addresses in TES, first is main address
	 *  @param optional optional input
	 *  @param accessType read, write or update
	 */

	template<class T>
	StatusCode declareInput(const std::string& propertyName, DataObjectHandle<T> & handle,
			const std::vector<std::string>& addresses,
			bool optional = false, MinimalDataObjectHandle::AccessType accessType =
					MinimalDataObjectHandle::READ) {

		bool res = m_inputDataObjects.insert(propertyName, &handle);

		handle.descriptor()->setTag(propertyName);
		handle.descriptor()->setAddresses(addresses);
		handle.descriptor()->setAccessType(accessType);
		handle.descriptor()->setOptional(optional);

		handle.setOwner(this);


		if (LIKELY(res)) {
			debug() << "Handle for " << propertyName << " ("
					<< (addresses.empty() ? DataObjectDescriptor::NULL_ : addresses[0]) << ")" << " successfully created and stored."
					<< endmsg;
		} else {
			error() << "Handle for " << propertyName << " ("
					<< (addresses.empty() ? DataObjectDescriptor::NULL_ : addresses[0]) << ")" << " could not be created." << endmsg;
		}

		return res;

	}

	/** Declare output data object
	 *
	 *  @param propertyName to identify input object in python config
	 *  @param handle data handle
	 *  @param address relative or absolute address in TES
	 *  @param optional optional input
	 *  @param accessType write or update
	 */
	template<class T>
	StatusCode declareOutput(const std::string& propertyName, DataObjectHandle<T> & handle,
			const std::string& address = DataObjectDescriptor::NULL_,
			bool optional = false,
			MinimalDataObjectHandle::AccessType accessType = MinimalDataObjectHandle::WRITE) {

		bool res = m_outputDataObjects.insert(propertyName, &handle);

		handle.descriptor()->setTag(propertyName);
		handle.descriptor()->setAddress(address);
		handle.descriptor()->setAccessType(accessType);
		handle.descriptor()->setOptional(optional);

		handle.setOwner(this);


		if (LIKELY(res)) {
			debug() << "Handle for " << propertyName << " ("
					<< address << ")" << " successfully created and stored."
					<< endmsg;
		} else {
			error() << "Handle for " << propertyName << " ("
					<< address << ")" << " could not be created." << endmsg;
		}

		return res;

	}

	/** get inputs
	 *  @return DataObjectDescriptorCollection of inputs
	 */

	const DataObjectDescriptorCollection & inputDataObjects() const override {
		return m_inputDataObjects;
	}


	/** get outputs
	 *  @return DataObjectDescriptorCollection of outputs
	 */
	const DataObjectDescriptorCollection & outputDataObjects() const override {
		return m_outputDataObjects;
	}

	void registerTool(IAlgTool * tool) const {

		debug() << "Registering tool " << tool->name() << endmsg;
		m_tools.push_back(tool);
	}

	void deregisterTool(IAlgTool * tool) const {
		auto it = std::find(m_tools.begin(), m_tools.end(), tool);
		if (it != m_tools.end()) {
			debug() << "De-Registering tool " << tool->name()
					<< endmsg;
			m_tools.erase(it);
		} else {
			debug() << "Could not de-register tool " << tool->name()
					<< endmsg;
		}
	}

	/** Declare used public tool
	 *
	 *  @param handle ToolHandle<T>
	 *  @param toolTypeAndName
	 *  @param parent, default public tool
	 *  @param create if necessary, default true
	 */
	template<class T>
	StatusCode declarePublicTool(ToolHandle<T> & handle, std::string toolTypeAndName =
			"", bool createIf = true) {

		if (toolTypeAndName.empty())
			toolTypeAndName = System::typeinfoName(typeid(T));

		StatusCode sc = handle.initialize(toolTypeAndName, 0, createIf);

		m_toolHandles.push_back(&handle);

		if (sc.isSuccess()) {
			debug() << "Handle for public tool" << toolTypeAndName
					<< " successfully created and stored." << endmsg;
		} else {

			error() << "Handle for public tool" << toolTypeAndName
					<< " could not be created." << endmsg;
		}

		return sc;

	}

	/** Declare used private tool
	 *
	 *  @param handle ToolHandle<T>
	 *  @param toolTypeAndName
	 *  @param parent, default public tool
	 *  @param create if necessary, default true
	 */
	template<class T>
	StatusCode declarePrivateTool(ToolHandle<T> & handle, std::string toolTypeAndName =
			"", bool createIf = true) {

		if (toolTypeAndName.empty())
			toolTypeAndName = System::typeinfoName(typeid(T));

		StatusCode sc = handle.initialize(toolTypeAndName, this, createIf);

		m_toolHandles.push_back(&handle);


		if (sc.isSuccess()) {
			debug() << "Handle for private tool" << toolTypeAndName
					<< " successfully created and stored." << endmsg;
		} else {

			error() << "Handle for private tool" << toolTypeAndName
					<< " could not be created." << endmsg;
		}

		return sc;

	}

	const std::vector<IAlgTool *> & tools() const;

protected:

	  DataObjectDescriptorCollection & inputDataObjects() {
		  return m_inputDataObjects;
	  }
	  DataObjectDescriptorCollection & outputDataObjects() {
		  return m_outputDataObjects;
	  }

	  std::vector<IAlgTool *> & tools();

private:
   //place IAlgTools defined via ToolHandles in m_tools
   void initToolHandles() const;

public:

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
    if ( !m_pMonitorSvc ) m_pMonitorSvc = service(m_monitorSvcName, false, true);
    return m_pMonitorSvc.get();
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
  int           outputLevel () const { return m_outputLevel.value() ; }

  /// Accessor for the Message level property
  IntegerProperty & outputLevelProperty() { return m_outputLevel; }


protected:

  // Standard destructor.
  ~AlgTool() override;

private:
  typedef std::list<std::pair<InterfaceID,void*> >  InterfaceList;
  IntegerProperty      m_outputLevel = MSG::NIL;      ///< AlgTool output level
  std::string          m_type;             ///< AlgTool type (concrete class name)
  const std::string    m_name;             ///< AlgTool full name
  const IInterface*    m_parent = nullptr;           ///< AlgTool parent
  mutable SmartIF<ISvcLocator> m_svcLocator ;       ///< Pointer to Service Locator service
  mutable SmartIF<IDataProviderSvc> m_evtSvc;      ///< Event data service
  mutable SmartIF<IToolSvc> m_ptoolSvc;         ///< Tool service
  mutable SmartIF<IMonitorSvc> m_pMonitorSvc;      ///< Online Monitoring Service
  std::string          m_monitorSvcName;   ///< Name to use for Monitor Service
  SmartIF<PropertyMgr> m_propertyMgr;      ///< Property Manager
  InterfaceList        m_interfaceList;    ///< Interface list
  std::string          m_threadID;         ///< Thread Id for Alg Tool

  DataObjectDescriptorCollection m_inputDataObjects; //input data objects
  DataObjectDescriptorCollection m_outputDataObjects; //output data objects

  //tools used by tool
  mutable std::vector<IAlgTool *> m_tools;
  mutable std::vector<BaseToolHandle *> m_toolHandles;
  mutable bool m_toolHandlesInit = false;  /// flag indicating whether ToolHandle tools have been added to m_tools

  /** implementation of service method */
  StatusCode service_i(const std::string& algName,
                       bool createIf,
                       const InterfaceID& iid,
                       void** ppSvc) const;
  StatusCode service_i(const std::string& svcType,
                       const std::string& svcName,
                       const InterfaceID& iid,
                       void** ppS) const;

  mutable SmartIF<IAuditorSvc> m_pAuditorSvc; ///< Auditor Service

  BooleanProperty m_auditInit = false;
  bool         m_auditorInitialize = false;///< flag for auditors in "initialize()"
  bool         m_auditorStart = false;     ///< flag for auditors in "start()"
  bool         m_auditorStop = false;      ///< flag for auditors in "stop()"
  bool         m_auditorFinalize = false;  ///< flag for auditors in "finalize()"
  bool         m_auditorReinitialize = false;///< flag for auditors in "reinitialize()"
  bool         m_auditorRestart = false;     ///< flag for auditors in "restart()"

  Gaudi::StateMachine::State m_state = Gaudi::StateMachine::CONFIGURED ;            ///< state of the Tool
  Gaudi::StateMachine::State m_targetState = Gaudi::StateMachine::CONFIGURED ;      ///< state of the Tool
};


#ifndef GAUDI_NEW_PLUGIN_SERVICE
template <class T>
struct ToolFactory {
  template <typename S, typename... Args>
  static typename S::ReturnType create(Args&&... args) {
    return new T(std::forward<Args>(args)...);
  }
};

// Macros to declare component factories
#define DECLARE_TOOL_FACTORY(x) \
  DECLARE_FACTORY_WITH_CREATOR(x, ToolFactory< x >, AlgTool::Factory)
#define DECLARE_NAMESPACE_TOOL_FACTORY(n, x) \
    DECLARE_TOOL_FACTORY(n::x)

#else

// Macros to declare component factories
#define DECLARE_TOOL_FACTORY(x)              DECLARE_COMPONENT(x)
#define DECLARE_NAMESPACE_TOOL_FACTORY(n,x)  DECLARE_COMPONENT(n::x)

#endif


#endif // GAUDIKERNEL_ALGTOOL_H
