// $Id: GaudiTool.cpp,v 1.10 2007/09/25 16:12:41 marcocle Exp $
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/Bootstrap.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiTool.h"
#include "GaudiAlg/GaudiAlgorithm.h"
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
namespace GaudiToolServices
{
  /// the default name for Event Data Service
  const std::string s_EventDataSvc    = "EventDataSvc"    ;
  /// the default name for Detector Data Service
  const std::string s_DetectorDataSvc = "DetectorDataSvc" ;
  /// the default name for Chrono & Stat Service
  const std::string s_ChronoStatSvc   = "ChronoStatSvc"   ;
  /// the default name for Incident Service
  const std::string s_IncidentSvc     = "IncidentSvc"     ;
  /// the default name for Histogram Service
  const std::string s_HistoSvc        = "HistogramDataSvc" ;
}
// ============================================================================
namespace GaudiToolLocal
{
  // ==========================================================================
  /** @class Counter
   *  simple local counter
   */
  class Counter
  {
  public:
    // ========================================================================
    // constructor
    Counter ( const std::string& msg = " Misbalance ")
      : m_map     ()
      , m_message ( msg )
    {};
    // destructor
    ~Counter() { report() ; m_map.clear() ;}
    // ========================================================================
  public:
    // ========================================================================
    /// make the increment
    long increment ( const std::string& object ) { return ++m_map[object] ; }
    /// make the decrement
    long decrement ( const std::string& object ) { return --m_map[object] ; }
    /// current count
    long counts    ( const std::string& object ) { return   m_map[object] ; }
    /// make a report
    void report() const
    {
      /// keep the silence?
      if ( !GaudiTool::summaryEnabled() ) { return ; } // RETURN
      //
      for ( Map::const_iterator entry = m_map.begin() ;
            m_map.end() != entry ; ++entry )
      {
        if( 0 == entry->second ) { continue ; }
        std::cout << "GaudiTool       WARNING  "          << m_message
                  << "'" << entry->first << "' Counts = " << entry->second
                  << std::endl ;
      }
    }
    // ========================================================================
  private:
    // ========================================================================
    typedef std::map<std::string,long> Map;
    Map         m_map     ;
    std::string m_message ;
    // ========================================================================
  };
  // ==========================================================================
  /** @var s_InstanceCounter
   *  The instance counter for all 'GaudiTool' based classes
   *  @author Vanya BELYAEV Ivan.Belyaev@Ivan.Belyaev@itep.ru
   *  @date   2004-01-19
   */
  static Counter s_InstanceCounter ( " Create/Destroy      (mis)balance " ) ;
  // ==========================================================================
  /** @var s_FinalizeCounter
   *  The initialize/finalize mismatch counter for all 'GaudiTool' based classes
   *  @author Vanya BELYAEV Ivan.Belyaev@Ivan.Belyaev@itep.ru
   *  @date   2004-01-19
   */
  static Counter s_FinalizeCounter ( " Initialize/Finalize (mis)balance " ) ;
  // ==========================================================================
}
// ============================================================================
/// summary is enabled
// ============================================================================
bool GaudiTool::s_enableSummary = true ;                  // summary is enabled
// ============================================================================
// enable/disable summary
// ============================================================================
bool GaudiTool::enableSummary  ( bool value )         // enable/disable summary
{
  s_enableSummary = value ;
  return summaryEnabled () ;
}
// ============================================================================
// is summary enabled?
// ============================================================================
bool GaudiTool::summaryEnabled ()                     // is summary enabled?
{ return s_enableSummary ; }
// ============================================================================
// Standard constructor
// ============================================================================
GaudiTool::GaudiTool ( const std::string& this_type   ,
                       const std::string& this_name   ,
                       const IInterface*  parent )
  : GaudiCommon<AlgTool> ( this_type , this_name , parent )
  //  services
  , m_ntupleSvc   ( 0 )
  , m_evtColSvc   ( 0 )
  , m_evtSvc      ( 0 )
  , m_detSvc      ( 0 )
  , m_chronoSvc   ( 0 )
  , m_incSvc      ( 0 )
  , m_histoSvc    ( 0 )
  , m_contextSvc  ( 0 ) // pointer to Algorithm Context Service
  , m_contextSvcName ( "AlgContextSvc" ) // Algorithm Context Service name
  //
  , m_local       ( this_type + "/" + this_name )
{
  declareProperty
    ( "ContextService" ,
      m_contextSvcName ,
      "The name of Algorithm Context Service" ) ;
  // make instance counts
  GaudiToolLocal::s_InstanceCounter.increment ( m_local ) ;
}
// ============================================================================
// destructor
// ============================================================================
GaudiTool::~GaudiTool()
{
  GaudiToolLocal::s_InstanceCounter.decrement ( m_local ) ;
}
// ============================================================================
// standard initialization method
// ============================================================================
StatusCode    GaudiTool::initialize ()
{
  // initialize the base class
  const StatusCode sc = GaudiCommon<AlgTool>::initialize() ;
  if ( sc.isFailure() ) { return sc; }

  // increment the counter
  GaudiToolLocal::s_FinalizeCounter.increment( m_local ) ;

  // return
  return sc;
}
// ============================================================================
// standard finalization method
// ============================================================================
StatusCode    GaudiTool::finalize   ()
{
  if ( msgLevel(MSG::DEBUG) )
    debug() << " ==> Finalize the base class GaudiTool " << endmsg;

  // clear "explicit services"
    m_evtSvc    = 0 ;
    m_detSvc    = 0 ;
    m_chronoSvc = 0 ;
    m_incSvc    = 0 ;
    m_histoSvc  = 0 ;

  // finalize the base class
  const StatusCode sc = GaudiCommon<AlgTool>::finalize() ;
  if ( sc.isFailure() ) { return sc; }

  // Decrement the counter
  GaudiToolLocal::s_FinalizeCounter.decrement( m_local ) ;

  // return
  return sc;
}
// ============================================================================
// accessor to detector service
// ============================================================================
IDataProviderSvc* GaudiTool::detSvc    () const
{
  if ( 0 == m_detSvc )
  {
    m_detSvc =
      svc<IDataProviderSvc>( GaudiToolServices::s_DetectorDataSvc , true ) ;
  }
  return m_detSvc ;
}
// ============================================================================
// The standard N-Tuple
// ============================================================================
INTupleSvc* GaudiTool::ntupleSvc () const
{
  if ( 0 == m_ntupleSvc )
  {
    m_ntupleSvc = svc<INTupleSvc>( "NTupleSvc" , true ) ;
  }
  return m_ntupleSvc ;
}
// ============================================================================
// The standard event collection service
// ============================================================================
INTupleSvc* GaudiTool::evtColSvc () const
{
  if ( 0 == m_evtColSvc )
  {
    m_evtColSvc = svc< INTupleSvc > ( "EvtTupleSvc" , true ) ;
  }
  return m_evtColSvc ;
}
// ============================================================================
// accessor to event service  service
// ============================================================================
IDataProviderSvc* GaudiTool::evtSvc    () const
{
  if ( 0 == m_evtSvc )
  {
    m_evtSvc =
      svc<IDataProviderSvc>( GaudiToolServices::s_EventDataSvc , true ) ;
  }
  return m_evtSvc ;
}
// ============================================================================
// accessor to Incident Service
// ============================================================================
IIncidentSvc*      GaudiTool::incSvc   () const
{
  if ( 0 == m_incSvc )
  {
    m_incSvc =
      svc<IIncidentSvc> ( GaudiToolServices::s_IncidentSvc , true ) ;
  }
  return m_incSvc ;
}
// ============================================================================
// accessor to Chrono & Stat Service
// ============================================================================
IChronoStatSvc*      GaudiTool::chronoSvc () const
{
  if ( 0 == m_chronoSvc )
  {
    m_chronoSvc =
      svc<IChronoStatSvc> ( GaudiToolServices::s_ChronoStatSvc , true ) ;
  }
  return m_chronoSvc ;
}
// ============================================================================
// accessor to histogram Service
// ============================================================================
IHistogramSvc*       GaudiTool::histoSvc  () const
{
  if ( 0 == m_histoSvc )
  {
    m_histoSvc = svc<IHistogramSvc> ( GaudiToolServices::s_HistoSvc, true ) ;
  }
  return m_histoSvc;
}
// ============================================================================
// accessor to Algorithm Context Service
// ============================================================================
IAlgContextSvc* GaudiTool::contextSvc  () const
{
  if ( 0 == m_contextSvc )
  {
    m_contextSvc = svc<IAlgContextSvc> ( m_contextSvcName , true ) ;
  }
  return m_contextSvc;
}
// ============================================================================
// The END
// ============================================================================
