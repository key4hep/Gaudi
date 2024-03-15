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
#ifndef GAUDIEXAMPLE_MULTIINPUT_WRITEALG_H
#define GAUDIEXAMPLE_MULTIINPUT_WRITEALG_H

// Framework include files
#include "GaudiKernel/Algorithm.h"

namespace Gaudi {
  namespace TestSuite {
    namespace MultiInput {
      /** Simple algorithm used to produce dummy data in the transient store.
       * Modified (reduced) version of the IO example WriteAlg. */
      class WriteAlg : public ::Algorithm {
      public:
        /// Constructor: A constructor of this form must be provided.
        using Algorithm::Algorithm;
        /// Initialize
        StatusCode initialize() override;
        /// Event callback
        StatusCode execute() override;

      private:
        int                                m_runnum = 0, m_evtnum = 0;
        Gaudi::Property<std::vector<long>> m_randomSeeds{
            this, "RandomSeeds", {}, "Seeds to be used in the random number generation" };
      };
    } // namespace MultiInput
  }   // namespace TestSuite
} // namespace Gaudi
#endif // GAUDIEXAMPLE_MULTIINPUT_WRITEALG_H
