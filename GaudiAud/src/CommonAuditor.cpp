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
#include "CommonAuditor.h"
#include "GaudiKernel/GaudiException.h"

void CommonAuditor::before( StandardEventType evt, INamedInterface* caller ) {
  if ( caller ) before( toStr( evt ), caller->name() );
}
void CommonAuditor::before( StandardEventType evt, const std::string& caller ) { before( toStr( evt ), caller ); }
void CommonAuditor::before( CustomEventTypeRef evt, INamedInterface* caller ) {
  if ( caller ) before( evt, caller->name() );
}
void CommonAuditor::before( CustomEventTypeRef evt, const std::string& caller ) {
  if ( i_auditEventType( evt ) ) i_before( evt, caller );
}

void CommonAuditor::after( StandardEventType evt, INamedInterface* caller, const StatusCode& sc ) {
  if ( caller ) after( toStr( evt ), caller->name(), sc );
}
void CommonAuditor::after( StandardEventType evt, const std::string& caller, const StatusCode& sc ) {
  after( toStr( evt ), caller, sc );
}
void CommonAuditor::after( CustomEventTypeRef evt, INamedInterface* caller, const StatusCode& sc ) {
  if ( caller ) after( evt, caller->name(), sc );
}
void CommonAuditor::after( CustomEventTypeRef evt, const std::string& caller, const StatusCode& sc ) {
  if ( i_auditEventType( evt ) ) i_after( evt, caller, sc );
}
