#ifndef GAUDIEXAMPLES_MULTIINPUT_READALG_H
#define GAUDIEXAMPLES_MULTIINPUT_READALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"
#include "RootCnv/RootAddress.h"
#include "MIHelpers.h"

#include <vector>

namespace Gaudi {
  namespace Examples {
    namespace MultiInput {
      /** Simple algorithm used to read data from two files. */
      class ReadAlg : public Algorithm {
      public:
        ReadAlg(const std::string& name, ISvcLocator* pSvcLoc);
        virtual ~ReadAlg();
        /// Initialize
        virtual StatusCode initialize();
        /// Finalize
        virtual StatusCode finalize();
        /// Event callback
        virtual StatusCode execute();
      private:
        std::string m_addressfile;
        /// Address details for the data to be added to the main event.
        std::vector<RootAddressArgs> m_addresses;
        size_t m_count;
      };
    }
  }
}
#endif
