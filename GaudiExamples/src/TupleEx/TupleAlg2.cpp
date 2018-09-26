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
#include <utility>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiTupleAlg.h"
#include "GaudiAlg/Tuple.h"
// ============================================================================
// local
// ============================================================================
#include "TupleDef.h"
// ============================================================================

// ============================================================================
/** @class TupleAlg2
 *
 *  Example of usage of partial specializations for Ntupel columns
 *
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date 2005-11-29
 */
// ============================================================================

class TupleAlg2 : public GaudiTupleAlg {
public:
  /// initialize the algorithm
  StatusCode initialize() override {
    StatusCode sc = GaudiTupleAlg::initialize();
    if ( sc.isFailure() ) { return sc; }
    // check for random numbers service
    Assert( randSvc() != 0, "Random Service is not available!" );
    //
    return StatusCode::SUCCESS;
  };
  /** the only one essential method
   *  @see IAlgoruthm
   */
  StatusCode execute() override;

  /** standard constructor
   *  @param name algorithm instance name
   *  @param pSvc pointer to Service Locator
   */
  using GaudiTupleAlg::GaudiTupleAlg;

private:
  // copy constructor is disabled
  TupleAlg2( const TupleAlg2& ) = delete;
  // assignement op[erator is disabled
  TupleAlg2& operator=( const TupleAlg2& ) = delete;
};

// ============================================================================
DECLARE_COMPONENT( TupleAlg2 )
// ============================================================================

// ============================================================================
/** the only one essential method
 *  @see IAlgoruthm
 */
// ============================================================================
StatusCode TupleAlg2::execute() {
  /// avoid long names
  using namespace Tuples;
  using namespace TupleExample;

  Rndm::Numbers gauss( randSvc(), Rndm::Gauss( 0.0, 1.0 ) );
  Rndm::Numbers flat( randSvc(), Rndm::Flat( -10.0, 10.0 ) );
  Rndm::Numbers expo( randSvc(), Rndm::Exponential( 1.0 ) );
  Rndm::Numbers breit( randSvc(), Rndm::BreitWigner( 0.0, 1.0 ) );
  Rndm::Numbers poisson( randSvc(), Rndm::Poisson( 2.0 ) );
  Rndm::Numbers binom( randSvc(), Rndm::Binomial( 8, 0.25 ) );

  // ==========================================================================
  // book and fill simple NTuple with "dublets"
  // ==========================================================================
  Tuple tuple1 = nTuple( "dublets", "Tuple with dublets" );

  Dublet d1 = Dublet( gauss(), gauss() );
  Dublet d2 = Dublet( flat(), flat() );
  Dublet d3 = Dublet( breit(), breit() );
  Dublet d4 = Dublet( expo(), expo() );

  tuple1 << Column( "d1", d1 ) << Column( "d2", d2 ) << Column( "d3", d3 ) << Column( "d4", d4 );

  tuple1->write().ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );

  // ==========================================================================
  // book and fill simple NTuple with "triplets"
  // ==========================================================================
  Tuple tuple2 = nTuple( "triplets", "Tuple with triplets" );

  Triplet tr1;
  tr1.first.first  = gauss();
  tr1.first.second = gauss();
  tr1.second       = gauss();

  Triplet tr2;
  tr2.first.first  = flat();
  tr2.first.second = flat();
  tr2.second       = flat();

  tuple2 << Column( "tr1", tr1 );
  tuple2 << Column( "tr2", tr2 );

  tuple2->write().ignore( /* AUTOMATICALLY ADDED FOR gaudi/Gaudi!763 */ );

  return StatusCode::SUCCESS;
}
// ============================================================================

// ============================================================================
// The END
// ============================================================================
