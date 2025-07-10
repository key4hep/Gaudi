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

#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/INamedInterface.h>
#include <GaudiKernel/StatusCode.h>

#include <string>

namespace Gaudi {

  /**
   * The IAuditor is the interface implemented by the Auditor base class.
   * Concrete auditors, derived from the Auditor base class are controlled
   * via this interface.
   */
  class GAUDI_API IAuditor : virtual public INamedInterface {
  public:
    DeclareInterfaceID( IAuditor, 1, 0 );

    /// Audit the start of a given "event" for a given call
    virtual void before( std::string const& event, std::string const& caller, EventContext const& ) = 0;

    /// Audit the end of a given "event" for a given caller
    virtual void after( std::string const& event, std::string const& caller, EventContext const&,
                        StatusCode const& sc = StatusCode::SUCCESS ) = 0;

    /// Tell if the auditor is enabled or not.
    virtual bool isEnabled() const = 0;

    /// Used by AuditorSvc.
    virtual StatusCode sysInitialize() = 0;

    /// Used by AuditorSvc.
    virtual StatusCode sysFinalize() = 0;

    // Defining a set of standard events used internally by the framework
    inline static const std::string Initialize   = "Initialize";
    inline static const std::string ReInitialize = "ReInitialize";
    inline static const std::string Start        = "Start";
    inline static const std::string ReStart      = "ReStart";
    inline static const std::string Execute      = "Execute";
    inline static const std::string Stop         = "Stop";
    inline static const std::string Finalize     = "Finalize";
  };

} // namespace Gaudi
