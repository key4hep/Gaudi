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
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/VectorMap.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/ISequencerTimerTool.h"
// ============================================================================
#ifdef __ICC
// disable icc warning #654: overloaded virtual function "B::Y" is only partially overridden in class "C"
//   TODO: there is only a partial overload of IAuditor::before and IAuditor::after
#  pragma warning( disable : 654 )
#endif
/** @class TimingAuditor
 *
 *  Simple auditor which uses SequencerTimerTool for *ALL*
 *  algorithms, including the algorithm from main Gaudi "TopAlg" list
 *  and algorithms managed by Data-On-Demand Service
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-01-31
 */
class TimingAuditor : public extends<Auditor, IIncidentListener> {
public:
  void before( StandardEventType evt, INamedInterface* alg ) override;
  void after( StandardEventType evt, INamedInterface* alg, const StatusCode& sc ) override;

  using Auditor::before; // avoid hiding base-class methods
  void before( CustomEventTypeRef evt, const std::string& name ) override;
  using Auditor::after; // avoid hiding base-class methods
  void after( CustomEventTypeRef evt, const std::string& name, const StatusCode& sc ) override;

private:
  void i_beforeInitialize( INamedInterface* alg );
  void i_afterInitialize( INamedInterface* alg );
  void i_beforeFinalize( INamedInterface* alg );
  void i_beforeExecute( INamedInterface* alg );
  void i_afterExecute( INamedInterface* alg );

public:
  /// Inform that a new incident has occurred
  void handle( const Incident& ) override;

public:
  StatusCode initialize() override;
  StatusCode finalize() override;

public:
  using extends::extends;

  // delete default/copy constructor and assignment
  TimingAuditor()                       = delete;
  TimingAuditor( const TimingAuditor& ) = delete;
  TimingAuditor& operator=( const TimingAuditor& ) = delete;

private:
  // tool service
  SmartIF<IToolSvc> m_toolSvc = nullptr; ///< tool service
  // incident service
  SmartIF<IIncidentSvc> m_incSvc = nullptr; ///< incident service
  // the timer tool
  ISequencerTimerTool* m_timer = nullptr; ///< the timer tool
  // ApplicationManager
  SmartIF<INamedInterface> m_appMgr = nullptr; ///< ApplicationManager
  //
  GaudiUtils::VectorMap<const INamedInterface*, int> m_map;
  // indentation level
  int m_indent = 0; ///< indentation level
  // "in event"
  bool m_inEvent = false; ///< "In event" flag
  //
  GaudiUtils::HashMap<std::string, int> m_mapUser; ///< map used to record user timing events

  // Whether the timing has been saved already
  bool m_histoSaved = false;

  Gaudi::Property<bool> m_goodForDOD{this, "OptimizedForDOD", false, "enable optimization for Data-On-Demand Service"};
};
// ============================================================================
/// factory:
// ============================================================================
DECLARE_COMPONENT( TimingAuditor )
// ============================================================================
StatusCode TimingAuditor::initialize() {
  StatusCode sc = Auditor::initialize();
  if ( sc.isFailure() ) { return sc; } // RETURN

  // get tool service
  if ( !m_toolSvc ) {
    m_toolSvc = Auditor::service( "ToolSvc" );
    if ( !m_toolSvc ) {
      error() << "Could not retrieve 'ToolSvc' " << endmsg;
      return StatusCode::FAILURE; // RETURN
    }
    if ( !m_timer ) {
      sc = m_toolSvc->retrieveTool( "SequencerTimerTool/TIMER", m_timer, this, true );
      if ( sc.isFailure() ) {
        error() << "Could not retrieve ISequencerTimerTool" << endmsg;
        return sc;
      }
    }
  }
  // get incident service
  if ( !m_incSvc ) {
    m_incSvc = Auditor::service( "IncidentSvc" );
    if ( !m_incSvc ) {
      error() << "Could not retrieve 'IncidentSvc'" << endmsg;
      return StatusCode::FAILURE;
    }
    m_incSvc->addListener( this, IncidentType::BeginEvent );
    m_incSvc->addListener( this, IncidentType::EndEvent );
  }
  // get the application manager
  if ( !m_appMgr ) {
    m_appMgr = Auditor::service( "ApplicationMgr" );
    if ( !m_appMgr ) {
      error() << "Could not retrieve 'ApplicationMgr'" << endmsg;
      return sc;
    }
    if ( m_map.end() == m_map.find( m_appMgr.get() ) ) {
      m_map.insert( m_appMgr.get(), m_timer->addTimer( "EVENT LOOP" ) );
    }
  }
  //
  return StatusCode::SUCCESS;
}
// ============================================================================
StatusCode TimingAuditor::finalize() {
  if ( m_incSvc ) {
    m_incSvc->removeListener( this, IncidentType::BeginEvent );
    m_incSvc->removeListener( this, IncidentType::EndEvent );
    m_incSvc.reset();
  }
  if ( m_toolSvc ) {
    // the 2 following line are commented out: it is
    // is a temporary hack which prevent a crash due to a problem in
    // the reference counting
    //     if ( 0 != m_timer )
    //     { m_toolSvc -> releaseTool ( m_timer ) . ignore() ; m_timer = 0 ; }
    m_toolSvc.reset();
  }
  m_appMgr.reset();
  // clear the map
  m_map.clear();
  // finalize the base class
  return Auditor::finalize();
}
// ============================================================================
void TimingAuditor::before( StandardEventType evt, INamedInterface* alg ) {
  switch ( evt ) {
  case IAuditor::Initialize:
    i_beforeInitialize( alg );
    break;
  case IAuditor::Execute:
    i_beforeExecute( alg );
    break;
  case IAuditor::Finalize:
    i_beforeFinalize( alg );
    break;
  default:
    break;
  }
}
// ============================================================================
void TimingAuditor::after( StandardEventType evt, INamedInterface* alg, const StatusCode& ) {
  switch ( evt ) {
  case IAuditor::Initialize:
    i_afterInitialize( alg );
    break;
  case IAuditor::Execute:
    i_afterExecute( alg );
    break;
  default:
    break;
  }
}
// ============================================================================
void TimingAuditor::i_beforeFinalize( INamedInterface* /*alg*/ ) {
  if ( !m_histoSaved ) {
    m_timer->saveHistograms();
    m_histoSaved = true;
  }
}

