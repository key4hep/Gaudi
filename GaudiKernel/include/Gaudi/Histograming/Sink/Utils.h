/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/GaudiException.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <algorithm>
#include <functional>
#include <gsl/span>
#include <nlohmann/json.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split_when.hpp>
#include <range/v3/view/transform.hpp>
#include <string>
#include <vector>

// upstream has renamed namespace ranges::view -> ranges::views
#if RANGE_V3_VERSION < 900
namespace ranges::views {
  using namespace ranges::view;
}
#endif

namespace Gaudi::Histograming::Sink {

  /**
   * templated Traits dealing with Root Histogram filling for standard histograms
   *
   * Specializatoins of the Traits type must provide the following static functions :
   *   - Histo create( std::string& name, std::string& title, Axis& axis... )
   *       it should intanciate a new ROOT histogram instance and return it
   *   - void fillMetaData( Histo& histo, nlohmann::json const& jsonAxis, unsigned int nentries)
   *       it should fill metadata in the ROOT histo histogram provides from the list of axis number of entries
   *   - void fill( Histo& histo, unsigned int i, const WeightType& weight )
   *       it should fill the given bin with the given value
   * Last point, it should have a static constexpr unsigned int Dimension
   */
  template <bool isProfile, typename RootHisto, unsigned int N>
  struct Traits;

  /**
   * generic function to convert json to a ROOT Histogram
   *
   * returns the Root histogram and the dir where to save it in the Root file
   * This may be different from input dir in case name has slashes
   */
  template <typename Traits>
  std::tuple<typename Traits::Histo, std::string> jsonToRootHistogram( std::string& dir, std::string& name,
                                                                       nlohmann::json const& j );

  /**
   * generic function to convert a ROOT Histogram to json
   *
   * essentially used for backward compatibility of old HistogramService with MonitoringHub
   */
  template <typename Histo>
  nlohmann::json rootHistogramTojson( Histo const& );

  /**
   * generic method to save histograms to files, based on Traits
   */
  template <typename Traits>
  void saveRootHisto( TFile& file, std::string dir, std::string name, nlohmann::json const& j );

  /**
   * generic method to save regular histograms to files
   *
   * Can be used in most cases as the handler function to register into Sink::Base
   * contains all the boiler plate code and redirects specific code to the adapted Traits template
   */
  template <unsigned int N, bool isProfile, typename ROOTHisto>
  void saveRootHisto( TFile& file, std::string dir, std::string name, nlohmann::json const& j );

  namespace details {

    /// Small helper struct representing the Axis of an Histogram
    struct Axis {
      unsigned int nBins;
      double       minValue;
      double       maxValue;
      std::string  title;
    };

    /// extract an Axis from json data
    inline Axis jsonToAxis( nlohmann::json& jAxis ) {
      return { jAxis.at( "nBins" ).get<unsigned int>(), jAxis.at( "minValue" ).get<double>(),
               jAxis.at( "maxValue" ).get<double>(),
               ";" + jAxis.at( "title" ).get<std::string>() }; // ";" to prepare concatenations of titles
    }

    /**
     * generic function to convert json to a ROOT Histogram - internal implementation
     */
    template <typename Traits, std::size_t... index>
    std::tuple<typename Traits::Histo, std::string> jsonToRootHistogramInternal( std::string& dir, std::string& name,
                                                                                 nlohmann::json const& j,
                                                                                 std::index_sequence<index...> ) {
      // extract data from json
      auto jsonAxis = j.at( "axis" );
      auto axis     = std::array{ jsonToAxis( jsonAxis[index] )... };
      auto weights  = j.at( "bins" ).get<std::vector<typename Traits::WeightType>>();
      auto title    = j.at( "title" ).get<std::string>();
      auto nentries = j.at( "nEntries" ).get<unsigned int>();
      // weird way ROOT has to give titles to axis
      title += ( axis[index].title + ... );
      // compute total number of bins, multiplying bins per axis
      auto totNBins = ( ( axis[index].nBins + 2 ) * ... );
      assert( weights.size() == totNBins );

      if ( name[0] == '/' ) {
        dir  = "";
        name = name.substr( 1 );
      }

      // take into account the case where name contains '/'s (e.g. "Group/Name") by
      // moving the prefix into dir
      if ( auto pos = name.rfind( '/' ); pos != std::string::npos ) {
        dir += '/' + name.substr( 0, pos );
        name = name.substr( pos + 1 );
      }

      // Create Root histogram calling constructors with the args tuple
      auto histo = Traits::create( name, title, axis[index]... );

      // fill Histo
      for ( unsigned int i = 0; i < totNBins; i++ ) Traits::fill( histo, i, weights[i] );
      // fill histo metadata, e.g. bins and number of entries
      Traits::fillMetaData( histo, jsonAxis, nentries );

      return { histo, dir };
    }

