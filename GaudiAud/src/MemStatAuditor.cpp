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

/** @class MemStatAuditor MemStatAuditor.h GaudiAud/MemStatAudit.h

    Just a minor modification of MemoryAuditor to allow
    the output memory statistics table to be printed

    @author  Vanya Belyaev
    @author  Marco Clemencic
    @date    04/02/2001
*/

#include "ProcStats.h"
#include <Gaudi/Auditor.h>
#include <GaudiKernel/IChronoStatSvc.h>
#include <GaudiKernel/MsgStream.h>
#include <GaudiKernel/Stat.h>

class MemStatAuditor : public Gaudi::Auditor {
public:
  using Auditor::Auditor;

  StatusCode initialize() override;

  void before( std::string const& /*evt*/, std::string const& /*caller*/, EventContext const& ) override {
    // It's not interesting to monitor the memory usage before the methods.
  }

  void after( std::string const& evt, std::string const& caller, EventContext const&, const StatusCode& ) override {
    i_printinfo( "Memory usage has changed after", evt, caller );
  }

private:
  void                    i_printinfo( std::string_view msg, std::string const& evt, std::string_view caller );
  SmartIF<IChronoStatSvc> m_stat;

  /// vsize of the previous call to printinfo
  double m_vSize = -1;
};

DECLARE_COMPONENT( MemStatAuditor )

StatusCode MemStatAuditor::initialize() {
  return Auditor::initialize().andThen( [&]() -> StatusCode {
    m_stat = serviceLocator()->service( "ChronoStatSvc" );
    if ( !m_stat ) {
      error() << "Cannot get ChronoStatSvc" << endmsg;
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  } );
}

void MemStatAuditor::i_printinfo( std::string_view msg, std::string const& evt, std::string_view caller ) {
  // cannot be exactly 0
  double deltaVSize = 0.00001;

  if ( procInfo pInfo; ProcStats::instance()->fetch( pInfo ) ) {

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
  // Stat stv(m_stat, caller + ":VMemUsage", pInfo.vsize);
  // Stat str(m_stat, caller + ":RMemUsage", pInfo.rss);
  Stat sts( m_stat, std::string{ caller }.append( ":VMem" ), deltaVSize );
}
