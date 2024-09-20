/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIEXAMPLES_MULTIINPUT_READALG_H
#define GAUDIEXAMPLES_MULTIINPUT_READALG_H

// Framework include files
#include "MIHelpers.h"
#include <GaudiKernel/Algorithm.h>
#include <RootCnv/RootAddress.h>

#include <vector>

namespace Gaudi {
  namespace TestSuite {
    namespace MultiInput {
      /** Simple algorithm used to read data from two files. */
      class ReadAlg : public ::Algorithm {
      public:
        using Algorithm::Algorithm;
        /// Initialize
        StatusCode initialize() override;
        /// Event callback
        StatusCode execute() override;

      private:
        Gaudi::Property<std::string> m_addressfile{
            this, "AddressesFile", {}, "File containing the address details of the extra data." };
        /// Address details for the data to be added to the main event.
        std::vector<RootAddressArgs> m_addresses;
        size_t                       m_count = 0;
      };
    } // namespace MultiInput
  }   // namespace TestSuite
} // namespace Gaudi
#endif
