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
#include "GaudiKernel/Auditor.h"
#include "GaudiKernel/MsgStream.h"

// ============================================================================
/** @class LoggingAuditor
 *
 *  Simple auditor that prints the event being audited.
 *
 *  @author Marco Clemencic
 *  @date 2008-04-03
 */
namespace GaudiExamples {

  class LoggingAuditor : public Auditor {
  public:
    /// constructor
    using Auditor::Auditor;

    void before( StandardEventType evt, INamedInterface* caller ) override {
      auto& log = info();
      log << "Auditing before of " << evt;
      if ( caller ) log << " for " << caller->name();
      log << endmsg;
    }

    void after( StandardEventType evt, INamedInterface* caller, const StatusCode& ) override {
      auto& log = info();
      log << "Auditing after of " << evt;
      if ( caller ) log << " for " << caller->name();
      log << endmsg;
    }

    void before( CustomEventTypeRef evt, INamedInterface* caller ) override {
      auto& log = info();
      log << "Auditing before of " << evt;
      if ( caller ) log << " for " << caller->name();
      log << endmsg;
    }

    void after( CustomEventTypeRef evt, INamedInterface* caller, const StatusCode& ) override {
      auto& log = info();
      log << "Auditing after of " << evt;
      if ( caller ) log << " for " << caller->name();
      log << endmsg;
    }

    void before( StandardEventType evt, const std::string& caller ) override {
      info() << "Auditing before of " << evt << " for " << caller << endmsg;
    }

    void after( StandardEventType evt, const std::string& caller, const StatusCode& ) override {
      info() << "Auditing after of " << evt << " for " << caller << endmsg;
    }

    void before( CustomEventTypeRef evt, const std::string& caller ) override {
      info() << "Auditing before of " << evt << " for " << caller << endmsg;
    }

    void after( CustomEventTypeRef evt, const std::string& caller, const StatusCode& ) override {
      info() << "Auditing after of " << evt << " for " << caller << endmsg;
    }
  };

  DECLARE_COMPONENT( LoggingAuditor )
} // namespace GaudiExamples
