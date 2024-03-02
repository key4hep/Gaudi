/***********************************************************************************\
* (c) Copyright 2023 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <Gaudi/Accumulators.h>
#include <Gaudi/Functional/Consumer.h>

namespace Gaudi::TestSuite {
  struct EventCounter final : Gaudi::Functional::Consumer<void()> {
    using Consumer::Consumer;

    void operator()() const override { ++m_count; }

    mutable Gaudi::Accumulators::Counter<> m_count{ this, "count" };
  };
  DECLARE_COMPONENT( EventCounter );
} // namespace Gaudi::TestSuite
