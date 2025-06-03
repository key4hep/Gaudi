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
#pragma once

#include <Gaudi/IAuditor.h>
#include <Gaudi/PluginService.h>
#include <Gaudi/PropertyFwd.h>
#include <GaudiKernel/CommonMessaging.h>
#include <GaudiKernel/IProperty.h>
#include <GaudiKernel/IService.h>
#include <GaudiKernel/ISvcLocator.h> /*used by service(..)*/
#include <GaudiKernel/PropertyHolder.h>
#include <string>

namespace Gaudi {

  /**
   * Base class from which all concrete auditor classes should be derived.
   * The only base class functionality which may be used in the
   * constructor of a concrete auditor is the declaration of
   * member variables as properties. All other functionality,
   * i.e. the use of services, may be used only in
   * initialize() and afterwards.
   */
  class GAUDI_API Auditor : public PropertyHolder<CommonMessaging<implements<IAuditor, IProperty>>> {
  public:
    using Factory = Gaudi::PluginService::Factory<IAuditor*( const std::string&, ISvcLocator* )>;

    Auditor( std::string name, ISvcLocator* svcloc ) : m_name( std::move( name ) ), m_pSvcLocator( svcloc ){};
    Auditor( const Auditor& a )              = delete;
    Auditor& operator=( const Auditor& rhs ) = delete;

    StatusCode sysInitialize() override;
    StatusCode sysFinalize() override;

    void before( std::string const&, std::string const&, EventContext const& ) override {}
    void after( std::string const&, std::string const&, EventContext const&,
                StatusCode const& = StatusCode::SUCCESS ) override {}

    virtual StatusCode initialize() { return StatusCode::SUCCESS; }
    virtual StatusCode finalize() { return StatusCode::SUCCESS; }

    const std::string& name() const override { return m_name; }
    bool               isEnabled() const override { return m_isEnabled; }

    /**
     * The standard service locator. Returns a pointer to the service locator service.
     * This service may be used by an auditor to request any services it requires in
     * addition to those provided by default.
     */
    SmartIF<ISvcLocator>& serviceLocator() const override { return m_pSvcLocator; }

    /// Access a service by name, creating it if it doesn't already exist.
    template <class T = IService>
    SmartIF<T> service( std::string_view name, bool createIf = false ) const {
      return serviceLocator()->service<T>( name, createIf );
    }

  private:
    std::string m_name; /// Auditor's name for identification

    mutable SmartIF<ISvcLocator> m_pSvcLocator; /// Pointer to service locator service

    Gaudi::Property<int> m_outputLevel{
        this, "OutputLevel", MSG::NIL,
        [this]( Gaudi::Details::PropertyBase& ) { this->updateMsgStreamOutputLevel( this->m_outputLevel ); },
        "output level" };
    Gaudi::Property<bool> m_isEnabled{ this, "Enable", true, "should the auditor be used or not" };

    bool m_isInitialized = false; /// Auditor has been initialized flag
    bool m_isFinalized   = false; /// Auditor has been finalized flag
  };

} // namespace Gaudi
