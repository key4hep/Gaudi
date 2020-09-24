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

#include "GaudiKernel/Service.h"

#include <Gaudi/MonitoringHub.h>

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TProfile3D.h>

#include <fmt/format.h>

#include <deque>
#include <algorithm>

namespace {

  struct Axis {
    unsigned int nBins;
    double minValue;
    double maxValue;
    std::string title;
  };
  
  Axis toAxis( nlohmann::json& jAxis ) {
    return { jAxis.at("nBins").get<unsigned int>(),
             jAxis.at("minValue").get<double>(),
             jAxis.at("maxValue").get<double>(),
             ";" + jAxis.at("title").get<std::string>() }; // ";" to prepare concatenations of titles
  }

  template <typename Traits, std::size_t... index>
  void saveRootHistoInternal( const std::string& component, const std::string& name, nlohmann::json& j,
                              std::index_sequence<index...> ) {
    auto id = fmt::format( "{}/{}", component, name );
    // extract data from json
    auto jsonAxis = j.at( "axis" );
    auto axis     = std::array{toAxis( jsonAxis[index] )...};
    auto weights  = j.at( "bins" ).get<std::vector<typename Traits::WeightType>>();
    auto title = j.at("title").get<std::string>();
    // weird way ROOT has to give titles to axis
    title += (axis[index].title + ...);
    // compute total number of bins, multiplying bins per axis
    auto totNBins = ((axis[index].nBins+2) * ...);
    assert(weights.size() == totNBins);
    // Create Root histogram calling constructors with the args tuple
    auto histo = std::make_from_tuple<typename Traits::Histo>(
        std::tuple_cat( std::tuple{id.c_str(), title.c_str()},
                        std::tuple{axis[index].nBins, axis[index].minValue, axis[index].maxValue}... ) );
    // fill Histo
    for ( unsigned int i = 0; i < totNBins; i++ ) Traits::fill( histo, i, weights[i] );
    // write to file
    histo.Write();
  }

  template <bool isProfile, typename RootHisto>
  struct Traits;

  template <typename RootHisto>
  struct Traits<false, RootHisto> {
    using Histo      = RootHisto;
    using WeightType = double;
    static auto fill( Histo& histo, unsigned int i, const WeightType& weight ) {
      histo.SetBinContent( i, weight );
    }
  };
  template <typename RootHisto>
  struct Traits<true, RootHisto> {
    /// Wrapper around TProfileX to be able to fill it
    template <typename TP>
    struct ProfileWrapper : TP {
      using TP::TP;
      void setBinNEntries( Int_t i, Int_t n ) { this->fBinEntries.fArray[i] = n; }
      void setBinW2( Int_t i, Double_t v ) { this->fSumw2.fArray[i] = v; }
    };
    using Histo      = ProfileWrapper<RootHisto>;
    using WeightType = std::tuple<std::tuple<unsigned int, double>, double>;
    static constexpr auto fill( Histo& histo, unsigned int i, const WeightType& weight ) {
      auto [c, sumWeight2]       = weight;
      auto [nEntries, sumWeight] = c;
      histo.setBinNEntries( i, nEntries );
      histo.SetBinContent( i, sumWeight );
      histo.setBinW2( i, sumWeight2 );
    };
  };

  template <unsigned int N, bool isProfile, typename ROOTHisto>
  void saveRootHisto( const std::string& component, const std::string& name, nlohmann::json& j ) {
    saveRootHistoInternal<Traits<isProfile, ROOTHisto>>( component, name, j, std::make_index_sequence<N>() );
  }

  using namespace std::string_literals;
  static const auto registry =
    std::map{std::pair{std::pair{"histogram:Histogram"s, 1}, &saveRootHisto<1, false, TH1D>},
             std::pair{std::pair{"histogram:WeightedHistogram"s, 1}, &saveRootHisto<1, false, TH1D>},
             std::pair{std::pair{"histogram:Histogram"s, 2}, &saveRootHisto<2, false, TH2D>},
             std::pair{std::pair{"histogram:WeightedHistogram"s, 2}, &saveRootHisto<2, false, TH2D>},
             std::pair{std::pair{"histogram:Histogram"s, 3}, &saveRootHisto<3, false, TH3D>},
             std::pair{std::pair{"histogram:WeightedHistogram"s, 3}, &saveRootHisto<3, false, TH3D>},
             std::pair{std::pair{"histogram:ProfileHistogram"s, 1}, &saveRootHisto<1, true, TProfile>},
             std::pair{std::pair{"histogram:WeightedProfileHistogram"s, 1}, &saveRootHisto<1, true, TProfile>},
             std::pair{std::pair{"histogram:ProfileHistogram"s, 2}, &saveRootHisto<2, true, TProfile2D>},
             std::pair{std::pair{"histogram:WeightedProfileHistogram"s, 2}, &saveRootHisto<2, true, TProfile2D>},
             std::pair{std::pair{"histogram:ProfileHistogram"s, 3}, &saveRootHisto<3, true, TProfile3D>},
             std::pair{std::pair{"histogram:WeightedProfileHistogram"s, 3}, &saveRootHisto<3, true, TProfile3D>}};
}

namespace Gaudi::Histograming::Sink {

  class Root : public Service, public Gaudi::Monitoring::Hub::Sink {

  public:

    using Service::Service;

    StatusCode initialize() override {
      StatusCode sc = Service::initialize();
      if ( sc.isFailure() ) return sc;
      serviceLocator()->monitoringHub().addSink( this );
      return StatusCode::SUCCESS;
    }

    StatusCode stop() override {
      auto ok = Service::stop();
      if ( !ok ) return ok;
      std::sort( begin( m_monitoringEntities ), end( m_monitoringEntities ), []( const auto& a, const auto& b ) {
        return a.component == b.component ? a.name > b.name : a.component > b.component;
      } );
      TFile histoFile( "testHisto.root", "RECREATE" );
      std::for_each( begin( m_monitoringEntities ), end( m_monitoringEntities ), []( auto& ent ) {
        auto j   = ent.toJSON();
        auto dim = j.at( "dimension" ).template get<unsigned int>();
        auto type  = j.at( "type" ).template get<std::string>();
        auto saver = registry.find( {type, dim} );
        if ( saver == registry.end() )
          throw GaudiException( "Unknown type : " + type + " dim : " + std::to_string( dim ), "Histogram::Sink::Root",
                                StatusCode::FAILURE );
        ( *saver->second )( ent.component, ent.name, j );
      } );
      histoFile.Close();
      return ok;
    }

    void registerEntity( Gaudi::Monitoring::Hub::Entity ent ) override {
      if ( std::string_view( ent.type ).substr( 0, 10 ) == "histogram:" ) {
        m_monitoringEntities.emplace_back( std::move( ent ) );
      }
    }

  private:
    std::deque<Gaudi::Monitoring::Hub::Entity> m_monitoringEntities;

  };

  DECLARE_COMPONENT( Root )

} // namespace Gaudi::Histograming::Sink

