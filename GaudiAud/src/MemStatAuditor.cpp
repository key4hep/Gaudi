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
#ifdef __ICC
// disable icc warning #654: overloaded virtual function "B::Y" is only partially overridden in class "C"
//   TODO: there is only a partial overload of IAuditor::before and IAuditor::after
#  pragma warning( disable : 654 )
#endif

#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/MsgStream.h"

#include "GaudiKernel/Stat.h"

/// local
#include "MemStatAuditor.h"
#include "ProcStats.h"

DECLARE_COMPONENT( MemStatAuditor )

StatusCode MemStatAuditor::initialize() {
  StatusCode sc = CommonAuditor::initialize();
  if ( UNLIKELY( sc.isFailure() ) ) return sc;

  m_stat = serviceLocator()->service( "ChronoStatSvc" );
  if ( UNLIKELY( !m_stat.get() ) ) {
    error() << "Cannot get ChronoStatSvc" << endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

void MemStatAuditor::i_before( CustomEventTypeRef /*evt*/, const std::string& /*caller*/ ) {
  // It's not interesting to monitor the memory usage before the methods.
}

void MemStatAuditor::i_printinfo( const std::string& msg, CustomEventTypeRef evt, const std::string& caller ) {
  // cannot be exactly 0
  double deltaVSize = 0.00001;

  procInfo pInfo;
  if ( getProcInfo( pInfo ) ) {

    if ( pInfo.vsize > 0 ) {
      if ( m_vSize > 0 ) { deltaVSize = pInfo.vsize - m_vSize; }
      // store the current VSize to be able to monitor the increment
      m_vSize = pInfo.vsize;
    }

    info() << msg << " " << caller << " " << evt << " \tvirtual size = " << pInfo.vsize << " MB"
           << " \tresident set size = " << pInfo.rss << " MB"
           << " deltaVsize = " << deltaVSize << "  MB" << endmsg;
  }
  // fill the stat for every call, not just when there is a change
  // only monitor the increment in VSize
  // Stat stv(statSvc(), caller + ":VMemUsage", pInfo.vsize);
  // Stat str(statSvc(), caller + ":RMemUsage", pInfo.rss);
  Stat sts( statSvc(), caller + ":VMem", deltaVSize );
}
