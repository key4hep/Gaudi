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
// ChronoAuditor:
// An auditor that monitors time

#include "ChronoAuditor.h"

DECLARE_COMPONENT( ChronoAuditor )

StatusCode ChronoAuditor::initialize() {
  return CommonAuditor::initialize().andThen( [&]() -> StatusCode {
    m_chronoSvc = serviceLocator()->service( "ChronoStatSvc" );
    if ( !m_chronoSvc.get() ) {
      error() << "Cannot get ChronoStatSvc" << endmsg;
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  } );
}

void ChronoAuditor::i_before( CustomEventTypeRef evt, std::string_view caller ) {
  chronoSvc()->chronoStart( i_id( evt, caller ) );
}

void ChronoAuditor::i_after( CustomEventTypeRef evt, std::string_view caller, const StatusCode& ) {
  chronoSvc()->chronoStop( i_id( evt, caller ) );
}
