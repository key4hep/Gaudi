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

#include "NameAuditor.h"

#include "GaudiKernel/MsgStream.h"

DECLARE_COMPONENT( NameAuditor )

void NameAuditor::i_before( CustomEventTypeRef evt, const std::string& caller ) {
  info() << "About to Enter " << caller << " with auditor trigger " << evt << endmsg;
}

void NameAuditor::i_after( CustomEventTypeRef evt, const std::string& caller, const StatusCode& ) {
  info() << "Just Exited " << caller << " with auditor trigger " << evt << endmsg;
}
