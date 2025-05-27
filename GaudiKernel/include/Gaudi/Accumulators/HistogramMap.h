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

#include <fmt/format.h>

#include <map>
#include <mutex>

namespace Gaudi::Accumulators {

  namespace details {

    /**
     * Default formating for histogram names and title, only calling fmt::format
     * on the text given at construction and passing the key as argument
     */
    template <typename Key>
    struct FormatHistDefaultT {
      std::string text;
      FormatHistDefaultT( std::string_view t ) : text{ t } {}
      auto operator()( Key k ) { return fmt::format( fmt::runtime( text ), k ); }
    };

    /**
     * internal class implementing a map of histograms
     *
     * @see HistogramMap
     */
    template <typename Key, typename Histo>
    class HistogramMapInternal {
    public:
      /// constructor with callables for FormatName and FormatTitle
      template <typename OWNER, std::invocable<Key const&> FormatName, std::invocable<Key const&> FormatTitle>
      HistogramMapInternal( OWNER* owner, FormatName&& fname, FormatTitle&& ftitle,
                            typename Histo::AxisTupleType&& allAxis )
          : m_makeHisto{ [owner, fname, ftitle, allAxis, this]( Key const& k ) -> Histo& {
            return m_map
                .emplace( std::piecewise_construct, std::forward_as_tuple( k ),
                          std::forward_as_tuple( owner, fname( k ), ftitle( k ), allAxis ) )
                .first->second;
          } } {}
      /// constructor for strings, FormatHistDefaultT is used as the default callable
      template <typename OWNER>
      HistogramMapInternal( OWNER* owner, std::string_view name, std::string_view title,
                            typename Histo::AxisTupleType&& allAxis )
          : m_makeHisto{ [owner, name, title, allAxis, this]( Key const& k ) -> Histo& {
            return m_map
                .emplace( std::piecewise_construct, std::forward_as_tuple( k ),
                          std::forward_as_tuple( owner, FormatHistDefaultT<Key>( name )( k ),
                                                 FormatHistDefaultT<Key>( title )( k ), allAxis ) )
                .first->second;
          } } {}
      /// operator[] method, made thread safe and thus declared const
      Histo& operator[]( Key const& k ) const {
        std::scoped_lock lock{ m_mapLock };
        auto             it = m_map.find( k );
        if ( it != m_map.end() ) return it->second;
        return m_makeHisto( k );
      }

    private:
      mutable std::map<Key, Histo>        m_map;
      mutable std::mutex                  m_mapLock{};
      std::function<Histo&( Key const& )> m_makeHisto;
      // operator[],
    };
  } // namespace details

  /**
   * generic class implementing a thread safe map of histograms
   *
   * The map looks like and std:map with only operator[] and constructor
   * The constructor takes 2 arguments allowing to build names and titles automatically
   * when inserting new histograms in the map.
   * There are 2 possibilities :
   *   - if 2 string_views are given, they are used in a call to
   *      std::format(name/title, key);
   *   - if 2 callables are given, they are called on the key
   *     they should take an argument of type Key and return some type convertible to string_view
   * The operator[] will then build on the fly the name and title of the new histograms from
   * the key when creating new entries
   *
   * Thread safety is ensured by serializing calls to operator[] through a mutex
   *
   * Note that this should in principle be used only with StaticHistograms, there is no
   * reason to use Histograms there, as the histograms are only created at run time anyway
   * so in a sense they are already dynamic
   *
   * Typical usage :
   *   // Map of 1D histograms with simple names and titles, key being an int
   *   // Names will be GaudiH1D-N and similarly for titles where N is the key associated
   *   Gaudi::Accumulators::HistogramMap<int, Gaudi::Accumulators::Histogram<1>> histo1d{
   *     &algo, "GaudiH1D-{}", "A Gaudi 1D histogram - number {}", { 21, -10.5, 10.5, "X" } };
   *   ++histo1d[3][-10.0];
   *   // Map of 2D weighted histograms with simple names and titles, key being int
   *   // Names will be Name0, Name1, ... and similarly for titles
   *   Gaudi::Accumulators::HistogramMap<int, Gaudi::Accumulators::WeightedHistogram<2>> histo2dw{
   *     &algo, "Name{}", "Title {}", { 21, -10.5, 10.5, "X" }, { 21, -10.5, 10.5, "Y" } };
   *   histo2dw[1][{ -10.0, -10.0 }] += 0.25;
   *   // Map of histograms with custom name and titles, key is a pair<int, string>
   *   // Names will be GaudiH1D-<N>-<S>, where the key was the pair (<n>, <S>)
   *   // Titles will be "Title <S> (<N>)"
   *   Gaudi::Accumulators::HistogramMap<std::pair<int, std::string>, Gaudi::Accumulators::Histogram<1>> histoCustom{
   *     &algo,
   *     []( std::pair<int, std::string> const& p ) { return fmt::format( "GaudiH1D-{}-{}", p.first, p.second ); },
   *     []( std::pair<int, std::string> const& p ) {
   *       return fmt::format( "Title {} ({})", p.first, p.second );
   *     },
   *     { 21, -10.5, 10.5, "X" } };
   *   ++histoCustom[{3, "three"}][-10.0];
   */
  template <typename Key, typename Histo,
            typename Seq = std::make_integer_sequence<unsigned int, std::tuple_size_v<typename Histo::AxisTupleType>>>
  struct HistogramMap;
  template <typename Key, typename Histo, unsigned int... ND>
  struct HistogramMap<Key, Histo, std::integer_sequence<unsigned int, ND...>>
      : details::HistogramMapInternal<Key, Histo> {
    template <typename OWNER, typename FormatName, typename FormatTitle>
    HistogramMap( OWNER* owner, FormatName&& fname, FormatTitle&& ftitle, typename Histo::AxisTupleType&& allAxis )
        : details::HistogramMapInternal<Key, Histo>( owner, fname, ftitle,
                                                     std::forward<typename Histo::AxisTupleType>( allAxis ) ) {}

    template <unsigned int I>
    using AxisType = std::tuple_element_t<I, typename Histo::AxisTupleType>;

    template <typename OWNER, typename FormatName, typename FormatTitle>
    HistogramMap( OWNER* owner, FormatName&& fname, FormatTitle&& ftitle, AxisType<ND>... allAxis )
        : HistogramMap( owner, fname, ftitle, std::make_tuple( allAxis... ) ) {}
  };

} // namespace Gaudi::Accumulators
