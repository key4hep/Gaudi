/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
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
#include <cmath>
#include <functional>
#include <gsl/span>
#include <nlohmann/json.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

#include <fmt/format.h>

namespace Gaudi::Histograming::Sink::detail {
  inline std::string formatTitle( std::string_view title, unsigned int width ) {
    if ( title.size() > width ) {
      return fmt::format( "\"{:.{}s}...\"", title, width - 3 );
    } else {
      return fmt::format( "\"{:.{}s}\"", title, width );
    }
  }
  inline std::string formatName( std::string_view name, unsigned int width ) {
    if ( name.size() > width ) {
      const auto preSize  = ( width / 2 ) - 2;
      const auto postSize = width - preSize - 3;
      return fmt::format( "{:.{}s}...{:.{}s}", name.substr( 0, preSize ), preSize,
                          name.substr( name.size() - postSize, postSize ), postSize );
    } else {
      return std::string{ name };
    }
  }
  constexpr std::string_view histo1DFormatting =
      " | {:{}.{}s} | {:{}s} | {:10} |{:11.5g} | {:<#11.5g}|{:11.5g} |{:11.5g} |";
  constexpr std::string_view histo2DFormatting =
      " ID={:{}.{}s}  {:{}s}  Ents/All={:>5.0f}/{:<5.0f}<X>/sX={:.5g}/{:<.5g},<Y>/sY={:.5g}/{:<.5g}";
  constexpr std::string_view histo3DFormatting =
      " ID={:{}.{}s}  {:{}s}  "
      "Ents/All={:>5.0f}/{:<5.0f}<X>/sX={:.5g}/{:<.5g},<Y>/sY={:.5g}/{:<.5g},<Z>/sZ={:.5g}/{:<.5g}";

  /// helper struct to print integers with fixed width
  struct IntWithFixedWidth {
    unsigned int value{ 0 };
  };

  /// sqrt or zero
  template <typename T>
  inline T sqrt_or_zero( const T x ) {
    return ( x > 0 ? std::sqrt( x ) : T{ 0 } );
  }

} // namespace Gaudi::Histograming::Sink::detail

/** fmt dedicated formatter for IntWithFixedWidth
 *
 *  supports 2 kinds of formatting string :
 *   - the empty '{}' which uses a width of 10
 *   - '{:n}' where n is an unsigned int, then n is the used width
 *  Using this formatter will result is always respecting the given width
 *  If the number of digits is less than n, the integer is printed using %d,
 *  otherwise it's using scientific notation with n-5 digits. So n has to be
 *  at least 6
 */
template <>
struct fmt::formatter<Gaudi::Histograming::Sink::detail::IntWithFixedWidth> {
  unsigned int width{ 10 };

  constexpr auto parse( format_parse_context& ctx ) {
    auto it = ctx.begin();
    if ( it == ctx.end() || *it == '}' ) { return it; }
    int w = fmt::detail::parse_nonnegative_int( it, ctx.end(), -1 );
    if ( w == -1 ) throw fmt::format_error( "bad" );
    width = (unsigned int)w;
    if ( it == ctx.end() || *it != '}' ) throw fmt::format_error( "bad" );
    return it;
  }

