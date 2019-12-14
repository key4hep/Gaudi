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

#ifdef __ICC
// disable icc warning #654: overloaded virtual function "B::Y" is only partially overridden in class "C"
//   TODO: there is only a partial overload of IAuditor::before and IAuditor::after
#  pragma warning( disable : 654 )
#endif

#include "ChronoAuditor.h"

DECLARE_COMPONENT( ChronoAuditor )

StatusCode ChronoAuditor::initialize() {
  StatusCode sc = CommonAuditor::initialize();
  if ( UNLIKELY( sc.isFailure() ) ) return sc;

  m_chronoSvc = serviceLocator()->service( "ChronoStatSvc" );
  if ( UNLIKELY( !m_chronoSvc.get() ) ) {
    error() << "Cannot get ChronoStatSvc" << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

void ChronoAuditor::i_before( CustomEventTypeRef evt, const std::string& caller ) {
  chronoSvc()->chronoStart( i_id( evt, caller ) );
}

void ChronoAuditor::i_after( CustomEventTypeRef evt, const std::string& caller, const StatusCode& ) {
  chronoSvc()->chronoStop( i_id( evt, caller ) );
}