    /**
     * Common base for Traits dealing with Histogram conversions to Root
     * Provides generic implementation for creating the histogram and filling meta data
     * The filling (method fill) is not implemented
     */
    template <typename RootHisto, unsigned int N>
    struct TraitsBase {
      static constexpr unsigned int Dimension{ N };
      // hleper method for the creation of the Root histogram, using an index_sequence for handling the axis
      template <typename AxisArray, std::size_t... index>
      static RootHisto create( std::string& name, std::string& title, AxisArray axis, std::index_sequence<index...> ) {
        return std::make_from_tuple<RootHisto>(
            std::tuple_cat( std::tuple{ name.c_str(), title.c_str() },
                            std::tuple{ std::get<index>( axis ).nBins, std::get<index>( axis ).minValue,
                                        std::get<index>( axis ).maxValue }... ) );
      }
      // Generic creation of the Root histogram from name, title and a set of axis
      template <typename... Axis>
      static RootHisto create( std::string& name, std::string& title, Axis&... axis ) {
        return create( name, title, std::make_tuple( axis... ), std::make_index_sequence<sizeof...( Axis )>() );
      }
      // Generic fill of the metadata of the Root histogram from json data
      static void fillMetaData( RootHisto& histo, nlohmann::json const& jsonAxis, unsigned int nentries ) {
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
        for ( unsigned int i = 0; i < jsonAxis.size(); i++ ) try_set_bin_labels( i );
        // Fix the number of entries, wrongly computed by ROOT from the numer of calls to fill
        histo.SetEntries( nentries );
      }
    };

    /// helper Wrapper around TProfileX to be able to fill it
    template <typename TP>
    struct ProfileWrapper : TP {
      template <typename... Args>
      ProfileWrapper( Args&&... args ) : TP( std::forward<Args>( args )... ) {
        // When the static function TH1::SetDefaultSumw2 had been called (passing true), `Sumw2(true)` is automatically
        // called by the constructor of TProfile. This is a problem because in the profiles in Gaudi::Accumulators we do
        // not keep track of the sum of squares of weights and consequently don't set fBinSumw2 of the TProfile, which
        // in turn leads to a self-inconsistent TProfile object. The "fix" is to disable sum of squares explicitly.
        this->Sumw2( false );
      }
      ProfileWrapper( ProfileWrapper const& ) = delete;
      ProfileWrapper& operator=( ProfileWrapper const& ) = delete;
      ProfileWrapper( ProfileWrapper const&& )           = delete;
      ProfileWrapper& operator=( ProfileWrapper const&& ) = delete;
      void            setBinNEntries( Int_t i, Int_t n ) { this->fBinEntries.fArray[i] = n; }
      void            setBinW2( Int_t i, Double_t v ) { this->fSumw2.fArray[i] = v; }
    };

    /**
     * changes to the ROOT directory given in the current ROOT file and returns
     * the current directory before the change
     */
    inline TDirectory* changeDir( TFile& file, std::string dir ) {
      // remember the current directory
      auto previousDir = gDirectory;
      // find or create the directory for the histogram
      using namespace ranges;
      auto is_delimiter        = []( auto c ) { return c == '/' || c == '.'; };
      auto transform_to_string = views::transform( []( auto&& rng ) { return rng | to<std::string>; } );
      auto currentDir          = accumulate( dir | views::split_when( is_delimiter ) | transform_to_string,
                                             file.GetDirectory( "" ), []( auto current, auto&& dir_level ) {
                                      if ( current ) {
                                        // try to get next level
                                        auto nextDir = current->GetDirectory( dir_level.c_str() );
                                        // if it does not exist, create it
                                        if ( !nextDir ) nextDir = current->mkdir( dir_level.c_str() );
                                        // move to next level
                                        current = nextDir;
                                      }
                                      return current;
                                             } );
      if ( !currentDir )
        throw GaudiException( "Could not create directory " + dir, "Histogram::Sink::Root", StatusCode::FAILURE );
      // switch to the directory
      currentDir->cd();
      return previousDir;
    }

    template <typename Histo>
    nlohmann::json allAxisTojson( Histo const& h ) {
      if constexpr ( std::is_base_of_v<TH3D, Histo> ) {
        return { *h.GetXaxis(), *h.GetYaxis(), *h.GetZaxis() };
      } else if constexpr ( std::is_base_of_v<TProfile2D, Histo> || std::is_base_of_v<TH2D, Histo> ) {
        return { *h.GetXaxis(), *h.GetYaxis() };
      } else {
        return { *h.GetXaxis() };
      }
    }

