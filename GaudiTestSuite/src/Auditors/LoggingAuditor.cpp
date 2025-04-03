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
#include <Gaudi/Auditor.h>
#include <GaudiKernel/MsgStream.h>

/** @class LoggingAuditor
 *
 *  Simple auditor that prints the event being audited.
 *
 *  @author Marco Clemencic
 *  @date 2008-04-03
 */
namespace GaudiTestSuite {

  class LoggingAuditor : public Gaudi::Auditor {
  public:
    /// constructor
    using Auditor::Auditor;

    void before( std::string const& evt, std::string const& caller, EventContext const& ) override {
      info() << "Auditing before of " << evt << " for " << caller << endmsg;
    }

    void after( std::string const& evt, std::string const& caller, EventContext const&, StatusCode const& ) override {
      info() << "Auditing after of " << evt << " for " << caller << endmsg;
    }
  };

  DECLARE_COMPONENT( LoggingAuditor )
} // namespace GaudiTestSuite

namespace GaudiExamples {
  struct LoggingAuditor : GaudiTestSuite::LoggingAuditor {
    using GaudiTestSuite::LoggingAuditor::LoggingAuditor;
  };
  DECLARE_COMPONENT( LoggingAuditor )
} // namespace GaudiExamples
