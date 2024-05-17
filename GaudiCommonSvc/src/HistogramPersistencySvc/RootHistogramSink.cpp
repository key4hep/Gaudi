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

#include <Gaudi/Accumulators/StaticHistogram.h>
#include <Gaudi/Histograming/Sink/Base.h>
#include <Gaudi/Histograming/Sink/Utils.h>
#include <GaudiKernel/Service.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TProfile3D.h>

namespace Gaudi::Histograming::Sink {

  namespace {
    using namespace std::string_literals;
    Base::HistoBinRegistry const binRegistry = {
        { std::type_index( typeid( Accumulators::StaticProfileHistogram<1u, Accumulators::atomicity::full, double> ) ),
          &saveProfileHisto<1, Accumulators::atomicity::full, double> },
        { std::type_index( typeid( Accumulators::StaticProfileHistogram<1u, Accumulators::atomicity::none, double> ) ),
          &saveProfileHisto<1, Accumulators::atomicity::none, double> },
        { std::type_index( typeid( Accumulators::StaticProfileHistogram<1u, Accumulators::atomicity::full, float> ) ),
          &saveProfileHisto<1, Accumulators::atomicity::full, float> },
        { std::type_index( typeid( Accumulators::StaticProfileHistogram<1u, Accumulators::atomicity::none, float> ) ),
          &saveProfileHisto<1, Accumulators::atomicity::none, float> },
        { std::type_index( typeid( Accumulators::StaticProfileHistogram<2u, Accumulators::atomicity::full, double> ) ),
          &saveProfileHisto<2, Accumulators::atomicity::full, double> },
        { std::type_index( typeid( Accumulators::StaticProfileHistogram<2u, Accumulators::atomicity::none, double> ) ),
          &saveProfileHisto<2, Accumulators::atomicity::none, double> },
        { std::type_index( typeid( Accumulators::StaticProfileHistogram<2u, Accumulators::atomicity::full, float> ) ),
          &saveProfileHisto<2, Accumulators::atomicity::full, float> },
        { std::type_index( typeid( Accumulators::StaticProfileHistogram<2u, Accumulators::atomicity::none, float> ) ),
          &saveProfileHisto<2, Accumulators::atomicity::none, float> },
        { std::type_index( typeid( Accumulators::StaticProfileHistogram<3u, Accumulators::atomicity::full, double> ) ),
          &saveProfileHisto<3, Accumulators::atomicity::full, double> },
        { std::type_index( typeid( Accumulators::StaticProfileHistogram<3u, Accumulators::atomicity::none, double> ) ),
          &saveProfileHisto<3, Accumulators::atomicity::none, double> },
        { std::type_index( typeid( Accumulators::StaticProfileHistogram<3u, Accumulators::atomicity::full, float> ) ),
          &saveProfileHisto<3, Accumulators::atomicity::full, float> },
        { std::type_index( typeid( Accumulators::StaticProfileHistogram<3u, Accumulators::atomicity::none, float> ) ),
          &saveProfileHisto<3, Accumulators::atomicity::none, float> },
    };
    Base::HistoRegistry const registry = {
        { { "histogram:Histogram"s, 1 }, &saveRootHisto<1, false, TH1D> },
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
  } // namespace

  struct Root : Base {
    using Base::Base;

    StatusCode initialize() override {
      return Base::initialize().andThen( [&] {
        for ( auto& [id, func] : binRegistry ) { registerHandler( id, func ); }
        for ( auto& [id, func] : registry ) { registerHandler( id, func ); }
      } );
    }
  };

  DECLARE_COMPONENT( Root )

} // namespace Gaudi::Histograming::Sink
