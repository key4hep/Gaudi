/***********************************************************************************\
* (c) Copyright 1998-2022 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

#include "GaudiKernel/Service.h"

#include <HistogramPersistencySvc/RootHistogramSinkBase.h>
#include <HistogramPersistencySvc/RootHistogramUtils.h>

#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TProfile3D.h>

namespace Gaudi::Histograming::Sink {

  using namespace std::string_literals;

  struct Root : Base {
    using Base::Base;
    HistoRegistry registry = { { { "histogram:Histogram"s, 1 }, &saveRootHisto<1, false, TH1D> },
                               { { "histogram:WeightedHistogram"s, 1 }, &saveRootHisto<1, false, TH1D> },
                               { { "histogram:Histogram"s, 2 }, &saveRootHisto<2, false, TH2D> },
                               { { "histogram:WeightedHistogram"s, 2 }, &saveRootHisto<2, false, TH2D> },
                               { { "histogram:Histogram"s, 3 }, &saveRootHisto<3, false, TH3D> },
                               { { "histogram:WeightedHistogram"s, 3 }, &saveRootHisto<3, false, TH3D> },
                               { { "histogram:ProfileHistogram"s, 1 }, &saveRootHisto<1, true, TProfile> },
                               { { "histogram:WeightedProfileHistogram"s, 1 }, &saveRootHisto<1, true, TProfile> },
                               { { "histogram:ProfileHistogram"s, 2 }, &saveRootHisto<2, true, TProfile2D> },
                               { { "histogram:WeightedProfileHistogram"s, 2 }, &saveRootHisto<2, true, TProfile2D> },
                               { { "histogram:ProfileHistogram"s, 3 }, &saveRootHisto<3, true, TProfile3D> },
                               { { "histogram:WeightedProfileHistogram"s, 3 }, &saveRootHisto<3, true, TProfile3D> } };

    StatusCode initialize() override {
      return Base::initialize().andThen( [&] {
        for ( auto& [id, func] : registry ) { registerHandler( id, func ); }
      } );
    }
  };

  DECLARE_COMPONENT( Root )

} // namespace Gaudi::Histograming::Sink
