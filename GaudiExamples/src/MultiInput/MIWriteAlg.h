#ifndef GAUDIEXAMPLE_MULTIINPUT_WRITEALG_H
#define GAUDIEXAMPLE_MULTIINPUT_WRITEALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"

namespace Gaudi
{
  namespace Examples
  {
    namespace MultiInput
    {
      /** Simple algorithm used to produce dummy data in the transient store.
       * Modified (reduced) version of the IO example WriteAlg. */
      class WriteAlg : public ::Algorithm
      {
      public:
        /// Constructor: A constructor of this form must be provided.
        using Algorithm::Algorithm;
        /// Initialize
        StatusCode initialize() override;
        /// Finalize
        StatusCode finalize() override;
        /// Event callback
        StatusCode execute() override;

      private:
        int                                m_runnum = 0, m_evtnum = 0;
        Gaudi::Property<std::vector<long>> m_randomSeeds{
            this, "RandomSeeds", {}, "Seeds to be used in the random number generation"};
      };
    }
  }
}
#endif // GAUDIEXAMPLE_MULTIINPUT_WRITEALG_H
