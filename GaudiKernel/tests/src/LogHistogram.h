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

#include <Gaudi/Accumulators/Histogram.h>

namespace Gaudi::Accumulators {

  /**
   * Example of definition of a Custom Histogram type, based on a different InputType
   *
   * In this example, the input type will simply take a log of the value,
   * basically building a log/log/... histogram
   *
   * The way to achieve this is simply to implement our own InputType for
   * our new histogram. Then use it in the definition of out own accumulator
   * and finally use our Accumulator in out new histogram Type
   *
   * Note how the resulting histograms has support for any arithmetic, any size and any atomicity
   */
  template <typename Arithmetic, unsigned int ND>
  struct LogInputType : std::array<Arithmetic, ND> {
    using ArithmeticType = Arithmetic;
    // allow construction from set of values
    template <class... ARGS>
    LogInputType( ARGS... args ) : std::array<Arithmetic, ND>{ static_cast<Arithmetic>( args )... } {}
    using ValueType = LogInputType<Arithmetic, ND>;
    template <class... AxisType, typename = typename std::enable_if_t<( sizeof...( AxisType ) == ND )>>
    unsigned int computeIndex( std::tuple<AxisType...> const& axis ) const {
      return computeIndexInternal<0, std::tuple<AxisType...>>( axis );
    }
    auto forInternalCounter() { return 1ul; }
    template <class... AxisType, typename = typename std::enable_if_t<( sizeof...( AxisType ) == ND )>>
    static unsigned int computeTotNBins( std::tuple<AxisType...> const& axis ) {
      return computeTotNBinsInternal<0, std::tuple<AxisType...>>( axis );
    }

  private:
    template <int N, class Tuple>
    unsigned int computeIndexInternal( Tuple const& allAxis ) const {
      // compute global index. Bins are stored in a column first manner
      auto const&  axis       = std::get<N>( allAxis );
      unsigned int localIndex = axis.index( log( std::get<N>( *this ) ) );
      if constexpr ( N + 1 == ND )
        return localIndex;
      else
        return localIndex + ( axis.numBins() + 2 ) * computeIndexInternal<N + 1, Tuple>( allAxis );
    }
    template <int N, class Tuple>
    static unsigned int computeTotNBinsInternal( Tuple const& allAxis ) {
      auto const&  axis       = std::get<N>( allAxis );
      unsigned int localNBins = axis.numBins() + 2;
      if constexpr ( N + 1 == ND )
        return localNBins;
      else
        return localNBins * computeTotNBinsInternal<N + 1, Tuple>( allAxis );
    }
  };

  template <atomicity Atomicity, typename Arithmetic, typename ND, typename AxisType>
  using LogAccumulator = HistogramingAccumulatorInternal<Atomicity, LogInputType<Arithmetic, ND::value>, unsigned long,
                                                         CountAccumulator, AxisType>;

  constexpr char logHistogramString[] = "histogram:Histogram";
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using LogHistogram = HistogramingCounterBase<ND, Atomicity, Arithmetic, logHistogramString, LogAccumulator,
                                               make_tuple_t<Axis<Arithmetic>, ND>>;

} // namespace Gaudi::Accumulators
