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
// STD&STL
// ============================================================================
#include <algorithm>
#include <functional>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IAlgContextSvc.h"
#include "GaudiKernel/IAlgorithm.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GetAlg.h"
#include "GaudiAlg/GetAlgs.h"
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
#include "GaudiAlg/GaudiHistoAlg.h"
#include "GaudiAlg/GaudiSequencer.h"
#include "GaudiAlg/GaudiTupleAlg.h"
#include "GaudiAlg/Sequencer.h"
// ============================================================================
/** @file
 *  Implementation file for the functions from the file GaudiKernel/GetAlgs.h
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ============================================================================
namespace {
  template <class TYPE>
  TYPE* getAlg( const IAlgContextSvc* svc ) {
    if ( !svc ) { return nullptr; } // RETURN
    const auto& algs = svc->algorithms();
    auto        it   = std::find_if( algs.rbegin(), algs.rend(), Gaudi::Utils::AlgTypeSelector<TYPE>{} );
    if ( algs.rend() == it ) { return nullptr; } // RETURN
    IAlgorithm* alg = *it;
    return dynamic_cast<TYPE*>( alg ); // RETURN
  }
} // namespace
// ============================================================================
/*  simple function to extract the last active
 *   GaudiAlgorithm from the context
 *
 *  @code
 *
 *  // get the context service:
 *  const IAlgContextSvc* svc = ... ;
 *
 *  GaudiAlgorithm* ga = getGaudiAlg ( svc ) ;
 *
 *  @endcode
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ============================================================================
GaudiAlgorithm* Gaudi::Utils::getGaudiAlg( const IAlgContextSvc* svc ) { return getAlg<GaudiAlgorithm>( svc ); }
// ============================================================================
/*  simple function to extract the last active
 *   GaudiHistoAlg from the context
 *
 *  @code
 *
 *  // get the context service:
 *  const IAlgContextSvc* svc = ... ;
 *
 *  GaudiHistoAlg* ha = getHistoAlg ( svc ) ;
 *
 *  @endcode
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ============================================================================
GaudiHistoAlg* Gaudi::Utils::getHistoAlg( const IAlgContextSvc* svc ) { return getAlg<GaudiHistoAlg>( svc ); }
// ============================================================================
/* simple function to extract the last active
 *   GaudiTupleAlg from the context
 *
 *  @code
 *
 *  // get the context service:
 *  const IAlgContextSvc* svc = ... ;
 *
 *  GaudiTupleAlg* ta = getTupleAlg ( svc ) ;
 *
 *  @endcode
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ============================================================================
GaudiTupleAlg* Gaudi::Utils::getTupleAlg( const IAlgContextSvc* svc ) { return getAlg<GaudiTupleAlg>( svc ); }
// ============================================================================
/*  simple function to extract the last active
 *   GaudiSequencer from the context
 *
 *  @code
 *
 *  // get the context service:
 *  const IAlgContextSvc* svc = ... ;
 *
 *  GaudiSequencer* sa = getGaudiSequencer ( svc ) ;
 *
 *  @endcode
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ============================================================================
GaudiSequencer* Gaudi::Utils::getGaudiSequencer( const IAlgContextSvc* svc ) { return getAlg<GaudiSequencer>( svc ); }
// ============================================================================
/* simple function to extract the last active
 *   Sequencer from the context
 *
 *  @code
 *
 *  // get the context service:
 *  const IAlgContextSvc* svc = ... ;
 *
 *  Sequencer* sa = getSequencerAlg ( svc ) ;
 *
 *  @endcode
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
Sequencer* Gaudi::Utils::getSequencerAlg( const IAlgContextSvc* svc ) { return getAlg<Sequencer>( svc ); }
// ========================================================================
/* simple function to extract the last active
 *   "Sequencer" () GaudiSequencer or Sequencer)
 *   from the context
 *
 *  @code
 *
 *  // get the context service:
 *  const IAlgContextSvc* svc = ... ;
 *
 *  IAlgorithm* a = getSequencer ( svc ) ;
 *
 *  @endcode
 *
 *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
 *  @date 2007-09-07
 */
// ========================================================================
IAlgorithm* Gaudi::Utils::getSequencer( const IAlgContextSvc* svc ) {
  if ( !svc ) { return nullptr; } // RETURN
  //
  AlgTypeSelector<GaudiSequencer> sel1;
  AlgTypeSelector<Sequencer>      sel2;

  const auto& algs = svc->algorithms();
  auto a = std::find_if( algs.rbegin(), algs.rend(), [&]( IAlgorithm* alg ) { return sel1( alg ) || sel2( alg ); } );
  return a != algs.rend() ? *a : nullptr;
}
// ========================================================================

// ============================================================================
// The END
// ============================================================================
