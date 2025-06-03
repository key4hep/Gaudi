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
#include <GaudiKernel/IChronoStatSvc.h>

/** @class ChronoAuditor
    Monitors the cpu time usage of each algorithm

    @author David Quarrie
    @author Marco Clemencic
*/
class ChronoAuditor : public Gaudi::Auditor {
public:
  using Auditor::Auditor;

  StatusCode initialize() override;

private:
  /// Default (catch-all) "before" Auditor hook
  void before( std::string const& evt, std::string const& caller, EventContext const& ) override;

  /// Default (catch-all) "after" Auditor hook
  void after( std::string const& evt, std::string const& caller, EventContext const&, const StatusCode& sc ) override;

  /// Compute the id string to be used for the chrono entity.
  std::string i_id( std::string const& evt, std::string_view caller ) { return std::string{ caller } + ":" + evt; }

  SmartIF<IChronoStatSvc>& chronoSvc() { return m_chronoSvc; }
  SmartIF<IChronoStatSvc>  m_chronoSvc;
};

DECLARE_COMPONENT( ChronoAuditor )

StatusCode ChronoAuditor::initialize() {
  return Auditor::initialize().andThen( [&]() -> StatusCode {
    m_chronoSvc = serviceLocator()->service( "ChronoStatSvc" );
    if ( !m_chronoSvc.get() ) {
      error() << "Cannot get ChronoStatSvc" << endmsg;
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  } );
}

void ChronoAuditor::before( std::string const& evt, std::string const& caller, EventContext const& ) {
  chronoSvc()->chronoStart( i_id( evt, caller ) );
}

void ChronoAuditor::after( std::string const& evt, std::string const& caller, EventContext const&, const StatusCode& ) {
  chronoSvc()->chronoStop( i_id( evt, caller ) );
}
