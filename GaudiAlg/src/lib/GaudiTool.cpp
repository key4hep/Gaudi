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
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/INTupleSvc.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/GaudiTool.h"
// ============================================================================
/** @file GaudiTool.cpp
 *
 *  Implementation file for class GaudiTool
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date 01/11/2001
 */
// ============================================================================
// templated methods
// ============================================================================
#include "GaudiCommon.icpp"
// ============================================================================
template class GaudiCommon<AlgTool>;
// ============================================================================
/** @namespace GaudiToolServices
 *  Collection of default services names to be used
 *  for class GaudiTool
 *  @author Vanya BELYAEV Ivan.Belyaev@Ivan.Belyaev@itep.ru
 *  @date   2004-01-19
 */
// ============================================================================
namespace GaudiToolServices {
  /// the default name for Event Data Service
  const std::string s_EventDataSvc = "EventDataSvc";
  /// the default name for Detector Data Service
  const std::string s_DetectorDataSvc = "DetectorDataSvc";
  /// the default name for Chrono & Stat Service
  const std::string s_ChronoStatSvc = "ChronoStatSvc";
  /// the default name for Incident Service
  const std::string s_IncidentSvc = "IncidentSvc";
  /// the default name for Histogram Service
  const std::string s_HistoSvc = "HistogramDataSvc";
} // namespace GaudiToolServices
// ============================================================================
namespace GaudiToolLocal {
  // ==========================================================================
  /** @class Counter
   *  simple local counter
   */
  class Counter final {
  public:
    // ========================================================================
    // constructor
    Counter( std::string msg = " Misbalance " ) : m_message( std::move( msg ) ){};
    // destructor
    ~Counter() { report(); }
    // ========================================================================
  public:
    // ========================================================================
    /// make the increment
    long increment( std::string_view object ) { return ++get( object ); }
    /// make the decrement
    long decrement( std::string_view object ) { return --get( object ); }
    /// current count
    long counts( std::string_view object ) { return get( object ); }
    /// make a report
    void report() const {
      /// keep the silence?
      if ( !GaudiTool::summaryEnabled() ) { return; } // RETURN
      //
      for ( const auto& entry : m_map ) {
        if ( entry.second ) {
          std::cout << "GaudiTool       WARNING  " << m_message << "'" << entry.first << "' Counts = " << entry.second
                    << std::endl;
        }
      }
    }
    // ========================================================================
  private:
    // ========================================================================
    using Map = std::map<std::string, long, std::less<>>;
    long& get( std::string_view which ) {
      auto i = m_map.find( which );
      if ( i == m_map.end() ) i = m_map.emplace( which, long{} ).first;
      return i->second;
    }
    Map         m_map;
    std::string m_message;
    // ========================================================================
  };
  // ==========================================================================
  /** @var s_InstanceCounter
   *  The instance counter for all 'GaudiTool' based classes
   *  @author Vanya BELYAEV Ivan.Belyaev@Ivan.Belyaev@itep.ru
   *  @date   2004-01-19
   */
  static Counter s_InstanceCounter( " Create/Destroy      (mis)balance " );
  // ==========================================================================
  /** @var s_FinalizeCounter
   *  The initialize/finalize mismatch counter for all 'GaudiTool' based classes
   *  @author Vanya BELYAEV Ivan.Belyaev@Ivan.Belyaev@itep.ru
   *  @date   2004-01-19
   */
  static Counter s_FinalizeCounter( " Initialize/Finalize (mis)balance " );
  // ==========================================================================
} // namespace GaudiToolLocal
// ============================================================================
/// summary is enabled
// ============================================================================
bool GaudiTool::s_enableSummary = true; // summary is enabled
// ============================================================================
// enable/disable summary
// ============================================================================
bool GaudiTool::enableSummary( bool value ) // enable/disable summary
{
  s_enableSummary = value;
  return summaryEnabled();
}
// ============================================================================
// is summary enabled?
// ============================================================================
bool GaudiTool::summaryEnabled() // is summary enabled?
{
  return s_enableSummary;
}
// ============================================================================
// Standard constructor
// ============================================================================
GaudiTool::GaudiTool( std::string this_type, std::string this_name, const IInterface* parent )
    : GaudiCommon<AlgTool>( std::move( this_type ), std::move( this_name ), parent ), m_local( type() + "/" + name() ) {
  // make instance counts
  GaudiToolLocal::s_InstanceCounter.increment( m_local );
}
// ============================================================================
// destructor
// ============================================================================
GaudiTool::~GaudiTool() { GaudiToolLocal::s_InstanceCounter.decrement( m_local ); }
// ============================================================================
// standard initialization method
// ============================================================================
StatusCode GaudiTool::initialize() {
  // initialize the base class
  const StatusCode sc = GaudiCommon<AlgTool>::initialize();
  if ( sc.isFailure() ) { return sc; }

  // increment the counter
  GaudiToolLocal::s_FinalizeCounter.increment( m_local );

  // are we a public tool ?
  m_isPublic = isPublic();

  // return
  return sc;
}
// ============================================================================
// standard finalization method
// ============================================================================
StatusCode GaudiTool::finalize() {
  if ( msgLevel( MSG::DEBUG ) ) debug() << " ==> Finalize the base class GaudiTool " << endmsg;

  // clear "explicit services"
  m_detSvc.reset();
  m_chronoSvc.reset();
  m_incSvc.reset();
  m_histoSvc.reset();

  // finalize the base class
  const StatusCode sc = GaudiCommon<AlgTool>::finalize();
  if ( sc.isFailure() ) { return sc; }

  // Decrement the counter
  GaudiToolLocal::s_FinalizeCounter.decrement( m_local );

  // return
  return sc;
}
// ============================================================================
// Determines if this tool is public or not (i.e. owned by the ToolSvc).
// ============================================================================
bool GaudiTool::isPublic() const {
  const IAlgTool* tool = this;
  // Recurse down the ownership tree, to see with we ever end up at the ToolSvc
  bool         ownedByToolSvc = false;
  unsigned int sanityCheck( 0 );
  while ( tool && ++sanityCheck < 99999 ) {
    ownedByToolSvc = ( nullptr != dynamic_cast<const IToolSvc*>( tool->parent() ) );
    if ( ownedByToolSvc ) { break; }
    // if parent is also a tool, try again
    tool = dynamic_cast<const IAlgTool*>( tool->parent() );
  }
  return ownedByToolSvc;
}
// ============================================================================
// accessor to detector service
// ============================================================================
IDataProviderSvc* GaudiTool::detSvc() const {
  if ( !m_detSvc ) m_detSvc = service( GaudiToolServices::s_DetectorDataSvc, true );
  return m_detSvc;
}
// ============================================================================
// The standard N-Tuple
// ============================================================================
INTupleSvc* GaudiTool::ntupleSvc() const {
  if ( !m_ntupleSvc ) m_ntupleSvc = service( "NTupleSvc", true );
  return m_ntupleSvc;
}
// ============================================================================
// The standard event collection service
// ============================================================================
INTupleSvc* GaudiTool::evtColSvc() const {
  if ( !m_evtColSvc ) m_evtColSvc = service( "EvtTupleSvc", true );
  return m_evtColSvc;
}
// ============================================================================
// accessor to Incident Service
// ============================================================================
IIncidentSvc* GaudiTool::incSvc() const {
  if ( !m_incSvc ) m_incSvc = service( GaudiToolServices::s_IncidentSvc, true );
  return m_incSvc;
}
// ============================================================================
// accessor to Chrono & Stat Service
// ============================================================================
IChronoStatSvc* GaudiTool::chronoSvc() const {
  if ( !m_chronoSvc ) m_chronoSvc = service( GaudiToolServices::s_ChronoStatSvc, true );
  return m_chronoSvc;
}
// ============================================================================
// accessor to histogram Service
// ============================================================================
IHistogramSvc* GaudiTool::histoSvc() const {
  if ( !m_histoSvc ) m_histoSvc = service( GaudiToolServices::s_HistoSvc, true );
  return m_histoSvc;
}
// ============================================================================
// accessor to Algorithm Context Service
// ============================================================================
IAlgContextSvc* GaudiTool::contextSvc() const {
  if ( !m_contextSvc ) m_contextSvc = service( m_contextSvcName, true );
  return m_contextSvc;
}
// ============================================================================
// The END
// ============================================================================
