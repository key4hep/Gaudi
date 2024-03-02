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
#include "GaudiKernel/Algorithm.h"

// ============================================================================
namespace GaudiTestSuite {
  /** @class CounterSvcAlg
   *  simple algorithm use for test case bug_30209_namespace_in_sequencer.qmt
   * nothing to do anymore with ICounterSvc
   */
  class CounterSvcAlg : public Algorithm {
  public:
    /// Constructor: A constructor of this form must be provided.
    using Algorithm::Algorithm;
    /// Event callback
    StatusCode execute() override { return StatusCode::SUCCESS; }
  };

  DECLARE_COMPONENT( CounterSvcAlg )

} // end of namespace GaudiTestSuite
