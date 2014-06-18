#ifndef GAUDIEXAMPLE_MULTIINPUT_WRITEALG_H
#define GAUDIEXAMPLE_MULTIINPUT_WRITEALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"

namespace Gaudi {
  namespace Examples {
    namespace MultiInput {
      /** Simple algorithm used to produce dummy data in the transient store.
       * Modified (reduced) version of the IO example WriteAlg. */
      class WriteAlg : public Algorithm {
      public:
        /// Constructor: A constructor of this form must be provided.
        WriteAlg(const std::string& name, ISvcLocator* pSvcLoc);
        /// Standard Destructor
        virtual ~WriteAlg();
        /// Initialize
        virtual StatusCode initialize();
        /// Finalize
        virtual StatusCode finalize();
        /// Event callback
        virtual StatusCode execute();
      private:
        int m_runnum, m_evtnum;
        std::vector<long> m_randomSeeds;
      };
    }
  }
}
#endif // GAUDIEXAMPLE_MULTIINPUT_WRITEALG_H
