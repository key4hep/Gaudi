/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_SERVICE_H
#define GAUDIKERNEL_SERVICE_H
// ============================================================================
// Include files
// ============================================================================
#include <Gaudi/PluginService.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/CommonMessaging.h>
#include <GaudiKernel/IAuditorSvc.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/IService.h>
#include <GaudiKernel/IStateful.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/PropertyHolder.h>
#include <GaudiKernel/ServiceLocatorHelper.h>
#include <GaudiKernel/SmartIF.h>
#include <GaudiKernel/ToolHandle.h>

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
class GAUDI_API Service : public PropertyHolder<CommonMessaging<implements<IService, IProperty, IStateful>>> {
public:
  using Factory = Gaudi::PluginService::Factory<IService*( const std::string&, ISvcLocator* )>;

  friend class ServiceManager;

  /** Retrieve name of the service               */
  const std::string& name() const override;

  // State machine implementation
  StatusCode                 configure() override { return StatusCode::SUCCESS; }
  StatusCode                 initialize() override;
  StatusCode                 start() override;
  StatusCode                 stop() override;
  StatusCode                 finalize() override;
  StatusCode                 terminate() override { return StatusCode::SUCCESS; }
  Gaudi::StateMachine::State FSMState() const override { return m_state; }
  Gaudi::StateMachine::State targetFSMState() const override { return m_targetState; }
  StatusCode                 reinitialize() override;
  StatusCode                 restart() override;

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

  template <typename IFace = IService>
  SmartIF<IFace> service( const std::string& name, bool createIf = true ) const {
    return ServiceLocatorHelper( *serviceLocator(), *this )
        .service<IFace>( name, !createIf, // quiet
                         createIf );
  }

  // ==========================================================================
  // Tool handling

  using PropertyHolderImpl::declareProperty;

  template <class T>
  Gaudi::Details::PropertyBase* declareProperty( const std::string& name, ToolHandle<T>& hndl,
                                                 const std::string& doc = "none" ) {
    this->declareTool( hndl, hndl.typeAndName() ).ignore();
    return PropertyHolderImpl::declareProperty( name, hndl, doc );
  }

  template <class T>
  StatusCode declareTool( ToolHandle<T>& handle, bool createIf = true ) {
    return this->declareTool( handle, handle.typeAndName(), createIf );
  }

  /** Declare used tool
   *
   *  @param handle ToolHandle<T>
   *  @param toolTypeAndName
   *  @param parent, default public tool
   *  @param create if necessary, default true
   */
  template <class T>
  StatusCode declareTool( ToolHandle<T>& handle, const std::string& toolTypeAndName, bool createIf = true ) {

    StatusCode sc = handle.initialize( toolTypeAndName, handle.isPublic() ? nullptr : this, createIf );
    if ( !sc ) {
      throw GaudiException{ std::string{ "Cannot create handle for " } + ( handle.isPublic() ? "public" : "private" ) +
                                " tool " + toolTypeAndName,
                            name(), sc };
    }

    m_toolHandles.push_back( &handle );

    return sc;
  }

  // declare ToolHandleArrays to the AlgTool
  template <class T>
  Gaudi::Details::PropertyBase* declareProperty( const std::string& name, ToolHandleArray<T>& hndlArr,
                                                 const std::string& doc = "none" ) {
    addToolsArray( hndlArr );
    return PropertyHolderImpl::declareProperty( name, hndlArr, doc );
  }

  template <class T>
  void addToolsArray( ToolHandleArray<T>& hndlArr ) {
    m_toolHandleArrays.push_back( &hndlArr );
  }

  const std::vector<IAlgTool*>& tools() const;

protected:
  std::vector<IAlgTool*>& tools();

private:
  // place IAlgTools defined via ToolHandles in m_tools
  void initToolHandles() const;

public:
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
  void           sysInitialize_imp();
  StatusCode     m_initSC;
  std::once_flag m_initFlag;

  /** Service Name  */
  std::string m_name;
  /** Service Locator reference                  */
  mutable SmartIF<ISvcLocator> m_svcLocator;
  SmartIF<ISvcManager>         m_svcManager;

  void setServiceManager( ISvcManager* ism ) override;

  // AlgTools used by Service
  mutable std::vector<IAlgTool*>             m_tools;
  mutable std::vector<BaseToolHandle*>       m_toolHandles;
  mutable std::vector<GaudiHandleArrayBase*> m_toolHandleArrays;
  mutable bool m_toolHandlesInit = false; /// flag indicating whether ToolHandle tools have been added to m_tools

protected:
  // Properties

  Gaudi::Property<int>  m_outputLevel{ this, "OutputLevel", MSG::NIL, "output level" };
  Gaudi::Property<bool> m_auditorInitialize{ this, "AuditInitialize", false, "trigger auditor on initialize()" };
  Gaudi::Property<bool> m_auditorStart{ this, "AuditStart", false, "trigger auditor on start()" };
  Gaudi::Property<bool> m_auditorStop{ this, "AuditStop", false, "trigger auditor on stop()" };
  Gaudi::Property<bool> m_auditorFinalize{ this, "AuditFinalize", false, "trigger auditor on finalize()" };
  Gaudi::Property<bool> m_auditorReinitialize{ this, "AuditReinitialize", false, "trigger auditor on reinitialize()" };
  Gaudi::Property<bool> m_auditorRestart{ this, "AuditRestart", false, "trigger auditor on restart()" };

  Gaudi::Property<bool> m_autoRetrieveTools{ this, "AutoRetrieveTools", true,
                                             "retrieve all AlgTools during initialize" };
  Gaudi::Property<bool> m_checkToolDeps{ this, "CheckToolDeps", true,
                                         "check data dependencies of AlgTools (error if any found)" };

  /** Auditor Service                            */
  mutable SmartIF<IAuditorSvc> m_pAuditorSvc;
};

#endif // GAUDIKERNEL_SERVICE_H
