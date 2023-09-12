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
#pragma once

#include <Gaudi/Accumulators/Histogram.h>
#include <utility>

namespace Gaudi::Accumulators {

  namespace details {

    /**
     * Default formating for histogram names and title, only calling fmt::format
     * on the text given at construction and passing the histo index as argument
     */
    struct FormatHistDefault {
      std::string_view text;
      FormatHistDefault( std::string_view t ) : text{ t } {}
      std::string operator()( size_t n );
    };

    /**
     * internal class implementing an array of histograms
     * @see HistogramArray
     */
    template <typename Histo, std::size_t N>
    struct HistogramArrayInternal : std::array<Histo, N> {
      /// constructor with callables for FormatName and FormatTitle
      template <typename OWNER, typename FormatName, typename FormatTitle, std::size_t... Ns, typename... Axis,
                typename = typename std::enable_if_t<std::is_invocable_v<FormatName, int>>,
                typename = typename std::enable_if_t<std::is_invocable_v<FormatTitle, int>>>
      HistogramArrayInternal( OWNER* owner, FormatName&& fname, FormatTitle&& ftitle,
                              std::integer_sequence<std::size_t, Ns...>, Axis&&... allAxis )
          : std::array<Histo, N>{ Histo{ owner, fname( Ns ), ftitle( Ns ), allAxis... }... } {
        static_assert( sizeof...( Ns ) < 1000, "Using HistogramArray with 1000 arrays or more is prohibited. This "
                                               "would lead to very long compilation times" );
      }
      /// constructor for strings, FormatHistDefault is used as the default callable
      template <typename OWNER, std::size_t... Ns, typename... Axis>
      HistogramArrayInternal( OWNER* owner, std::string_view name, std::string_view title,
                              std::integer_sequence<std::size_t, Ns...>, Axis&&... allAxis )
          : std::array<Histo, N>{
                Histo{ owner, FormatHistDefault{ name }( Ns ), FormatHistDefault{ title }( Ns ), allAxis... }... } {
        static_assert( sizeof...( Ns ) < 1000, "Using HistogramArray with 1000 arrays or more is prohibited. This "
                                               "would lead to very long compilation times" );
      }
    };
  } // namespace details

  /**
   * generic class implementing an array of histograms
   * The only addition to a raw array is the constructor that allows
   * to build names and titles for the histograms automatically from
   * the index of the histogram in the array
   * There are 2 possibilities :
   *   - if 2 string_views are given, they are used in a call to
   *      std::format(name/title, n);
   *   - if 2 callables are given, they are called on the index
   *     they should take a size_t and return some type convertible to string_view
   * actual implementation is in HistogramArrayInternal
   *
   * Typical usage :
   *   // Array of 5 1D histograms with simple names and titles
   *   // Names will be GaudiH1D-0, GaudiH1D-1, ... and similarly for titles
   *   Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::Histogram<1>, 5> histo1d{
   *     &algo, "GaudiH1D-{}", "A Gaudi 1D histogram - number {}", { 21, -10.5, 10.5, "X" } };
   *   ++histo1d[3][-10.0];
   *   // Array of 5 2D weighted histograms with simple names and titles
   *   // Names will be Name0, Name1, ... and similarly for titles
   *   Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::WeightedHistogram<2>, 7> histo2dw{
   *     &algo, "Name{}", "Title {}", { 21, -10.5, 10.5, "X" }, { 21, -10.5, 10.5, "Y" } };
   *   histo2dw[1][{ -10.0, -10.0 }] += 0.25;
   *   // Array of histograms with custom name and titles
   *   // Names will be GaudiH1D-0-0, GaudiH1D-1-1, GaudiH1D-2-4, ...
   *   // Titles will be "Title 1 of 5", "Title 2 of 5", "Title 3 of 5", ...
   *   Gaudi::Accumulators::HistogramArray<Gaudi::Accumulators::Histogram<1>, 5> histoCustom{
   *     &algo,
   *     []( int n ) { return fmt::format( "GaudiH1D-{}-{}", n, n*n ); },
   *     [nb = 5]( int n ) {
   *       return fmt::format( "Title {} of {}", n+1, nb );
   *     },
   *     { 21, -10.5, 10.5, "X" } };
   *   ++histoCustom[2][-10.0];
   */
  template <typename Histo, std::size_t N, typename Seq>
  struct HistogramArrayBase;
  template <typename Histo, std::size_t N, std::size_t... NDs>
  struct HistogramArrayBase<Histo, N, std::index_sequence<NDs...>> : details::HistogramArrayInternal<Histo, N> {
    template <typename OWNER, typename FormatName, typename FormatTitle>
    HistogramArrayBase( OWNER* owner, FormatName&& fname, FormatTitle&& ftitle,
                        details::alwaysT<NDs, typename Histo::AxisType>&&... allAxis )
        : details::HistogramArrayInternal<Histo, N>( owner, fname, ftitle, std::make_integer_sequence<std::size_t, N>{},
                                                     allAxis... ) {}
  };
  template <typename Histo, std::size_t N>
  using HistogramArray = HistogramArrayBase<Histo, N, std::make_index_sequence<Histo::NumberDimensions::value>>;

} // namespace Gaudi::Accumulators
