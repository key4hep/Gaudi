/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once
// ============================================================================
// Include files
// ============================================================================
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/IStateful.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ITimelineSvc.h"

#include <string>
#include <vector>

// Extra include files (forward declarations should be sufficient)
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/CounterHolder.h"
#include "GaudiKernel/DataObjID.h" // must be include before Property.h, which is included in PropertyHolder.h
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IAuditorSvc.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IExceptionSvc.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/IMonitorSvc.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/PropertyHolder.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/ToolHandle.h"
#include <Gaudi/PluginService.h>

// For concurrency
#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/DataHandleHolderBase.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgExecStateSvc.h"

class IAlgTool;
class ToolHandleInfo;
class AlgorithmManager;

#ifndef PACKAGE_VERSION
#  define PACKAGE_VERSION "unknown"
#endif
namespace Gaudi {
  namespace Details {
    bool getDefaultAuditorValue( ISvcLocator* loc );
  }

  /** Base class from which all concrete algorithm classes should
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
  class GAUDI_API Algorithm
      : public DataHandleHolderBase<CounterHolder<
            PropertyHolder<CommonMessaging<implements<IAlgorithm, IDataHandleHolder, IProperty, IStateful>>>>> {
  public:
#ifndef __REFLEX__
    typedef Gaudi::PluginService::Factory<IAlgorithm*( const std::string&, ISvcLocator* )> Factory;
#endif
    friend AlgorithmManager;

    /** Constructor
     *  @param name    The algorithm object's name
     *  @param svcloc  A pointer to a service location service
     */
    Algorithm( const std::string& name, ISvcLocator* svcloc, const std::string& version = PACKAGE_VERSION );

    /** Reinitialization method invoked by the framework. This method is responsible
     *  for any reinitialization required by the framework itself.
     *  It will in turn invoke the reinitialize() method of the derived algorithm,
     * and of any sub-algorithms which it creates.
     */
    StatusCode sysStart() override;

    /** Initialization method invoked by the framework. This method is responsible
     *  for any bookkeeping of initialization required by the framework itself.
     *  It will in turn invoke the initialize() method of the derived algorithm,
     * and of any sub-algorithms which it creates.
     */
    StatusCode sysInitialize() override;

    /** Reinitialization method invoked by the framework. This method is responsible
     *  for any reinitialization required by the framework itself.
     *  It will in turn invoke the reinitialize() method of the derived algorithm,
     * and of any sub-algorithms which it creates.
     */
    StatusCode sysReinitialize() override;

    /** Restart method invoked by the framework.
        It will in turn invoke the restart() method of the derived algorithm,
        and of any sub-algorithms which it creates.
    */
    StatusCode sysRestart() override;

    /** The actions to be performed by the algorithm on an event. This method is
     * invoked once per event for top level algorithms by the application
     *  manager.
     *  This method invokes execute() method.
     *  For sub-algorithms either the sysExecute() method or execute() method
     *  must be EXPLICITLY invoked by  the parent algorithm.
     */
    StatusCode sysExecute( const EventContext& ctx ) override;

    /** System stop. This method invokes the stop() method of a concrete
        algorithm and the stop() methods of all of that algorithm's sub algorithms.
    */
    StatusCode sysStop() override;

    /** System finalization. This method invokes the finalize() method of a
     *  concrete algorithm and the finalize() methods of all of that algorithm's
     *  sub algorithms.
     */
    StatusCode sysFinalize() override;

    /** The identifying name of the algorithm object. This is the name of a
     *  particular instantiation of an algorithm object as opposed to the name
     *  of the algorithm itself, e.g. "LinearTrackFit" may be the name of a
     *  concrete algorithm class,
     *  whereas "ApproxTrackFit" and "BestTrackFit" may be two instantiations
     *  of the class configured to find tracks with different fit criteria.
     */
    const std::string&      name() const override;
    const Gaudi::StringKey& nameKey() const override;

    /** The type of the algorithm object.
     */
    const std::string& type() const override { return m_type; }
    void               setType( const std::string& type ) override { m_type = type; } // BH, TODO: move to proper place

    const std::string& version() const override;

    unsigned int index() const override;

    /// Dummy implementation of IStateful::configure() method
    StatusCode configure() override { return StatusCode::SUCCESS; }
    /// Dummy implementation of IStateful::terminate() method
    StatusCode terminate() override { return StatusCode::SUCCESS; }

