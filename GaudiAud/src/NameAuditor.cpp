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
// NameAuditor:
//  An auditor that prints the name of each algorithm method before
// and after it is called///

#include "CommonAuditor.h"

/**
 * @brief Prints the name of each algorithm before entering the algorithm and after leaving it
 * @author M. Shapiro, LBNL
 * @author Marco Clemencic
 */
struct NameAuditor : CommonAuditor {
  using CommonAuditor::CommonAuditor;

  /// Print a message on "before".
  void i_before( CustomEventTypeRef evt, std::string_view caller ) override {
    info() << "About to Enter " << caller << " with auditor trigger " << evt << endmsg;
  }
  /// Print a message on "after".
  void i_after( CustomEventTypeRef evt, std::string_view caller, const StatusCode& ) override {
    info() << "Just Exited " << caller << " with auditor trigger " << evt << endmsg;
  }
};

DECLARE_COMPONENT( NameAuditor )
