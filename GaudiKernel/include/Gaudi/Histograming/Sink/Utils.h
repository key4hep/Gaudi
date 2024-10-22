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
#pragma once

#include <Gaudi/Accumulators/StaticHistogram.h>
#include <Gaudi/MonitoringHub.h>
#include <GaudiKernel/GaudiException.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TProfile3D.h>
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
   *   - void fill( Histo& histo, unsigned int nbins, const WeightType& weight )
   *       it should fill the given number of bin with the given values
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
     * handles cases where name includes '/' character(s) and move needed part of it
     * to dir. Also handle case of absolute names
     */
    inline void fixNameAndDir( std::string& name, std::string& dir ) {
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
      auto weights  = j.at( "bins" ).get<typename Traits::WeightType>();
      auto title    = j.at( "title" ).get<std::string>();
      auto nentries = j.at( "nEntries" ).get<unsigned int>();
      // weird way ROOT has to give titles to axis
      title += ( axis[index].title + ... );
      // compute total number of bins, multiplying bins per axis
      auto totNBins = ( ( axis[index].nBins + 2 ) * ... );
      // fix name and dir if needed
      fixNameAndDir( name, dir );
      // Create Root histogram calling constructors with the args tuple
      auto histo = Traits::create( name, title, axis[index]... );
      // fill Histo
      Traits::fill( histo, totNBins, weights );
      // in case we have sums, overwrite them in the histogram with our more precise values
      // FIXME This is only supporting regular histograms. It won't work in case of weighted histograms
      if constexpr ( sizeof...( index ) == 1 ) {
        if ( j.find( "sum" ) != j.end() ) {
          double s[13];
          s[0] = j.at( "nTotEntries" ).get<double>();
          s[1] = j.at( "nTotEntries" ).get<double>();
          s[2] = j.at( "sum" ).get<double>();
          s[3] = j.at( "sum2" ).get<double>();
          histo.PutStats( s );
        }
      } else {
        if ( j.find( "sumx" ) != j.end() ) {
          double s[13];
          s[0] = j.at( "nTotEntries" ).get<double>();
          s[1] = j.at( "nTotEntries" ).get<double>();
          s[2] = j.at( "sumx" ).get<double>();
          s[3] = j.at( "sumx2" ).get<double>();
          s[4] = j.at( "sumy" ).get<double>();
          s[5] = j.at( "sumy2" ).get<double>();
          s[6] = j.at( "sumxy" ).get<double>();
          if constexpr ( sizeof...( index ) > 2 ) {
            s[7]  = j.at( "sumz" ).get<double>();
            s[8]  = j.at( "sumz2" ).get<double>();
            s[9]  = j.at( "sumxz" ).get<double>();
            s[10] = j.at( "sumyz" ).get<double>();
          }
          histo.PutStats( s );
        }
      }
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
          if ( axis && jsonAxis[idx].contains( "labels" ) ) {
            const auto labels = jsonAxis[idx].at( "labels" );
            for ( unsigned int i = 0; i < labels.size(); i++ ) {
              axis->SetBinLabel( i + 1, labels[i].template get<std::string>().c_str() );
            }
          }
          if ( axis && jsonAxis[idx].contains( "xbins" ) ) {
            const auto xbins = jsonAxis[idx].at( "xbins" );
            axis->Set( xbins.size() - 1, xbins.template get<std::vector<double>>().data() );
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
      ProfileWrapper( ProfileWrapper const& )             = delete;
      ProfileWrapper& operator=( ProfileWrapper const& )  = delete;
      ProfileWrapper( ProfileWrapper const&& )            = delete;
      ProfileWrapper& operator=( ProfileWrapper const&& ) = delete;
      void            setBinNEntries( Int_t i, Int_t n ) { this->fBinEntries.fArray[i] = n; }
      void            setBinW2( Int_t i, Double_t v ) { this->fSumw2.fArray[i] = v; }
      Double_t        getBinW2( Int_t i ) { return this->fSumw2.fArray[i]; }
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
      bool        isProfile = std::is_base_of_v<TProfile, Histo> || std::is_base_of_v<TProfile2D, Histo>;
      std::string type      = isProfile ? "histogram:ProfileHistogram:double" : "histogram:Histogram:double";
      auto        j         = nlohmann::json{ { "type", type },
                                              { "title", h.GetTitle() },
                                              { "dimension", h.GetDimension() },
                                              { "empty", (int)h.GetEntries() == 0 },
                                              { "nEntries", (int)h.GetEntries() },
                                              { "axis", allAxisTojson( h ) },
                                              { "bins", binsTojson( h ) } };
      if ( !isProfile ) {
        double s[13];
        h.GetStats( s );
        if ( h.GetDimension() == 1 ) {
          j["nTotEntries"] = s[0];
          j["sum"]         = s[2];
          j["sum2"]        = s[3];
          j["mean"]        = s[2] / s[0];
        } else {
          j["nTotEntries"] = s[0];
          j["sumx"]        = s[2];
          j["sumx2"]       = s[3];
          j["meanx"]       = s[2] / s[0];
          j["sumy"]        = s[4];
          j["sumy2"]       = s[5];
          j["sumxy"]       = s[6];
          j["meany"]       = s[4] / s[0];
          if ( h.GetDimension() >= 3 ) {
            j["sumz"]  = s[7];
            j["sumz2"] = s[8];
            j["sumxz"] = s[9];
            j["sumyz"] = s[10];
            j["meanz"] = s[7] / s[0];
          }
        }
      }
      return j;
    }

  } // namespace details

  /**
   * Specialization of Traits dealing with non profile Root Histograms
   */
  template <typename RootHisto, unsigned int N>
  struct Traits<false, RootHisto, N> : details::TraitsBase<RootHisto, N> {
    using Histo      = RootHisto;
    using WeightType = std::vector<double>;
    static void fill( Histo& histo, unsigned int nbins, const WeightType& weight ) {
      for ( unsigned int i = 0; i < nbins; i++ ) { histo.SetBinContent( i, weight[i] ); }
    }
  };

  /**
   * Specialization of Traits dealing with profile Root Histograms
   */
  template <typename RootHisto, unsigned int N>
  struct Traits<true, RootHisto, N> : details::TraitsBase<details::ProfileWrapper<RootHisto>, N> {
    using Histo      = details::ProfileWrapper<RootHisto>;
    using WeightType = std::vector<std::tuple<std::tuple<unsigned int, double>, double>>;
    static constexpr void fill( Histo& histo, unsigned int nbins, const WeightType& weight ) {
      for ( unsigned int i = 0; i < nbins; i++ ) {
        auto [c, sumWeight2]       = weight[i];
        auto [nEntries, sumWeight] = c;
        histo.setBinNEntries( i, nEntries );
        histo.SetBinContent( i, sumWeight );
        histo.setBinW2( i, sumWeight2 );
      }
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

  /// Direct conversion of 1D histograms from Gaudi to ROOT format
  template <Accumulators::atomicity Atomicity = Accumulators::atomicity::full, typename Arithmetic = double>
  details::ProfileWrapper<TProfile> profileHisto1DToRoot( std::string name, Monitoring::Hub::Entity const& ent ) {
    // get original histogram from the Entity
    auto const& gaudiHisto =
        *reinterpret_cast<Gaudi::Accumulators::StaticProfileHistogram<1, Atomicity, Arithmetic>*>( ent.id() );
    // convert to Root
    auto const&                       gaudiAxis = gaudiHisto.template axis<0>();
    details::ProfileWrapper<TProfile> histo{ name.c_str(), gaudiHisto.title().c_str(), gaudiAxis.numBins(),
                                             gaudiAxis.minValue(), gaudiAxis.maxValue() };
    histo.Sumw2();
    unsigned long totNEntries{ 0 };
    for ( unsigned int i = 0; i < gaudiHisto.totNBins(); i++ ) { totNEntries += gaudiHisto.nEntries( i ); }
    if ( !gaudiAxis.labels().empty() ) {
      auto& axis = *histo.GetXaxis();
      for ( unsigned int i = 0; i < gaudiAxis.labels().size(); i++ ) {
        axis.SetBinLabel( i + 1, gaudiAxis.labels()[i].c_str() );
      }
    }
    for ( unsigned int i = 0; i < gaudiHisto.totNBins(); i++ ) {
      auto const& [tmp, sumw2] = gaudiHisto.binValue( i );
      auto const& [nent, sumw] = tmp;
      histo.SetBinContent( i, sumw );
      histo.setBinNEntries( i, nent );
      histo.setBinW2( i, sumw2 );
    }
    histo.SetEntries( totNEntries );
    return histo;
  }

  /// Direct conversion of 2D histograms from Gaudi to ROOT format
  template <Accumulators::atomicity Atomicity = Accumulators::atomicity::full, typename Arithmetic = double>
  details::ProfileWrapper<TProfile2D> profileHisto2DToRoot( std::string name, Monitoring::Hub::Entity const& ent ) {
    // get original histogram from the Entity
    auto const& gaudiHisto =
        *reinterpret_cast<Gaudi::Accumulators::StaticProfileHistogram<2, Atomicity, Arithmetic>*>( ent.id() );
    // convert to Root
    auto const&                         gaudiXAxis = gaudiHisto.template axis<0>();
    auto const&                         gaudiYAxis = gaudiHisto.template axis<1>();
    details::ProfileWrapper<TProfile2D> histo{ name.c_str(),          gaudiHisto.title().c_str(), gaudiXAxis.numBins(),
                                               gaudiXAxis.minValue(), gaudiXAxis.maxValue(),      gaudiYAxis.numBins(),
                                               gaudiYAxis.minValue(), gaudiYAxis.maxValue() };
    histo.Sumw2();
    if ( !gaudiXAxis.labels().empty() ) {
      auto& axis = *histo.GetXaxis();
      for ( unsigned int i = 0; i < gaudiXAxis.labels().size(); i++ ) {
        axis.SetBinLabel( i + 1, gaudiXAxis.labels()[i].c_str() );
      }
    }
    if ( !gaudiYAxis.labels().empty() ) {
      auto& axis = *histo.GetYaxis();
      for ( unsigned int i = 0; i < gaudiYAxis.labels().size(); i++ ) {
        axis.SetBinLabel( i + 1, gaudiYAxis.labels()[i].c_str() );
      }
    }
    for ( unsigned int i = 0; i < gaudiHisto.totNBins(); i++ ) {
      auto const& [tmp, sumw2] = gaudiHisto.binValue( i );
      auto const& [nent, sumw] = tmp;
      histo.SetBinContent( i, sumw );
      histo.setBinNEntries( i, nent );
      histo.setBinW2( i, sumw2 );
    }
    unsigned long totNEntries{ 0 };
    for ( unsigned int i = 0; i < gaudiHisto.totNBins(); i++ ) { totNEntries += gaudiHisto.nEntries( i ); }
    histo.SetEntries( totNEntries );
    return histo;
  }

  /// Direct conversion of 3D histograms from Gaudi to ROOT format
  template <Accumulators::atomicity Atomicity = Accumulators::atomicity::full, typename Arithmetic = double>
  details::ProfileWrapper<TProfile3D> profileHisto3DToRoot( std::string name, Monitoring::Hub::Entity const& ent ) {
    // get original histogram from the Entity
    auto const& gaudiHisto =
        *reinterpret_cast<Gaudi::Accumulators::StaticProfileHistogram<3, Atomicity, Arithmetic>*>( ent.id() );
    // convert to Root
    auto const&                         gaudiXAxis = gaudiHisto.template axis<0>();
    auto const&                         gaudiYAxis = gaudiHisto.template axis<1>();
    auto const&                         gaudiZAxis = gaudiHisto.template axis<2>();
    details::ProfileWrapper<TProfile3D> histo{ name.c_str(),          gaudiHisto.title().c_str(), gaudiXAxis.numBins(),
                                               gaudiXAxis.minValue(), gaudiXAxis.maxValue(),      gaudiYAxis.numBins(),
                                               gaudiYAxis.minValue(), gaudiYAxis.maxValue(),      gaudiZAxis.numBins(),
                                               gaudiZAxis.minValue(), gaudiZAxis.maxValue() };
    histo.Sumw2();
    if ( !gaudiXAxis.labels().empty() ) {
      auto& axis = *histo.GetXaxis();
      for ( unsigned int i = 0; i < gaudiXAxis.labels().size(); i++ ) {
        axis.SetBinLabel( i + 1, gaudiXAxis.labels()[i].c_str() );
      }
    }
    if ( !gaudiYAxis.labels().empty() ) {
      auto& axis = *histo.GetYaxis();
      for ( unsigned int i = 0; i < gaudiYAxis.labels().size(); i++ ) {
        axis.SetBinLabel( i + 1, gaudiYAxis.labels()[i].c_str() );
      }
    }
    if ( !gaudiZAxis.labels().empty() ) {
      auto& axis = *histo.GetZaxis();
      for ( unsigned int i = 0; i < gaudiZAxis.labels().size(); i++ ) {
        axis.SetBinLabel( i + 1, gaudiZAxis.labels()[i].c_str() );
      }
    }
    for ( unsigned int i = 0; i < gaudiHisto.totNBins(); i++ ) {
      auto const& [tmp, sumw2] = gaudiHisto.binValue( i );
      auto const& [nent, sumw] = tmp;
      histo.SetBinContent( i, sumw );
      histo.setBinNEntries( i, nent );
      histo.setBinW2( i, sumw2 );
    }
    unsigned long totNEntries{ 0 };
    for ( unsigned int i = 0; i < gaudiHisto.totNBins(); i++ ) { totNEntries += gaudiHisto.nEntries( i ); }
    histo.SetEntries( totNEntries );
    return histo;
  }

  template <unsigned int N, Accumulators::atomicity Atomicity = Accumulators::atomicity::full,
            typename Arithmetic = double>
  void saveProfileHisto( TFile& file, std::string dir, std::string name, Monitoring::Hub::Entity const& ent ) {
    // fix name and dir if needed
    details::fixNameAndDir( name, dir );
    // Change to the proper directory in the ROOT file
    auto previousDir = details::changeDir( file, dir );
    // write to file
    if constexpr ( N == 1 ) {
      profileHisto1DToRoot<Atomicity, Arithmetic>( name, ent ).Write();
    } else if constexpr ( N == 2 ) {
      profileHisto2DToRoot<Atomicity, Arithmetic>( name, ent ).Write();
    } else if constexpr ( N == 3 ) {
      profileHisto3DToRoot<Atomicity, Arithmetic>( name, ent ).Write();
    }
    // switch back to the previous directory
    previousDir->cd();
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