    /// the default (empty) implementation of IStateful::initialize() method
    StatusCode initialize() override { return StatusCode::SUCCESS; }
    /// the default (empty) implementation of IStateful::start() method
    StatusCode start() override { return StatusCode::SUCCESS; }
    /// the default (empty) implementation of IStateful::stop() method
    StatusCode stop() override { return StatusCode::SUCCESS; }
    /// the default (empty) implementation of IStateful::finalize() method
    StatusCode finalize() override { return StatusCode::SUCCESS; }
    /// the default (empty) implementation of IStateful::reinitialize() method
    StatusCode reinitialize() override;
    /// the default (empty) implementation of IStateful::restart() method
    StatusCode restart() override;
    /// returns the current state of the algorithm
    Gaudi::StateMachine::State FSMState() const override { return m_state; }
    /// returns the state the algorithm will be in after the ongoing transition
    Gaudi::StateMachine::State targetFSMState() const override { return m_targetState; }

    /// Is this algorithm enabled or disabled?
    bool isEnabled() const override;

    /// Are we a Sequence?
    bool isSequence() const override { return false; }

    /// Get the number of failures of the algorithm.
    unsigned int errorCount() const;

    /// Access a service by name, creating it if it doesn't already exist.
    template <class T>
    StatusCode service( const std::string& name, T*& psvc, bool createIf = true ) const {
      return service_i( name, createIf, T::interfaceID(), (void**)&psvc );
    }

    /// Access a service by name and type, creating it if it doesn't already exist.
    template <class T>
    StatusCode service( const std::string& svcType, const std::string& svcName, T*& psvc ) const {
      return service_i( svcType, svcName, T::interfaceID(), reinterpret_cast<void**>( &psvc ) );
    }

    /// Return a pointer to the service identified by name (or "type/name")
    SmartIF<IService> service( const std::string& name, const bool createIf = true, const bool quiet = false ) const;

    template <class T>
    SmartIF<T> service( const std::string& name, bool createIf = true, bool quiet = false ) const {
      return service( name, createIf, quiet ).as<T>();
    }

    /** The standard auditor service.May not be invoked before sysInitialize()
     *  has been invoked.
     */
    SmartIF<IAuditorSvc>& auditorSvc() const;

    /** The standard Chrono & Stat service,
     *  Return a pointer to the service if present
     */
    SmartIF<IChronoStatSvc>&                                             chronoSvc() const;
    [[deprecated( "use chronoSvc() instead" )]] SmartIF<IChronoStatSvc>& chronoStatService() const {
      return chronoSvc();
    }

    /** The standard detector data service.
     *  May not be invoked before sysInitialize() has been invoked.
     */
    SmartIF<IDataProviderSvc>&                                          detSvc() const;
    [[deprecated( "use detSvc() instead" )]] SmartIF<IDataProviderSvc>& detDataService() const { return detSvc(); }

    /** The standard detector data persistency conversion service.
     *  May not be invoked before sysInitialize() has been invoked.
     */
    SmartIF<IConversionSvc>&                                             detCnvSvc() const;
    [[deprecated( "use detCnvSvc() instead" )]] SmartIF<IConversionSvc>& detDataCnvService() const {
      return detCnvSvc();
    }

    /** The standard event data service.
     *  May not be invoked before sysInitialize() has been invoked.
     */
    SmartIF<IDataProviderSvc>& eventSvc() const;
    /// shortcut for  method eventSvc
    SmartIF<IDataProviderSvc>&                                            evtSvc() const { return eventSvc(); }
    [[deprecated( "use eventSvc() instead" )]] SmartIF<IDataProviderSvc>& eventDataService() const {
      return eventSvc();
    }

    /** The standard event data persistency conversion service.
     *  May not be invoked before sysInitialize() has been invoked.
     */
    SmartIF<IConversionSvc>&                                               eventCnvSvc() const;
    [[deprecated( "use eventCnvSvc() instead" )]] SmartIF<IConversionSvc>& eventDataCnvService() const {
      return eventCnvSvc();
    }

    /** The standard histogram service.
     *  May not be invoked before sysInitialize() has been invoked.
     */
    SmartIF<IHistogramSvc>&                                            histoSvc() const;
    [[deprecated( "use histoSvc() instead" )]] SmartIF<IHistogramSvc>& histogramDataService() const {
      return histoSvc();
    }

