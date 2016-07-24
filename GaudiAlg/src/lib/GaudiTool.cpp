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
  class Counter final
  {
  public:
    // ========================================================================
    // constructor
    Counter ( std::string msg = " Misbalance ")
      : m_message ( std::move(msg) )
    {};
    // destructor
    ~Counter() { report(); }
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
      for ( const auto& entry : m_map )
      {
        if( entry.second ) {
            std::cout << "GaudiTool       WARNING  "         << m_message
                      << "'" << entry.first << "' Counts = " << entry.second
                      << std::endl ;
        }
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

  // are we a public tool ?
  m_isPublic = isPublic();

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
  m_detSvc.reset();
  m_chronoSvc.reset();
  m_incSvc.reset();
  m_histoSvc.reset();

  // finalize the base class
  const StatusCode sc = GaudiCommon<AlgTool>::finalize() ;
  if ( sc.isFailure() ) { return sc; }

  // Decrement the counter
  GaudiToolLocal::s_FinalizeCounter.decrement( m_local ) ;

  // return
  return sc;
}
// ============================================================================
// Determines if this tool is public or not (i.e. owned by the ToolSvc).
// ============================================================================
bool GaudiTool::isPublic() const
{
  const IAlgTool * tool = this;
  // Recurse down the ownership tree, to see with we ever end up at the ToolSvc
  bool ownedByToolSvc = false;
  unsigned int sanityCheck(0);
  while ( tool && ++sanityCheck < 99999 )
  {
    ownedByToolSvc = ( nullptr != dynamic_cast<const IToolSvc*>(tool->parent()) );
    if ( ownedByToolSvc ) { break; }
    // if parent is also a tool, try again
    tool = dynamic_cast<const IAlgTool*>(tool->parent());
  }
  return ownedByToolSvc;
}
// ============================================================================
// accessor to detector service
// ============================================================================
IDataProviderSvc* GaudiTool::detSvc () const
{
  if ( UNLIKELY(!m_detSvc) ) m_detSvc = service( GaudiToolServices::s_DetectorDataSvc , true ) ;
  return m_detSvc ;
}
// ============================================================================
// The standard N-Tuple
// ============================================================================
INTupleSvc* GaudiTool::ntupleSvc () const
{
  if ( UNLIKELY(!m_ntupleSvc) ) m_ntupleSvc = service( "NTupleSvc" , true ) ;
  return m_ntupleSvc ;
}
// ============================================================================
// The standard event collection service
// ============================================================================
INTupleSvc* GaudiTool::evtColSvc () const
{
  if (UNLIKELY(!m_evtColSvc)) m_evtColSvc = service( "EvtTupleSvc" , true ) ;
  return m_evtColSvc ;
}
// ============================================================================
// accessor to Incident Service
// ============================================================================
IIncidentSvc*      GaudiTool::incSvc () const
{
  if (UNLIKELY(!m_incSvc)) m_incSvc = service( GaudiToolServices::s_IncidentSvc , true ) ;
  return m_incSvc ;
}
// ============================================================================
// accessor to Chrono & Stat Service
// ============================================================================
IChronoStatSvc*      GaudiTool::chronoSvc () const
{
  if (UNLIKELY( !m_chronoSvc)) m_chronoSvc = service( GaudiToolServices::s_ChronoStatSvc , true ) ;
  return m_chronoSvc ;
}
// ============================================================================
// accessor to histogram Service
// ============================================================================
IHistogramSvc*       GaudiTool::histoSvc () const
{
  if (UNLIKELY(!m_histoSvc)) m_histoSvc = service ( GaudiToolServices::s_HistoSvc, true ) ;
  return m_histoSvc;
}
// ============================================================================
// accessor to Algorithm Context Service
// ============================================================================
IAlgContextSvc* GaudiTool::contextSvc () const
{
  if (UNLIKELY(!m_contextSvc)) m_contextSvc = service( m_contextSvcName , true ) ;
  return m_contextSvc;
}
// ============================================================================
// declare data handle property
// ============================================================================

Property* GaudiTool::declareProperty( const std::string& name,
                                      DataObjectHandleBase& hndl,
                                      const std::string& doc ) const
{
  if ( hndl.mode() & Gaudi::DataHandle::Reader ) {
    (const_cast<GaudiTool*>(this))->AlgTool::declareInput(&hndl);
  }

  if ( hndl.mode() & Gaudi::DataHandle::Writer ) {
    (const_cast<GaudiTool*>(this))->AlgTool::declareOutput(&hndl);
  }

  if (!hndl.owner()) hndl.setOwner((const_cast<GaudiTool*>(this)));

  return m_propertyMgr->declareProperty(name, hndl, doc);
}

// ============================================================================
// The END
// ============================================================================
