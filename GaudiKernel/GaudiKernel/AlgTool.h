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
class GAUDI_API AlgTool: public implements3<IAlgTool, IProperty, IStateful> {
  friend class ToolSvc;
public:
  typedef Gaudi::PluginService::Factory3<IAlgTool*,
                                         const std::string&,
                                         const std::string&,
                                         const IInterface*> Factory;

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

		MsgStream log(msgSvc(), name());

		if (LIKELY(res)) {
			log << MSG::DEBUG << "Handle for " << propertyName << " ("
					<< address << ")" << " successfully created and stored."
					<< endmsg;
		} else {
			log << MSG::ERROR << "Handle for " << propertyName << " ("
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

		MsgStream log(msgSvc(), name());

		if (LIKELY(res)) {
			log << MSG::DEBUG << "Handle for " << propertyName << " ("
					<< addresses[0] << ")" << " successfully created and stored."
					<< endmsg;
		} else {
			log << MSG::ERROR << "Handle for " << propertyName << " ("
					<< addresses[0] << ")" << " could not be created." << endmsg;
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
			MinimalDataObjectHandle::AccessType accessType = MinimalDataObjectHandle::WRITE,
			bool optional = false) {

		bool res = m_outputDataObjects.insert(propertyName, &handle);

		handle.descriptor()->setTag(propertyName);
		handle.descriptor()->setAddress(address);
		handle.descriptor()->setAccessType(accessType);
		handle.descriptor()->setOptional(optional);

		handle.setOwner(this);

		MsgStream log(msgSvc(), name());

		if (LIKELY(res)) {
			log << MSG::DEBUG << "Handle for " << propertyName << " ("
					<< address << ")" << " successfully created and stored."
					<< endmsg;
		} else {
			log << MSG::ERROR << "Handle for " << propertyName << " ("
					<< address << ")" << " could not be created." << endmsg;
		}

		return res;

	}

	/** get inputs
	 *  @return DataObjectDescriptorCollection of inputs
	 */

	const DataObjectDescriptorCollection & inputDataObjects() const {
		return m_inputDataObjects;
	}


	/** get outputs
	 *  @return DataObjectDescriptorCollection of outputs
	 */
	const DataObjectDescriptorCollection & outputDataObjects() const {
		return m_outputDataObjects;
	}

	void registerTool(IAlgTool * tool) const {
		MsgStream log(msgSvc(), name());

		log << MSG::DEBUG << "Registering tool " << tool->name() << endmsg;
		m_tools.push_back(tool);
	}

	void deregisterTool(IAlgTool * tool) const {
		std::vector<IAlgTool *>::iterator it = std::find(m_tools.begin(),
				m_tools.end(), tool);

		MsgStream log(msgSvc(), name());
		if (it != m_tools.end()) {
			log << MSG::DEBUG << "De-Registering tool " << tool->name()
					<< endmsg;
			m_tools.erase(it);
		} else {
			log << MSG::DEBUG << "Could not de-register tool " << tool->name()
					<< endmsg;
		}
	}

	void registerToolHandle(ToolHandleInfo * toolhandle) const {

		MsgStream log(msgSvc(), name());

		log << MSG::DEBUG << "Registering toolhandle " << endmsg;

		m_toolHandles.push_back(toolhandle);
	}

	void deregisterToolHandle(ToolHandleInfo * toolhandle) const {
		std::vector<ToolHandleInfo *>::iterator it = std::find(
				m_toolHandles.begin(), m_toolHandles.end(), toolhandle);

		MsgStream log(msgSvc(), name());
		if (it != m_toolHandles.end()) {

			log << MSG::DEBUG << "De-Registering toolhandle" << endmsg;

			m_toolHandles.erase(it);
		} else {
			log << MSG::DEBUG << "Could not de-register toolhandle" << endmsg;
		}
	}

	/** Declare used tool
	 *
	 *  @param handle ToolHandle<T>
	 *  @param toolTypeAndName
	 *  @param parent, default public tool
	 *  @param create if necessary, default true
	 */
	template<class T>
	StatusCode declareTool(ToolHandle<T> & handle, std::string toolTypeAndName =
			"", const IInterface* parent = 0, bool createIf = true) {

		if (toolTypeAndName == "")
			toolTypeAndName = System::typeinfoName(typeid(T));

		StatusCode sc = handle.initialize(toolTypeAndName, parent, createIf);

		m_toolHandles.push_back(&handle);

		MsgStream log(msgSvc(), name());

		if (sc.isSuccess()) {
			log << MSG::DEBUG << "Handle for tool" << toolTypeAndName
					<< " successfully created and stored." << endmsg;
		} else {

			log << MSG::ERROR << "Handle for tool" << toolTypeAndName
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
  mutable IDataProviderSvc* m_evtSvc; 	   ///< Event data service
  mutable IToolSvc*    m_ptoolSvc;         ///< Tool service
  mutable IMonitorSvc* m_pMonitorSvc;      ///< Online Monitoring Service
  std::string          m_monitorSvcName;   ///< Name to use for Monitor Service
  PropertyMgr*         m_propertyMgr;      ///< Property Manager
  InterfaceList        m_interfaceList;    ///< Interface list
  std::string          m_threadID;         ///< Thread Id for Alg Tool

  DataObjectDescriptorCollection m_inputDataObjects; //input data objects
  DataObjectDescriptorCollection m_outputDataObjects; //output data objects

  //tools used by tool
  mutable std::vector<IAlgTool *> m_tools;
  mutable std::vector<ToolHandleInfo *> m_toolHandles;
  mutable bool m_toolHandlesInit;  /// flag indicating whether ToolHandle tools have been added to m_tools

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


#ifndef GAUDI_NEW_PLUGIN_SERVICE
template <class T>
class ToolFactory {
public:
  template <typename S>
  static typename S::ReturnType create(typename S::Arg1Type a1,
                                       typename S::Arg2Type a2,
                                       typename S::Arg3Type a3) {
    return new T(a1, a2, a3);
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