    /** The standard N tuple service.
     *  Returns a pointer to the N tuple service if present.
     */
    SmartIF<INTupleSvc>&                                             ntupleSvc() const;
    [[deprecated( "use ntupleSvc() instead" )]] SmartIF<INTupleSvc>& ntupleService() const { return ntupleSvc(); }

    /** The standard RandomGen service,
     *  Return a pointer to the service if present
     */
    SmartIF<IRndmGenSvc>& randSvc() const;

    /// The standard ToolSvc service, Return a pointer to the service if present
    SmartIF<IToolSvc>& toolSvc() const;

    /// Get the exception Service
    SmartIF<IExceptionSvc>& exceptionSvc() const;

    /// get Algorithm Context Service
    SmartIF<IAlgContextSvc>& contextSvc() const;

    SmartIF<ITimelineSvc>& timelineSvc() const;

    /** The standard service locator.
     *  Returns a pointer to the service locator service.
     *  This service may be used by an algorithm to request
     *  any services it requires in addition to those provided by default.
     */
    SmartIF<ISvcLocator>& serviceLocator() const override;
    /// shortcut for method serviceLocator
    SmartIF<ISvcLocator>& svcLoc() const { return serviceLocator(); }

    SmartIF<IHiveWhiteBoard>& whiteboard() const;

    SmartIF<IAlgExecStateSvc>& algExecStateSvc() const;

    /// register for Algorithm Context Service?
    bool registerContext() const { return m_registerContext; }

    /** Set the algorithm's properties.
     *  This method requests the job options service
     *  to set the values of any declared properties.
     *  The method is invoked from within sysInitialize() by the framework
     *  and does not need to be explicitly
     *  called by a concrete algorithm.
     */
    StatusCode setProperties();

    // ==========================================================================
    using PropertyHolderImpl::declareProperty;

    // declare Tools to the Algorithms
    template <class T>
    Gaudi::Details::PropertyBase* declareProperty( const std::string& name, ToolHandle<T>& hndl,
                                                   const std::string& doc = "none" ) {
      this->declareTool( hndl, hndl.typeAndName() ).ignore();
      return PropertyHolderImpl::declareProperty( name, hndl, doc );
    }

    // ==========================================================================
    // declare ToolHandleArrays to the Algorithms

