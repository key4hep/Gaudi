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

#include "GaudiKernel/IDataHandleHolder.h"
#include "GaudiKernel/DataHandle.h"

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
class GAUDI_API AlgTool: public CommonMessaging<implements<IAlgTool,
                                                           IDataHandleHolder,
                                                           IProperty,
                                                           IStateful>>,
                         public PropertyMgr {
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

public:
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

  using PropertyMgr::declareProperty;

  template<class T>

    Property* declareProperty(const std::string& name,
                              ToolHandle<T>& hndl,
                              const std::string& doc = "none" ) {
    this->declareTool(hndl).ignore();
    return PropertyMgr::declareProperty(name, hndl, doc);

  }

  template<class T>
    StatusCode declareTool(ToolHandle<T> &handle,
                           std::string toolTypeAndName = "",
                           bool createIf = true) {
    if (handle.isPublic()) {
      return declarePublicTool(handle, toolTypeAndName, createIf);
    } else {
      return declarePrivateTool(handle, toolTypeAndName, createIf);
    }
  }

  // ==========================================================================
  // declare ToolHandleArrays to the AlgTool
  template <class T>
    Property* declareProperty(const std::string& name,
                              ToolHandleArray<T>& hndlArr,
                              const std::string& doc = "none" ) {
    m_toolHandleArrays.push_back(&hndlArr);
    return PropertyMgr::declareProperty(name, hndlArr, doc);
  }

 protected:
  virtual void declareInput(Gaudi::DataHandle* im) override {
    m_inputHandles.push_back(im);
  }

  virtual void declareOutput(Gaudi::DataHandle* im) override {
    m_outputHandles.push_back(im);
		}
 public:

  virtual std::vector<Gaudi::DataHandle*> inputHandles() const override {
    return m_inputHandles; }
  virtual std::vector<Gaudi::DataHandle*> outputHandles() const override {
    return m_outputHandles; }

  virtual const DataObjIDColl& extraInputDeps() const override {
    return m_extInputDataObjs;
  }
  virtual const DataObjIDColl& extraOutputDeps() const override {
    return m_extOutputDataObjs;
	}


  virtual void acceptDHVisitor(IDataHandleVisitor*) const override;

  DataObjIDColl inputDataObjs() const { return m_inputDataObjs; }
  DataObjIDColl outputDataObjs() const { return m_outputDataObjs; }

  void commitHandles() override;


 private:
  std::vector<Gaudi::DataHandle*> m_inputHandles, m_outputHandles;
  DataObjIDColl m_inputDataObjs, m_outputDataObjs;

  DataObjIDColl m_extInputDataObjs, m_extOutputDataObjs;

 public:



	void registerTool(IAlgTool * tool) const {
		if (UNLIKELY(msgLevel(MSG::DEBUG)))
		  debug() << "Registering tool " << tool->name() << endmsg;
		m_tools.push_back(tool);
	}

	void deregisterTool(IAlgTool * tool) const {
		auto it = std::find(m_tools.begin(), m_tools.end(), tool);
		if (it != m_tools.end()) {
		        if (UNLIKELY(msgLevel(MSG::DEBUG)))
			  debug() << "De-Registering tool " << tool->name() << endmsg;
			m_tools.erase(it);
		} else {
		        if (UNLIKELY(msgLevel(MSG::DEBUG)))
			  debug() << "Could not de-register tool " << tool->name() << endmsg;
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
    StatusCode declarePublicTool(ToolHandle<T> & handle,
                                 std::string toolTypeAndName = "",
                                 bool createIf = true) {

		if (toolTypeAndName == "")
      toolTypeAndName = handle.typeAndName();

		StatusCode sc = handle.initialize(toolTypeAndName, 0, createIf);
		if (UNLIKELY(!sc)) {
		      throw GaudiException{"Cannot create handle for public tool " + toolTypeAndName,
		                           name(), sc};
		}

		m_toolHandles.push_back(&handle);

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
    StatusCode declarePrivateTool(ToolHandle<T> & handle,
                                  std::string toolTypeAndName = "",
                                  bool createIf = true) {

		if (toolTypeAndName == "")
      toolTypeAndName = handle.typeAndName();

		StatusCode sc = handle.initialize(toolTypeAndName, this, createIf);
		if (UNLIKELY(!sc)) {
		      throw GaudiException{"Cannot create handle for private tool " + toolTypeAndName,
		                           name(), sc};
		}

		m_toolHandles.push_back(&handle);

		return sc;
	}

	const std::vector<IAlgTool *> & tools() const;

protected:

	  std::vector<IAlgTool *> & tools();

	  /// Hook for for derived classes to provide a custom visitor for data handles.
	  std::unique_ptr<IDataHandleVisitor> m_updateDataHandles;

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
  InterfaceList        m_interfaceList;    ///< Interface list
  std::string          m_threadID;         ///< Thread Id for Alg Tool

  //tools used by tool
  mutable std::vector<IAlgTool *> m_tools;
  mutable std::vector<BaseToolHandle *> m_toolHandles;
  mutable std::vector<GaudiHandleArrayBase*> m_toolHandleArrays;
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
