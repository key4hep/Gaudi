/***********************************************************************************\
* (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations *
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

#include <TDirectory.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TProfile3D.h>

#include <algorithm>
#include <deque>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split_when.hpp>
#include <range/v3/view/transform.hpp>
#include <string>
#include <vector>

namespace {

  struct Axis {
    unsigned int nBins;
    double       minValue;
    double       maxValue;
    std::string  title;
  };

  Axis toAxis( nlohmann::json& jAxis ) {
    return {jAxis.at( "nBins" ).get<unsigned int>(), jAxis.at( "minValue" ).get<double>(),
            jAxis.at( "maxValue" ).get<double>(),
            ";" + jAxis.at( "title" ).get<std::string>()}; // ";" to prepare concatenations of titles
  }

  template <typename Traits, std::size_t... index>
  void saveRootHistoInternal( const std::string& name, nlohmann::json& j, std::index_sequence<index...> ) {
    // extract data from json
    auto jsonAxis = j.at( "axis" );
    auto axis     = std::array{toAxis( jsonAxis[index] )...};
    auto weights  = j.at( "bins" ).get<std::vector<typename Traits::WeightType>>();
    auto title    = j.at( "title" ).get<std::string>();
    // weird way ROOT has to give titles to axis
    title += ( axis[index].title + ... );
    // compute total number of bins, multiplying bins per axis
    auto totNBins = ( ( axis[index].nBins + 2 ) * ... );
    assert( weights.size() == totNBins );
    // Create Root histogram calling constructors with the args tuple
    auto histo = std::make_from_tuple<typename Traits::Histo>(
        std::tuple_cat( std::tuple{name.c_str(), title.c_str()},
                        std::tuple{axis[index].nBins, axis[index].minValue, axis[index].maxValue}... ) );
    // fill Histo
    for ( unsigned int i = 0; i < totNBins; i++ ) Traits::fill( histo, i, weights[i] );
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
    ( try_set_bin_labels( index ), ... );
    // write to file
    histo.Write();
  }

  template <bool isProfile, typename RootHisto>
  struct Traits;

  template <typename RootHisto>
  struct Traits<false, RootHisto> {
    using Histo      = RootHisto;
    using WeightType = double;
    static auto fill( Histo& histo, unsigned int i, const WeightType& weight ) { histo.SetBinContent( i, weight ); }
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
  void saveRootHisto( const std::string& name, nlohmann::json& j ) {
    saveRootHistoInternal<Traits<isProfile, ROOTHisto>>( name, j, std::make_index_sequence<N>() );
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
} // namespace

namespace Gaudi::Histograming::Sink {

  class Root : public Service, public Gaudi::Monitoring::Hub::Sink {

  public:
    using Service::Service;

    StatusCode initialize() override {
      return Service::initialize().andThen( [&] { serviceLocator()->monitoringHub().addSink( this ); } );
    }

    StatusCode stop() override {
      auto ok = Service::stop();
      if ( !ok ) return ok;
      std::sort( begin( m_monitoringEntities ), end( m_monitoringEntities ), []( const auto& a, const auto& b ) {
        return std::tie( a.name, a.component ) > std::tie( b.name, b.component );
      } );
      TFile histoFile( m_fileName.value().c_str(), "RECREATE" );
      std::for_each( begin( m_monitoringEntities ), end( m_monitoringEntities ), [&histoFile]( auto& ent ) {
        auto j    = ent.toJSON();
        auto dim  = j.at( "dimension" ).template get<unsigned int>();
        auto type = j.at( "type" ).template get<std::string>();
        // cut type after last ':' if there is one. The rest is precision parameter that we do not need here
        // as ROOT anyway treats everything as doubles in histograms
        type       = type.substr( 0, type.find_last_of( ':' ) );
        auto saver = registry.find( {type, dim} );
        if ( saver == registry.end() )
          throw GaudiException( "Unknown type : " + type + " dim : " + std::to_string( dim ), "Histogram::Sink::Root",
                                StatusCode::FAILURE );
        // convert (owner + name) to (dir, name) splitting the name on "/" if needed
        std::string dir  = ent.component;
        std::string name = ent.name;
        if ( auto pos = name.rfind( '/' ); pos != std::string::npos ) {
          dir += '/' + name.substr( 0, pos );
          name = name.substr( pos + 1 );
        }
        // find or create the directory for the histogram
        auto currentDir = histoFile.GetDirectory( "" );
        for ( const auto& dir_level :
              dir | ranges::view::split_when( []( auto c ) { return c == '/' || c == '.'; } ) |
                  ranges::view::transform( []( auto&& rng ) { return rng | ranges::to<std::string>; } ) ) {
          auto nextDir = currentDir->GetDirectory( dir_level.c_str() );
          if ( !nextDir ) nextDir = currentDir->mkdir( dir_level.c_str() );
          if ( !nextDir )
            throw GaudiException( "Could not create directory " + dir, "Histogram::Sink::Root", StatusCode::FAILURE );
          currentDir = nextDir;
        }
        // switch to the directory and save the histogram
        currentDir->cd();
        ( *saver->second )( name, j );
        histoFile.cd();
      } );
      return ok;
    }

    void registerEntity( Monitoring::Hub::Entity ent ) override {
      if ( std::string_view( ent.type ).substr( 0, 10 ) == "histogram:" ) {
        m_monitoringEntities.emplace_back( std::move( ent ) );
      }
    }

    void removeEntity( Monitoring::Hub::Entity const& ent ) override {
      auto it = std::find( begin( m_monitoringEntities ), end( m_monitoringEntities ), ent );
      if ( it != m_monitoringEntities.end() ) { m_monitoringEntities.erase( it ); }
    }

  private:
    std::deque<Gaudi::Monitoring::Hub::Entity> m_monitoringEntities;

    Gaudi::Property<std::string> m_fileName{this, "FileName", "testHisto.root",
                                            "Name of file where to save histograms"};
  };

  DECLARE_COMPONENT( Root )

} // namespace Gaudi::Histograming::Sink