    template <class T>
    Gaudi::Details::PropertyBase* declareProperty( const std::string& name, ToolHandleArray<T>& hndlArr,
                                                   const std::string& doc = "none" ) {
      addToolsArray( hndlArr );
      return PropertyHolderImpl::declareProperty( name, hndlArr, doc );
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
    inline SmartIF<IMonitorSvc>& monitorSvc() const {
      // If not already located try to locate it without forcing a creation
      if ( !m_pMonitorSvc ) {
        m_pMonitorSvc = service( m_monitorSvcName, false, true ); // do not create and be quiet
      }
      return m_pMonitorSvc;
    }

    /** Declare monitoring information
        @param name Monitoring information name known to the external system
        @param var  Monitoring Listener address (the item to monitor...)
        @param desc Textual description of the information being monitored
    */
    template <class T>
    void declareInfo( const std::string& name, const T& var, const std::string& desc ) const {
      IMonitorSvc* mS = monitorSvc().get();
      if ( mS ) mS->declareInfo( name, var, desc, this );
    }

    /** Declare monitoring information (special case)
        @param name Monitoring information name known to the external system
        @param format Format information
        @param var  Monitoring Listener address
        @param size Monitoring Listener address size
        @param desc Textual description of the information being monitored
    */
    void declareInfo( const std::string& name, const std::string& format, const void* var, int size,
                      const std::string& desc ) const {
      IMonitorSvc* mS = monitorSvc().get();
      if ( mS ) mS->declareInfo( name, format, var, size, desc, this );
    }

  public:
    void acceptDHVisitor( IDataHandleVisitor* ) const override;

  public:
    void registerTool( IAlgTool* tool ) const;
    void deregisterTool( IAlgTool* tool ) const;

    template <class T>
    StatusCode declareTool( ToolHandle<T>& handle, bool createIf = true ) {
      return this->declareTool( handle, handle.typeAndName(), createIf );
    }

    template <class T>
    StatusCode declareTool( ToolHandle<T>& handle, std::string toolTypeAndName, bool createIf = true ) {

      StatusCode sc = handle.initialize( toolTypeAndName, handle.isPublic() ? nullptr : this, createIf );
      if ( UNLIKELY( !sc ) ) {
        throw GaudiException{std::string{"Cannot create handle for "} + ( handle.isPublic() ? "public" : "private" ) +
                                 " tool " + toolTypeAndName,
                             name(), sc};
      }

      m_toolHandles.push_back( &handle );

      return sc;
    }

    template <class T>
    void addToolsArray( ToolHandleArray<T>& hndlArr ) {
      m_toolHandleArrays.push_back( &hndlArr );
    }

    const std::vector<IAlgTool*>& tools() const;

    // Return the I/O-boundness flag
    bool isIOBound() const { return m_isIOBound; }
    // Set the I/O-boundness flag
    void setIOBound( bool value ) { m_isIOBound = value; }

  protected:
    std::vector<IAlgTool*>& tools();

    // // adds declared in- and outputs of subAlgorithms to own DOHs
    //  void addSubAlgorithmDataObjectHandles();

  private:
    // place IAlgTools defined via ToolHandles in m_tools
    void initToolHandles() const;

  public:
    // /// Specifies the clonability of the algorithm
    // bool isClonable() const override { return false; }

    /// Return the cardinality
    unsigned int cardinality() const override { return m_cardinality; }

    const std::vector<std::string>& neededResources() const override { return m_neededResources; }

  protected:
    /// Has the Algorithm already been initialized?
    bool isInitialized() const override { return Gaudi::StateMachine::INITIALIZED == m_state; }

    /// Has the Algorithm already been finalized?
    bool isFinalized() const override { return Gaudi::StateMachine::CONFIGURED == m_state; }

    /// set instantiation index of Alg
    void setIndex( const unsigned int& idx ) override;

  public:
    /// reference to AlgExecState of Alg
    AlgExecState& execState( const EventContext& ctx ) const override;

    /// Produce string represention of the control flow expression.
    std::ostream& toControlFlowExpression( std::ostream& os ) const override;

  private:
    unsigned int maxErrors() const { return m_errorMax; }

  protected:
    bool isReEntrant() const override { return true; }

  private:
    Gaudi::StringKey m_name;      ///< Algorithm's name for identification
    std::string      m_type;      ///< Algorithm's type
    std::string      m_version;   ///< Algorithm's version
    unsigned int     m_index = 0; ///< Algorithm's index

    // tools used by algorithm
    mutable std::vector<IAlgTool*>             m_tools;
    mutable std::vector<BaseToolHandle*>       m_toolHandles;
    mutable std::vector<GaudiHandleArrayBase*> m_toolHandleArrays;

  private:
    template <typename IFace>
    SmartIF<IFace>& get_svc_( SmartIF<IFace>& p, const char* service_name ) const;

    mutable SmartIF<IMessageSvc>      m_MS;          ///< Message service
    mutable SmartIF<IDataProviderSvc> m_EDS;         ///< Event data service
    mutable SmartIF<IHiveWhiteBoard>  m_WB;          ///< Event data service (whiteboard)
    mutable SmartIF<IConversionSvc>   m_ECS;         ///< Event conversion service
    mutable SmartIF<IDataProviderSvc> m_DDS;         ///< Detector data service
    mutable SmartIF<IConversionSvc>   m_DCS;         ///< Detector conversion service
    mutable SmartIF<IHistogramSvc>    m_HDS;         ///< Histogram data service
    mutable SmartIF<INTupleSvc>       m_NTS;         ///< N tuple service
    mutable SmartIF<IChronoStatSvc>   m_CSS;         ///< Chrono & Stat Service
    mutable SmartIF<IRndmGenSvc>      m_RGS;         ///< Random Number Generator Service
    mutable SmartIF<IExceptionSvc>    m_EXS;         ///< Exception Handler Service
    mutable SmartIF<IAuditorSvc>      m_pAuditorSvc; ///< Auditor Service
    mutable SmartIF<IToolSvc>         m_ptoolSvc;    ///< ToolSvc Service
    mutable SmartIF<IMonitorSvc>      m_pMonitorSvc; ///< Online Monitoring Service
    mutable SmartIF<IAlgContextSvc>   m_contextSvc;  ///< Algorithm Context Service

    mutable SmartIF<ITimelineSvc>     m_timelineSvc; ///< Timeline Service
    mutable SmartIF<IAlgExecStateSvc> m_aess;        ///< Alg execution state mgr

    SmartIF<ISvcLocator> m_pSvcLocator; ///< Pointer to service locator service

  protected:
    /// Hook for for derived classes to provide a custom visitor for data handles.
    std::unique_ptr<IDataHandleVisitor> m_updateDataHandles;

  private:
    // Properties
    Gaudi::Property<int> m_outputLevel{
        this, "OutputLevel", MSG::NIL,
        [this]( Gaudi::Details::PropertyBase& ) { this->updateMsgStreamOutputLevel( this->m_outputLevel ); },
        "output level"};
    Gaudi::Property<bool> m_isEnabled{this, "Enable", true, "should the algorithm be executed or not"};

    Gaudi::Property<unsigned int> m_errorMax{this, "ErrorMax", 1, "[[deprecated]] max number of errors"};

    Gaudi::Property<bool> m_auditInit{this, "AuditAlgorithms", Details::getDefaultAuditorValue( m_pSvcLocator ),
                                      "[[deprecated]] unused"};
    Gaudi::Property<bool> m_auditorInitialize{this, "AuditInitialize", m_auditInit.value(),
                                              "trigger auditor on initialize()"};
    Gaudi::Property<bool> m_auditorReinitialize{this, "AuditReinitialize", m_auditInit.value(),
                                                "trigger auditor on reinitialize()"};
    Gaudi::Property<bool> m_auditorRestart{this, "AuditRestart", m_auditInit.value(), "trigger auditor on restart()"};
    Gaudi::Property<bool> m_auditorExecute{this, "AuditExecute", m_auditInit.value(), "trigger auditor on execute()"};
    Gaudi::Property<bool> m_auditorFinalize{this, "AuditFinalize", m_auditInit.value(),
                                            "trigger auditor on finalize()"};
    Gaudi::Property<bool> m_auditorStart{this, "AuditStart", m_auditInit.value(), "trigger auditor on start()"};
    Gaudi::Property<bool> m_auditorStop{this, "AuditStop", m_auditInit.value(), "trigger auditor on stop()"};

    Gaudi::Property<bool> m_doTimeline{this, "Timeline", true, "send events to TimelineSvc"};

    Gaudi::Property<std::string> m_monitorSvcName{this, "MonitorService", "MonitorSvc",
                                                  "name to use for Monitor Service"};

    Gaudi::Property<bool> m_registerContext{this, "RegisterForContextService", false,
                                            "flag to enforce the registration for Algorithm Context Service"};

    Gaudi::Property<int> m_cardinality{this, "Cardinality", 0, "how many clones to create - 0 means algo is reentrant"};
    Gaudi::Property<std::vector<std::string>> m_neededResources{
        this, "NeededResources", {}, "named resources needed during event looping"};

    Gaudi::Property<bool> m_isIOBound{this, "IsIOBound", false,
                                      "if the algorithm is I/O-bound (in the broad sense of Von Neumann bottleneck)"};

    // The default should be changed to "false" for v29
    Gaudi::Property<bool> m_filterCircDeps{this, "FilterCircularDependencies", true,
                                           "filter out circular data dependencies"};

    mutable bool m_toolHandlesInit = false; /// flag indicating whether ToolHandle tools have been added to m_tools

    Gaudi::StateMachine::State m_state       = Gaudi::StateMachine::CONFIGURED; ///< Algorithm has been initialized flag
    Gaudi::StateMachine::State m_targetState = Gaudi::StateMachine::CONFIGURED; ///< Algorithm has been initialized flag
    bool                       m_isFinalized;                                   ///< Algorithm has been finalized flag

    /// implementation of service method
    StatusCode service_i( const std::string& svcName, bool createIf, const InterfaceID& iid, void** ppSvc ) const;
    StatusCode service_i( const std::string& svcType, const std::string& svcName, const InterfaceID& iid,
                          void** ppSvc ) const;

    /// Private Copy constructor: NO COPY ALLOWED
    Algorithm( const Algorithm& a ) = delete;

    /// Private assignment operator: NO ASSIGNMENT ALLOWED
    Algorithm& operator=( const Algorithm& rhs ) = delete;
  };
} // namespace Gaudi
