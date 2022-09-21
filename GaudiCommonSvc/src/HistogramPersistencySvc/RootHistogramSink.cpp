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

#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TProfile3D.h>

namespace {

  template <typename RootHisto, unsigned int N>
  struct TraitsBase {
    static constexpr unsigned int Dimension{N};
    template <typename AxisArray, std::size_t... index>
    static RootHisto create( std::string& name, std::string& title, AxisArray axis, std::index_sequence<index...> ) {
      return std::make_from_tuple<RootHisto>
        (std::tuple_cat( std::tuple{ name.c_str(), title.c_str() },
                         std::tuple{ std::get<index>( axis ).nBins, std::get<index>( axis ).minValue, std::get<index>( axis ).maxValue }... ) );
    }
    template <typename... Axis>
    static RootHisto create( std::string& name, std::string& title, Axis&... axis ) {
      return create( name, title, std::make_tuple( axis... ), std::make_index_sequence<sizeof...( Axis )>() );
    }
    static void fillMetaData( RootHisto& histo, nlohmann::json const& jsonAxis, unsigned int nentries) {
      auto try_set_bin_labels = [&histo, &jsonAxis]( auto idx ) {
        if ( jsonAxis[idx].contains( "labels" ) ) {
          TAxis* axis = nullptr;
          switch ( idx ) {
          case 0:
            axis = histo.GetXaxis();
            break;
          case 1:
            axis = histo.GetYaxis();
            break;
          case 2:
            axis = histo.GetZaxis();
            break;
          default:
            break;
          }
          if ( axis ) {
            const auto labels = jsonAxis[idx].at( "labels" );
            for ( unsigned int i = 0; i < labels.size(); i++ ) {
              axis->SetBinLabel( i + 1, labels[i].template get<std::string>().c_str() );
            }
          }
        }
      };
      for (unsigned int i = 0; i < jsonAxis.size(); i++) try_set_bin_labels( i );
      // Fix the number of entries, wrongly computed by ROOT from the numer of calls to fill
      histo.SetEntries( nentries );
    }
  };

  template <bool isProfile, typename RootHisto, unsigned int N>
  struct Traits;

  template <typename RootHisto, unsigned int N>
  struct Traits<false, RootHisto, N> : TraitsBase<RootHisto, N> {
    using Histo      = RootHisto;
    using WeightType = double;
    static void fill( Histo& histo, unsigned int i, const WeightType& weight ) { histo.SetBinContent( i, weight ); }
  };
  /// Wrapper around TProfileX to be able to fill it
  template <typename TP>
  struct ProfileWrapper : TP {
    using TP::TP;
    void setBinNEntries( Int_t i, Int_t n ) { this->fBinEntries.fArray[i] = n; }
    void setBinW2( Int_t i, Double_t v ) { this->fSumw2.fArray[i] = v; }
  };
  template <typename RootHisto, unsigned int N>
  struct Traits<true, RootHisto, N> : TraitsBase<ProfileWrapper<RootHisto>, N>  {
    using Histo      = ProfileWrapper<RootHisto>;
    using WeightType = std::tuple<std::tuple<unsigned int, double>, double>;
    static constexpr void fill( Histo& histo, unsigned int i, const WeightType& weight ) {
      auto [c, sumWeight2]       = weight;
      auto [nEntries, sumWeight] = c;
      histo.setBinNEntries( i, nEntries );
      histo.SetBinContent( i, sumWeight );
      histo.setBinW2( i, sumWeight2 );
    };
  };

} // namespace

namespace Gaudi::Histograming::Sink {

  using namespace std::string_literals;

  template <unsigned int N, bool isProfile, typename ROOTHisto>
  void saveRootHisto( TFile& file, std::string dir, std::string name, nlohmann::json& j ) {
    details::saveRootHisto<Traits<isProfile, ROOTHisto, N>>( file, std::move( dir ), std::move( name ), j );
  }
  
  struct Root : Base {

    using Base::Base;

    HistoRegistry registry = {
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

    StatusCode initialize() override {
      return Base::initialize().andThen( [&] {
        for( auto& [id, func] : registry ) {
          registerHandler( id, func );
        }
      } );
    }
    
  };

  DECLARE_COMPONENT( Root )
}