// ============================================================================
void TimingAuditor::i_beforeInitialize( INamedInterface* alg ) {
  if ( m_goodForDOD ) { return; }
  //
  if ( !alg ) { return; }
  auto found = m_map.find( alg );
  if ( m_map.end() != found ) { return; }
  ++m_indent;
  std::string nick = alg->name();
  if ( 0 < m_indent ) { nick = std::string( m_indent, ' ' ) + nick; }
  if ( m_inEvent ) {
    nick[0] = '*';
    debug() << "Insert non-structural component '" << alg->name() << "' of type '"
            << System::typeinfoName( typeid( *alg ) ) << "' at level " << m_indent << endmsg;
  }
  int timer = m_timer->addTimer( nick );
  m_map.insert( alg, timer );
  m_timer->start( timer );
}
// ============================================================================
void TimingAuditor::i_afterInitialize( INamedInterface* alg ) {
  if ( m_goodForDOD || !alg ) { return; }
  --m_indent;
}
// ============================================================================
void TimingAuditor::i_beforeExecute( INamedInterface* alg ) {
  if ( !alg ) { return; }
  ++m_indent;
  auto found = m_map.find( alg );
  if ( m_map.end() == found ) {
    debug() << "Insert non-structural component '" << alg->name() << "' of type '"
            << System::typeinfoName( typeid( *alg ) ) << "' at level " << m_indent << endmsg;
    std::string nick = alg->name();
    if ( 0 < m_indent ) { nick = std::string( m_indent, ' ' ) + nick; }
    if ( !m_goodForDOD ) { nick[0] = '*'; }
    int timer = m_timer->addTimer( nick );
    m_map.insert( alg, timer );
    m_timer->start( timer );
    return;
  }
  m_timer->start( found->second );
}
// ============================================================================
void TimingAuditor::i_afterExecute( INamedInterface* alg ) {
  if ( !alg ) { return; }
  auto found = m_map.find( alg );
  if ( m_map.end() == found ) { return; }
  m_timer->stop( found->second );
  --m_indent;
}
// ============================================================================
void TimingAuditor::before( CustomEventTypeRef evt, const std::string& name ) {
  // Ignore obvious mistakes
  if ( name.empty() && evt.empty() ) { return; }

  // look for the user timer in the map
  int         timer = 0;
  std::string nick  = name + ":" + evt;
  auto        found = m_mapUser.find( nick );

  if ( m_mapUser.end() == found ) {
    // add a new timer if not yet available
    timer           = m_timer->addTimer( nick );
    m_mapUser[nick] = timer;
  } else {
    timer = found->second;
  }
  m_timer->start( timer );
}
// ============================================================================
void TimingAuditor::after( CustomEventTypeRef evt, const std::string& name, const StatusCode& ) {
  // Ignore obvious mistakes
  if ( name.empty() && evt.empty() ) { return; }

  // look for the user timer in the map
  std::string nick  = name + ":" + evt;
  auto        found = m_mapUser.find( nick );

  // We cannot do much if the timer is not available
  if ( m_mapUser.end() == found ) {
    warning() << "Trying to stop the measure  of the timing for '" << nick
              << "' but it was never started. Check the code" << endmsg;
    return;
  }
  m_timer->stop( found->second );
}
// ============================================================================
void TimingAuditor::handle( const Incident& i ) {
  if ( IncidentType::BeginEvent == i.type() ) {
    m_timer->start( m_map[m_appMgr.get()] );
    ++m_indent;
    m_inEvent = true;
  } else if ( IncidentType::EndEvent == i.type() ) {
    m_timer->stop( m_map[m_appMgr.get()] );
    --m_indent;
    m_inEvent = false;
  }
}
// ============================================================================
// The END
// ============================================================================
