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
    // allow construction from set of values
    template <class... ARGS>
    LogInputType( ARGS... args ) : std::array<Arithmetic, ND>{ static_cast<Arithmetic>( args )... } {}
    using ValueType          = LogInputType<Arithmetic, ND>;
    using AxisArithmeticType = Arithmetic;
    unsigned int computeIndex( const std::array<Axis<Arithmetic>, ND>& axis ) const {
      unsigned int index = 0;
      for ( unsigned int j = 0; j < ND; j++ ) {
        unsigned int dim = ND - j - 1;
        // compute local index for a given dimension
        int localIndex = axis[dim].index( log( ( *this )[dim] ) );
        // compute global index. Bins are stored in a column first manner
        index *= ( axis[dim].nBins + 2 );
        index += localIndex;
      }
      return index;
    }
    auto forInternalCounter() { return 1ul; }
    template <typename AxisType, long unsigned NAxis>
    static unsigned int computeTotNBins( std::array<AxisType, NAxis> axis ) {
      unsigned int nTotBins = 1;
      for ( unsigned int i = 0; i < NAxis; i++ ) { nTotBins *= ( axis[i].nBins + 2 ); }
      return nTotBins;
    }
  };

  template <atomicity Atomicity, typename Arithmetic, typename ND>
  using LogAccumulator = HistogramingAccumulatorInternal<Atomicity, LogInputType<Arithmetic, ND::value>, unsigned long,
                                                         ND, CountAccumulator>;

  constexpr char logHistogramString[] = "histogram:Histogram";
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using LogHistogram = HistogramingCounterBase<ND, Atomicity, Arithmetic, logHistogramString, LogAccumulator>;

} // namespace Gaudi::Accumulators
