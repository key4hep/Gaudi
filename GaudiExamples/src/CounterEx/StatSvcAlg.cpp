// $Id: StatSvcAlg.cpp,v 1.1 2007/08/06 08:44:04 marcocle Exp $
// ============================================================================
// Include files 
// ============================================================================
// STD & STL
// ============================================================================
#include <cmath>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IStatSvc.h"
#include "GaudiKernel/Stat.h"
#include "GaudiKernel/AlgFactory.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================
/** @file
 *  Simple example to illustrate the statistical abilities of 
 *  "Stat"-part of Chrono&Stat Service
 *  @see Stat
 *  @see StatEntity
 *  @see IStatSvc
 *  @see IChronoStatSvc
 *  @see GaudiExamples::StatSvcAlg
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2008-07-08
 */
// ============================================================================
namespace GaudiExamples
{
  /** @class StatSvcAlg
   *  Simple algorithm to illustrate the statistical abilities of 
   *  "Stat"-part of Chrono&Stat Service
   *  @see Stat
   *  @see StatEntity
   *  @see IStatSvc
   *  @see IChronoStatSvc
   *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
   *  @date 2008-07-08
   */
  class StatSvcAlg : public GaudiAlgorithm
  {
    // friend factory for instantiation:
    friend class AlgFactory<StatSvcAlg> ;
  public:
    /// initialize the algorithm
    StatusCode initialize () 
    {
      StatusCode sc = GaudiAlgorithm::initialize () ;
      if ( sc.isFailure() ) { return sc ; }                      // RETURN 
      m_stat = svc<IStatSvc> ( "ChronoStatSvc" , true ) ;
      return StatusCode::SUCCESS ;
    }
    /// the main execution method 
    StatusCode execute    () ;
  protected:
    /// standard constructor from name and Service Locator
    StatSvcAlg ( const std::string& name , ISvcLocator* svc ) 
      : GaudiAlgorithm ( name , svc ) 
      , m_stat         ( 0 )
    {} 
    /// destructor (virtual and protected)
    virtual ~StatSvcAlg() {} ;
  private:
    // pointer to Stat Service
    IStatSvc* m_stat ; ///< pointer to Stat Service
  } ;
} // end of namespace GaudiExamples
// ============================================================================
// the main execution method 
// ============================================================================
StatusCode GaudiExamples::StatSvcAlg::execute    () 
{
  Stat st1 ( m_stat , "counter1" ) ;
  Stat st2 ( m_stat , "counter2" ) ;
  Stat st3 ( m_stat , "counter3" , 0.3 ) ;
  
  Stat eff ( m_stat , "eff" , 0 < sin( 10 * st1->flag() ) ) ;
  //
  st1 += 0.1     ;
  st1 -= 0.1000452  ;
  st2 += st1 ;
  ++st3 ;
  st2-- ;
  
  eff += 0 < cos ( 20 * st2->flag() ) ;
  
  //
  return StatusCode::SUCCESS ;
}
// ============================================================================
// Factory:
// ============================================================================
DECLARE_NAMESPACE_ALGORITHM_FACTORY(GaudiExamples,StatSvcAlg)
// ============================================================================
// The END
// ============================================================================


