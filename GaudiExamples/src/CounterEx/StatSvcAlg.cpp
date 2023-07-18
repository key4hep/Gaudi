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
// STD & STL
// ============================================================================
#include <cmath>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IStatSvc.h"
#include "GaudiKernel/Stat.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiKernel/Algorithm.h"
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
namespace GaudiExamples {
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
  class StatSvcAlg : public Algorithm {
  public:
    /// standard constructor from name and Service Locator
    using Algorithm::Algorithm;

    /// initialize the algorithm
    StatusCode initialize() override {
      StatusCode sc = Algorithm::initialize();
      if ( sc.isFailure() ) return sc; // RETURN
      m_stat = service( "ChronoStatSvc", true );
      return StatusCode::SUCCESS;
    }
    /// the main execution method
    StatusCode execute() override;

  private:
    // pointer to Stat Service
    SmartIF<IStatSvc> m_stat; ///< pointer to Stat Service
  };
} // end of namespace GaudiExamples
// ============================================================================
// the main execution method
// ============================================================================
StatusCode GaudiExamples::StatSvcAlg::execute() {
  Stat st1( m_stat, "counter1" );
  Stat st2( m_stat, "counter2" );
  Stat st3( m_stat, "counter3", 0.3 );

  Stat eff( m_stat, "eff", 0 < sin( 10 * st1->flag() ) );
  //
  st1 += 0.1;
  st1 -= 0.1000452;
  st2 += st1;
  ++st3;
  st2--;

  eff += 0 < cos( 20 * st2->flag() );

  //
  return StatusCode::SUCCESS;
}
// ============================================================================
// Factory:
// ============================================================================
using GaudiExamples::StatSvcAlg;
DECLARE_COMPONENT( StatSvcAlg )
// ============================================================================
// The END
// ============================================================================
