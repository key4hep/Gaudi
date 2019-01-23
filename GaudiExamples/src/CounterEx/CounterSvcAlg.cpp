// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Algorithm.h"

// ============================================================================
namespace GaudiExamples
{
  /** @class CounterSvcAlg
   *  simple algorithm use for test case bug_30209_namespace_in_sequencer.qmt
   * nothing to do anymore with ICounterSvc
   */
  class CounterSvcAlg : public Algorithm
  {
  public:
    /// Constructor: A constructor of this form must be provided.
    using Algorithm::Algorithm;
    /// Event callback
    StatusCode execute() override { return StatusCode::SUCCESS; }
  };

  DECLARE_COMPONENT( CounterSvcAlg )

} // end of namespace GaudiExamples