    template <typename Histo>
    nlohmann::json binsTojson( Histo const& h ) {
      if constexpr ( std::is_base_of_v<TProfile, Histo> || std::is_base_of_v<TProfile2D, Histo> ) {
        nlohmann::json j;
        // ROOT TProfile interface being completely inconsistent, we have to play
        // with different ways to access values of the histogram... one per value !
        auto* sums  = h.GetArray();
        auto* sums2 = h.GetSumw2();
        for ( unsigned long n = 0; n < (unsigned long)h.GetSize(); n++ ) {
          j.push_back( nlohmann::json{ { h.GetBinEntries( n ), sums[n] }, sums2->At( n ) } );
        }
        return j;
      } else {
        return gsl::span{ h.GetArray(), (unsigned long)h.GetSize() };
      }
    }

    /// automatic translation of Root Histograms to json
    template <typename Histo>
    nlohmann::json rootHistogramToJson( Histo const& h ) {
      std::string type = std::is_base_of_v<TProfile, Histo> || std::is_base_of_v<TProfile2D, Histo>
                             ? "histogram:ProfileHistogram:double"
                             : "histogram:Histogram:double";
      return nlohmann::json{ { "type", type },
                             { "title", h.GetTitle() },
                             { "dimension", h.GetDimension() },
                             { "empty", (int)h.GetEntries() == 0 },
                             { "nEntries", (int)h.GetEntries() },
                             { "axis", allAxisTojson( h ) },
                             { "bins", binsTojson( h ) } };
    }

  } // namespace details

  /**
   * Specialization of Traits dealing with non profile Root Histograms
   */
  template <typename RootHisto, unsigned int N>
  struct Traits<false, RootHisto, N> : details::TraitsBase<RootHisto, N> {
    using Histo      = RootHisto;
    using WeightType = double;
    static void fill( Histo& histo, unsigned int i, const WeightType& weight ) { histo.SetBinContent( i, weight ); }
  };

  /**
   * Specialization of Traits dealing with profile Root Histograms
   */
  template <typename RootHisto, unsigned int N>
  struct Traits<true, RootHisto, N> : details::TraitsBase<details::ProfileWrapper<RootHisto>, N> {
    using Histo      = details::ProfileWrapper<RootHisto>;
    using WeightType = std::tuple<std::tuple<unsigned int, double>, double>;
    static constexpr void fill( Histo& histo, unsigned int i, const WeightType& weight ) {
      auto [c, sumWeight2]       = weight;
      auto [nEntries, sumWeight] = c;
      histo.setBinNEntries( i, nEntries );
      histo.SetBinContent( i, sumWeight );
      histo.setBinW2( i, sumWeight2 );
    };
  };

  template <typename Traits>
  std::tuple<typename Traits::Histo, std::string> jsonToRootHistogram( std::string& dir, std::string& name,
                                                                       nlohmann::json const& j ) {
    return details::jsonToRootHistogramInternal<Traits>( dir, name, j, std::make_index_sequence<Traits::Dimension>() );
  }

  template <typename Traits>
  void saveRootHisto( TFile& file, std::string dir, std::string name, nlohmann::json const& j ) {
    // get the Root histogram
    auto [histo, newDir] = jsonToRootHistogram<Traits>( dir, name, j );
    // Change to the proper directory in the ROOT file
    auto previousDir = details::changeDir( file, newDir );
    // write to file
    histo.Write();
    // switch back to the previous directory
    previousDir->cd();
  }

  template <unsigned int N, bool isProfile, typename ROOTHisto>
  void saveRootHisto( TFile& file, std::string dir, std::string name, nlohmann::json const& j ) {
    saveRootHisto<Traits<isProfile, ROOTHisto, N>>( file, std::move( dir ), std::move( name ), j );
  }

} // namespace Gaudi::Histograming::Sink

namespace nlohmann {
  /// automatic translation of TAxis to json
  inline void to_json( nlohmann::json& j, TAxis const& axis ) {
    j = nlohmann::json{ { "nBins", axis.GetNbins() },
                        { "minValue", axis.GetXmin() },
                        { "maxValue", axis.GetXmax() },
                        { "title", axis.GetTitle() } };
  }
  /// automatic translation of Root histograms to json
  inline void to_json( nlohmann::json& j, TH1D const& h ) {
    j = Gaudi::Histograming::Sink::details::rootHistogramToJson( h );
  }
  inline void to_json( nlohmann::json& j, TH2D const& h ) {
    j = Gaudi::Histograming::Sink::details::rootHistogramToJson( h );
  }
  inline void to_json( nlohmann::json& j, TH3D const& h ) {
    j = Gaudi::Histograming::Sink::details::rootHistogramToJson( h );
  }
  inline void to_json( nlohmann::json& j, TProfile const& h ) {
    j = Gaudi::Histograming::Sink::details::rootHistogramToJson( h );
  }
  inline void to_json( nlohmann::json& j, TProfile2D const& h ) {
    j = Gaudi::Histograming::Sink::details::rootHistogramToJson( h );
  }

} // namespace nlohmann