  auto format( Gaudi::Histograming::Sink::detail::IntWithFixedWidth n, format_context& ctx ) const {
    auto s = fmt::format( "{:{}d}", n.value, width );
    if ( s.size() > width ) { s = fmt::format( "{:{}.{}g}", (double)n.value, width, width - 5 ); }
    return fmt::format_to( ctx.out(), "{:s}", s );
  }
};

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
      unsigned int nBins{};
      double       minValue{};
      double       maxValue{};
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
    inline TDirectory* changeDir( TFile& file, std::string_view dir ) {
      // remember the current directory
      auto previousDir = gDirectory;
      // find or create the directory for the histogram
      auto currentDir = ranges::accumulate(
          dir | std::views::transform( []( char c ) { return c == '.' ? '/' : c; } ) | std::views::split( '/' ) |
              std::views::transform( []<std::ranges::input_range R>( R&& r ) {
                return std::string( std::ranges::begin( r ), std::ranges::end( r ) );
              } ),
          file.GetDirectory( "" ), []( TDirectory* current, std::string const& d ) -> TDirectory* {
            if ( !current ) return current;
            if ( auto next = current->GetDirectory( d.c_str() ); next ) return next;
            return current->mkdir( d.c_str() );
          } );
      if ( !currentDir )
        throw GaudiException( fmt::format( "Could not create directory {}", dir ), "Histogram::Sink::Root",
                              StatusCode::FAILURE );
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
    }
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
    unsigned long totNEntries{ 0 };
    for ( unsigned int i = 0; i < gaudiHisto.totNBins(); i++ ) { totNEntries += gaudiHisto.nEntries( i ); }
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

  namespace details {
    template <typename TYPE>
    struct BinAvValue {
    private:
      TYPE m_min{ 0 }, m_max{ 0 }, m_binSize{ 0 };
      bool m_isInt{ false };

    public:
      BinAvValue( const TYPE minV, const TYPE maxV, unsigned int nbins, const bool isInt )
          : m_min( minV ), m_max( maxV ), m_isInt( isInt ) {
        if ( nbins > 0 ) { m_binSize = ( maxV - minV ) / nbins; }
      }
      auto operator()( const unsigned int n ) const {
        TYPE binAv{ 0 };
        if ( m_isInt ) {
          // Need to find all int values that are in range for this bin and form average
          const auto   binMin = m_min + ( n - 1.0 ) * m_binSize;
          const auto   binMax = binMin + m_binSize;
          unsigned int nAv{ 0 };
          for ( auto iv = std::floor( binMin ); iv < std::ceil( binMax ); iv += 1.0 ) {
            if ( binMin <= iv && iv <= binMax ) {
              binAv += iv;
              ++nAv;
            }
          }
          binAv = ( nAv > 0 ? binAv / nAv : 0 );
        } else {
          // For floats just use bin centre
          binAv = m_min + ( n - 0.5 ) * m_binSize;
        }
        return binAv;
      }
    };
  } // namespace details

  template <Gaudi::Accumulators::atomicity Atomicity = Gaudi::Accumulators::atomicity::full,
            typename Arithmetic                      = double>
  std::string printProfileHisto1D( std::string_view name, Gaudi::Monitoring::Hub::Entity const& ent,
                                   unsigned int stringsWidth = 45 ) {
    using std::abs;
    // Type to use for internal calculation. Use double to avoid precision issues across different builds.
    using FPType = double;
    // get original histogram from the Entity
    auto const& gaudiHisto =
        *reinterpret_cast<Gaudi::Accumulators::StaticProfileHistogram<1, Atomicity, Arithmetic>*>( ent.id() );
    // compute min and max values, we actually display the axis limits
    auto const&        gaudiAxisX = gaudiHisto.template axis<0>();
    const FPType       minValueX  = gaudiAxisX.minValue();
    const FPType       maxValueX  = gaudiAxisX.maxValue();
    const unsigned int nBinsX     = gaudiAxisX.numBins();
    // Compute fist and second momenta for normal dimension
    // Compute 1st and 2nd momenta for the profile dimension
    unsigned int              nEntries{ 0 }, nAllEntries{ 0 };
    FPType                    totalSumW{ 0 };
    FPType                    sumX{ 0 }, sum2X{ 0 }, sum3X{ 0 }, sum4X{ 0 };
    const details::BinAvValue xBinAv( minValueX, maxValueX, nBinsX, std::is_integral_v<Arithmetic> );
    for ( unsigned int nx = 0; nx <= nBinsX + 1; ++nx ) {
      auto const& [tmp, sumw2] = gaudiHisto.binValue( nx );
      auto const& [nent, sumw] = tmp;
      nAllEntries += nent;
      if ( nx > 0 && nx <= nBinsX ) {
        const FPType binXValue = xBinAv( nx );
        nEntries += nent;
        const auto sumw_abs = abs( sumw );
        totalSumW += sumw_abs;
        auto val = binXValue * sumw_abs;
        sumX += val;
        val *= binXValue;
        sum2X += val;
        val *= binXValue;
        sum3X += val;
        val *= binXValue;
        sum4X += val;
      }
    }
    const std::string ftitle = detail::formatTitle( gaudiHisto.title(), stringsWidth - 2 );
    if ( nEntries == 0 || !( abs( totalSumW ) > 0 ) ) { return ""; }
    const FPType meanX     = sumX / totalSumW;
    const FPType sigmaX2   = ( sum2X / totalSumW ) - std::pow( meanX, 2 );
    const FPType stddevX   = detail::sqrt_or_zero( sigmaX2 );
    const FPType EX3       = sum3X / totalSumW;
    const FPType A         = sigmaX2 * stddevX;
    const FPType skewnessX = ( abs( A ) > 0.0 ? ( EX3 - ( 3 * sigmaX2 + meanX * meanX ) * meanX ) / A : 0.0 );
    const FPType B         = sigmaX2 * sigmaX2;
    const FPType kurtosisX =
        ( abs( B ) > 0.0 && abs( A ) > 0.0
              ? ( sum4X / totalSumW - meanX * ( 4 * EX3 - meanX * ( 6 * sigmaX2 + 3 * meanX * meanX ) ) ) / B
              : 3.0 );
    // print
    return fmt::format( fmt::runtime( detail::histo1DFormatting ), detail::formatName( name, stringsWidth ),
                        stringsWidth, stringsWidth, ftitle, stringsWidth, detail::IntWithFixedWidth{ nEntries }, meanX,
                        stddevX, skewnessX, kurtosisX - FPType{ 3.0 } );
  }

  template <Gaudi::Accumulators::atomicity Atomicity = Gaudi::Accumulators::atomicity::full,
            typename Arithmetic                      = double>
  std::string printProfileHisto2D( std::string_view name, Gaudi::Monitoring::Hub::Entity const& ent,
                                   unsigned int stringsWidth = 45 ) {
    using std::abs;
    // Type to use for internal calculation. Use double to avoid precision issues across different builds.
    using FPType = double;
    // get original histogram from the Entity
    auto const& gaudiHisto =
        *reinterpret_cast<Gaudi::Accumulators::StaticProfileHistogram<2, Atomicity, Arithmetic>*>( ent.id() );
    // compute min and max values, we actually display the axis limits
    auto const&        gaudiAxisX = gaudiHisto.template axis<0>();
    const FPType       minValueX  = gaudiAxisX.minValue();
    const FPType       maxValueX  = gaudiAxisX.maxValue();
    const unsigned int nBinsX     = gaudiAxisX.numBins();
    auto const&        gaudiAxisY = gaudiHisto.template axis<1>();
    const FPType       minValueY  = gaudiAxisY.minValue();
    const FPType       maxValueY  = gaudiAxisY.maxValue();
    const unsigned int nBinsY     = gaudiAxisY.numBins();
    // Compute fist and second momenta for normal dimenstion
    // Compute 1st and 2nd momenta for the profile dimension
    FPType                    nEntries{ 0 }, nAllEntries{ 0 };
    FPType                    totalSumW{ 0 };
    FPType                    sumX{ 0 }, sumY{ 0 };
    FPType                    sum2X{ 0 }, sum2Y{ 0 };
    const details::BinAvValue xBinAv( minValueX, maxValueX, nBinsX, std::is_integral_v<Arithmetic> );
    const details::BinAvValue yBinAv( minValueY, maxValueY, nBinsY, std::is_integral_v<Arithmetic> );
    for ( unsigned int ny = 0; ny <= nBinsY + 1; ++ny ) {
      const auto offset = ny * ( nBinsX + 2 );
      for ( unsigned int nx = 0; nx <= nBinsX + 1; ++nx ) {
        auto const& [tmp, sumw2] = gaudiHisto.binValue( nx + offset );
        auto const& [nent, sumw] = tmp;
        nAllEntries += nent;
        if ( nx > 0 && ny > 0 && nx <= nBinsX && ny <= nBinsY ) {
          const FPType binXValue = xBinAv( nx );
          const FPType binYValue = yBinAv( ny );
          nEntries += nent;
          const auto sumw_abs = abs( sumw );
          totalSumW += sumw_abs;
          sumX += binXValue * sumw_abs;
          sum2X += binXValue * binXValue * sumw_abs;
          sumY += binYValue * sumw_abs;
          sum2Y += binYValue * binYValue * sumw_abs;
        }
      }
    }
    const FPType meanX = abs( totalSumW ) > 0 ? sumX / totalSumW : 0.0;
    const FPType stddevX =
        abs( totalSumW ) > 0 ? detail::sqrt_or_zero( ( sum2X - sumX * ( sumX / totalSumW ) ) / totalSumW ) : 0.0;
    const FPType meanY = abs( totalSumW ) > 0 ? sumY / totalSumW : 0.0;
    const FPType stddevY =
        abs( totalSumW ) > 0 ? detail::sqrt_or_zero( ( sum2Y - sumY * ( sumY / totalSumW ) ) / totalSumW ) : 0.0;
    // print
    const std::string ftitle = detail::formatTitle( gaudiHisto.title(), stringsWidth - 2 );
    return fmt::format( fmt::runtime( detail::histo2DFormatting ), detail::formatName( name, stringsWidth ),
                        stringsWidth, stringsWidth, ftitle, stringsWidth, nEntries, nAllEntries, meanX, stddevX, meanY,
                        stddevY );
  }

  template <Gaudi::Accumulators::atomicity Atomicity = Gaudi::Accumulators::atomicity::full,
            typename Arithmetic                      = double>
  std::string printProfileHisto3D( std::string_view name, Gaudi::Monitoring::Hub::Entity const& ent,
                                   unsigned int stringsWidth = 45 ) {
    using std::abs;
    // Type to use for internal calculation. Use double to avoid precision issues across different builds.
    using FPType = double;
    // get original histogram from the Entity
    auto const& gaudiHisto =
        *reinterpret_cast<Gaudi::Accumulators::StaticProfileHistogram<3, Atomicity, Arithmetic>*>( ent.id() );
    // compute min and max values, we actually display the axis limits
    auto const&        gaudiAxisX = gaudiHisto.template axis<0>();
    const FPType       minValueX  = gaudiAxisX.minValue();
    const FPType       maxValueX  = gaudiAxisX.maxValue();
    const unsigned int nBinsX     = gaudiAxisX.numBins();
    auto const&        gaudiAxisY = gaudiHisto.template axis<1>();
    const FPType       minValueY  = gaudiAxisY.minValue();
    const FPType       maxValueY  = gaudiAxisY.maxValue();
    const unsigned int nBinsY     = gaudiAxisY.numBins();
    auto const&        gaudiAxisZ = gaudiHisto.template axis<2>();
    const FPType       minValueZ  = gaudiAxisZ.minValue();
    const FPType       maxValueZ  = gaudiAxisZ.maxValue();
    const unsigned int nBinsZ     = gaudiAxisZ.numBins();
    // Compute fist and second momenta for normal dimenstion
    // Compute 1st and 2nd momenta for the profile dimension
    FPType                    nEntries{ 0 }, nAllEntries{ 0 };
    FPType                    totalSumW{};
    FPType                    sumX{ 0 }, sumY{ 0 }, sumZ{ 0 };
    FPType                    sum2X{ 0 }, sum2Y{ 0 }, sum2Z{ 0 };
    const details::BinAvValue xBinAv( minValueX, maxValueX, nBinsX, std::is_integral_v<Arithmetic> );
    const details::BinAvValue yBinAv( minValueY, maxValueY, nBinsY, std::is_integral_v<Arithmetic> );
    const details::BinAvValue zBinAv( minValueZ, maxValueZ, nBinsZ, std::is_integral_v<Arithmetic> );
    for ( unsigned int nz = 0; nz <= nBinsZ + 1; ++nz ) {
      const auto offsetz = nz * ( nBinsY + 2 );
      for ( unsigned int ny = 0; ny <= nBinsY; ++ny ) {
        const auto offset = ( offsetz + ny ) * ( nBinsX + 2 );
        for ( unsigned int nx = 0; nx <= nBinsX; ++nx ) {
          auto const& [tmp, sumw2] = gaudiHisto.binValue( nx + offset );
          auto const& [nent, sumw] = tmp;
          nAllEntries += nent;
          if ( nx > 0 && ny > 0 && nz > 0 && nx <= nBinsX && ny <= nBinsY && nz <= nBinsZ ) {
            const FPType binXValue = xBinAv( nx );
            const FPType binYValue = yBinAv( ny );
            const FPType binZValue = zBinAv( nz );
            nEntries += nent;
            const auto sumw_abs = abs( sumw );
            totalSumW += sumw_abs;
            sumX += binXValue * sumw_abs;
            sum2X += binXValue * binXValue * sumw_abs;
            sumY += binYValue * sumw_abs;
            sum2Y += binYValue * binYValue * sumw_abs;
            sumZ += binZValue * sumw_abs;
            sum2Z += binZValue * binZValue * sumw_abs;
          }
        }
      }
    }
    const FPType meanX = abs( totalSumW ) > 0 ? sumX / totalSumW : 0.0;
    const FPType stddevX =
        abs( totalSumW ) > 0 ? detail::sqrt_or_zero( ( sum2X - sumX * ( sumX / totalSumW ) ) / totalSumW ) : 0.0;
    const FPType meanY = abs( totalSumW ) > 0 ? sumY / totalSumW : 0.0;
    const FPType stddevY =
        abs( totalSumW ) > 0 ? detail::sqrt_or_zero( ( sum2Y - sumY * ( sumY / totalSumW ) ) / totalSumW ) : 0.0;
    const FPType meanZ = abs( totalSumW ) > 0 ? sumZ / totalSumW : 0.0;
    const FPType stddevZ =
        abs( totalSumW ) > 0 ? detail::sqrt_or_zero( ( sum2Z - sumZ * ( sumZ / totalSumW ) ) / totalSumW ) : 0.0;
    // print
    const std::string ftitle = detail::formatTitle( gaudiHisto.title(), stringsWidth - 2 );
    return fmt::format( fmt::runtime( detail::histo3DFormatting ), detail::formatName( name, stringsWidth ),
                        stringsWidth, stringsWidth, ftitle, stringsWidth, nEntries, nAllEntries, meanX, stddevX, meanY,
                        stddevY, meanZ, stddevZ );
  }

  struct BinAccessor {
    std::function<double( unsigned int )> m_get;
    BinAccessor( std::string_view type, const nlohmann::json& j ) {
      auto subtype = std::string_view( type ).substr( 10 ); // remove "histogram:" in front
      if ( subtype.substr( 0, 15 ) == "WeightedProfile" || subtype.substr( 0, 7 ) == "Profile" ) {
        m_get = [bins = j.at( "bins" ).get<std::vector<std::tuple<std::tuple<unsigned int, double>, double>>>()](
                    unsigned int nx ) {
          auto [c, sumWeight2] = bins[nx];
          auto [n, sumWeight]  = c;
          return sumWeight;
        };
      } else {
        m_get = [bins = j.at( "bins" ).get<std::vector<double>>()]( unsigned int nx ) { return bins[nx]; };
      }
    }
    auto operator[]( const unsigned int n ) const {
      using std::abs;
      return abs( m_get( n ) );
    }
  };

  struct ArthTypeAccessor {
    static bool is_integral( const std::string_view type ) {
      for ( const std::string s : { ":i", ":l" } ) {
        if ( 0 == type.compare( type.length() - s.length(), s.length(), s ) ) { return true; }
      }
      return false;
    }
  };

  inline std::string printHistogram1D( std::string_view type, std::string_view name, std::string_view title,
                                       const nlohmann::json& j, unsigned int stringsWidth = 45 ) {
    using std::abs;
    const auto& jaxis       = j.at( "axis" );
    const auto  minValueX   = jaxis[0].at( "minValue" ).get<double>();
    const auto  maxValueX   = jaxis[0].at( "maxValue" ).get<double>();
    const auto  nBinsX      = jaxis[0].at( "nBins" ).get<unsigned int>();
    const auto  nAllEntries = j.at( "nEntries" ).get<unsigned int>();
    // compute the various momenta
    BinAccessor               ba{ type, j };
    double                    sumX{}, sum2X{}, sum3X{}, sum4X{}, nEntries{ 0 };
    const details::BinAvValue xBinAv( minValueX, maxValueX, nBinsX, ArthTypeAccessor::is_integral( type ) );
    for ( unsigned int nx = 1; nx <= nBinsX; ++nx ) {
      const double binXValue = xBinAv( nx );
      const auto   n         = ba[nx];
      nEntries += n;
      auto val = binXValue * n;
      sumX += val;
      val *= binXValue;
      sum2X += val;
      val *= binXValue;
      sum3X += val;
      val *= binXValue;
      sum4X += val;
    }
    const std::string ftitle = detail::formatTitle( title, stringsWidth - 2 );
    if ( !( abs( nEntries ) > 0.0 ) ) { return ""; }
    const double meanX     = sumX / nEntries;
    const double sigmaX2   = ( sum2X / nEntries ) - std::pow( meanX, 2 );
    const double stddevX   = detail::sqrt_or_zero( sigmaX2 );
    const double EX3       = sum3X / nEntries;
    const double A         = sigmaX2 * stddevX;
    const double skewnessX = ( abs( A ) > 0.0 ? ( EX3 - ( 3 * sigmaX2 + meanX * meanX ) * meanX ) / A : 0.0 );
    const double B         = sigmaX2 * sigmaX2;
    const double kurtosisX =
        ( abs( B ) > 0.0 && abs( A ) > 0.0
              ? ( sum4X / nEntries - meanX * ( 4 * EX3 - meanX * ( 6 * sigmaX2 + 3 * meanX * meanX ) ) ) / B
              : 3.0 );
    // print
    return fmt::format( fmt::runtime( detail::histo1DFormatting ), detail::formatName( name, stringsWidth ),
                        stringsWidth, stringsWidth, ftitle, stringsWidth, detail::IntWithFixedWidth{ nAllEntries },
                        meanX, stddevX, skewnessX, kurtosisX - 3.0 );
  }

  inline std::string printHistogram2D( std::string_view type, std::string_view name, std::string_view title,
                                       const nlohmann::json& j, unsigned int stringsWidth = 45 ) {
    using std::abs;
    const auto& jaxis       = j.at( "axis" );
    const auto  minValueX   = jaxis[0].at( "minValue" ).get<double>();
    const auto  maxValueX   = jaxis[0].at( "maxValue" ).get<double>();
    const auto  nBinsX      = jaxis[0].at( "nBins" ).get<unsigned int>();
    const auto  minValueY   = jaxis[1].at( "minValue" ).get<double>();
    const auto  maxValueY   = jaxis[1].at( "maxValue" ).get<double>();
    const auto  nBinsY      = jaxis[1].at( "nBins" ).get<unsigned int>();
    const auto  nAllEntries = j.at( "nEntries" ).get<double>();
    // compute the various memneta
    BinAccessor               ba{ type, j };
    double                    nEntries{};
    double                    sumX{}, sumY{};
    double                    sum2X{}, sum2Y{};
    const auto                isInt = ArthTypeAccessor::is_integral( type );
    const details::BinAvValue xBinAv( minValueX, maxValueX, nBinsX, isInt );
    const details::BinAvValue yBinAv( minValueY, maxValueY, nBinsY, isInt );
    for ( unsigned int ny = 1; ny <= nBinsY; ++ny ) {
      const auto offset = ny * ( nBinsX + 2 );
      for ( unsigned int nx = 1; nx <= nBinsX; ++nx ) {
        const double binXValue = xBinAv( nx );
        const double binYValue = yBinAv( ny );
        const auto   n         = ba[offset + nx];
        nEntries += n;
        sumX += n * binXValue;
        sum2X += n * binXValue * binXValue;
        sumY += n * binYValue;
        sum2Y += n * binYValue * binYValue;
      }
    }
    const double meanX = abs( nEntries ) > 0 ? sumX / nEntries : 0.0;
    const double stddevX =
        abs( nEntries ) > 0 ? detail::sqrt_or_zero( ( sum2X - sumX * ( sumX / nEntries ) ) / nEntries ) : 0.0;
    const double meanY = abs( nEntries ) > 0 ? sumY / nEntries : 0.0;
    const double stddevY =
        abs( nEntries ) > 0 ? detail::sqrt_or_zero( ( sum2Y - sumY * ( sumY / nEntries ) ) / nEntries ) : 0.0;
    // print
    const std::string ftitle = detail::formatTitle( title, stringsWidth - 2 );
    return fmt::format( fmt::runtime( detail::histo2DFormatting ), detail::formatName( name, stringsWidth ),
                        stringsWidth, stringsWidth, ftitle, stringsWidth, nEntries, nAllEntries, meanX, stddevX, meanY,
                        stddevY );
  }

  inline std::string printHistogram3D( std::string_view type, std::string_view name, std::string_view title,
                                       const nlohmann::json& j, unsigned int stringsWidth = 45 ) {
    using std::abs;
    const auto& jaxis       = j.at( "axis" );
    const auto  minValueX   = jaxis[0].at( "minValue" ).get<double>();
    const auto  maxValueX   = jaxis[0].at( "maxValue" ).get<double>();
    const auto  nBinsX      = jaxis[0].at( "nBins" ).get<unsigned int>();
    const auto  minValueY   = jaxis[1].at( "minValue" ).get<double>();
    const auto  maxValueY   = jaxis[1].at( "maxValue" ).get<double>();
    const auto  nBinsY      = jaxis[1].at( "nBins" ).get<unsigned int>();
    const auto  minValueZ   = jaxis[2].at( "minValue" ).get<double>();
    const auto  maxValueZ   = jaxis[2].at( "maxValue" ).get<double>();
    const auto  nBinsZ      = jaxis[2].at( "nBins" ).get<unsigned int>();
    const auto  nAllEntries = j.at( "nEntries" ).get<double>();
    // compute the various memneta
    BinAccessor               ba{ type, j };
    double                    nEntries{};
    double                    sumX{}, sumY{}, sumZ{};
    double                    sum2X{}, sum2Y{}, sum2Z{};
    const auto                isInt = ArthTypeAccessor::is_integral( type );
    const details::BinAvValue xBinAv( minValueX, maxValueX, nBinsX, isInt );
    const details::BinAvValue yBinAv( minValueY, maxValueY, nBinsY, isInt );
    const details::BinAvValue zBinAv( minValueZ, maxValueZ, nBinsZ, isInt );
    for ( unsigned int nz = 1; nz <= nBinsZ; ++nz ) {
      const auto offsetz = nz * ( nBinsY + 2 );
      for ( unsigned int ny = 1; ny <= nBinsY; ++ny ) {
        const auto offset = ( offsetz + ny ) * ( nBinsX + 2 );
        for ( unsigned int nx = 1; nx <= nBinsX; ++nx ) {
          const double binXValue = xBinAv( nx );
          const double binYValue = yBinAv( ny );
          const double binZValue = zBinAv( nz );
          const auto   n         = ba[offset + nx];
          nEntries += n;
          sumX += n * binXValue;
          sum2X += n * binXValue * binXValue;
          sumY += n * binYValue;
          sum2Y += n * binYValue * binYValue;
          sumZ += n * binZValue;
          sum2Z += n * binZValue * binZValue;
        }
      }
    }
    const double meanX = abs( nEntries ) > 0 ? sumX / nEntries : 0.0;
    const double stddevX =
        abs( nEntries ) > 0 ? detail::sqrt_or_zero( ( sum2X - sumX * ( sumX / nEntries ) ) / nEntries ) : 0.0;
    const double meanY = abs( nEntries ) > 0 ? sumY / nEntries : 0.0;
    const double stddevY =
        abs( nEntries ) > 0 ? detail::sqrt_or_zero( ( sum2Y - sumY * ( sumY / nEntries ) ) / nEntries ) : 0.0;
    const double meanZ = abs( nEntries ) > 0 ? sumZ / nEntries : 0.0;
    const double stddevZ =
        abs( nEntries ) > 0 ? detail::sqrt_or_zero( ( sum2Z - sumZ * ( sumZ / nEntries ) ) / nEntries ) : 0.0;
    // print
    const std::string ftitle = detail::formatTitle( title, stringsWidth - 2 );
    return fmt::format( fmt::runtime( detail::histo3DFormatting ), detail::formatName( name, stringsWidth ),
                        stringsWidth, stringsWidth, ftitle, stringsWidth, nEntries, nAllEntries, meanX, stddevX, meanY,
                        stddevY, meanZ, stddevZ );
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
