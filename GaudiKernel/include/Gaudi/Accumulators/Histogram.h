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

#include <Gaudi/Accumulators/AxisAsProperty.h>
#include <Gaudi/Accumulators/HistogramWrapper.h>
#include <Gaudi/Accumulators/StaticHistogram.h>

namespace Gaudi::Accumulators {

  /// standard custom histogram. See HistogramWrapper and StaticHistogram for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using Histogram = HistogramWrapper<StaticHistogram<ND, Atomicity, Arithmetic, AxisTupleType>>;

  /// custom histogram with weight. See HistogramWrapper and StaticWeightedHistogram for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using WeightedHistogram = HistogramWrapper<StaticWeightedHistogram<ND, Atomicity, Arithmetic, AxisTupleType>>;

  /// custom profile histograming. See HistogramWrapper and StaticProfileHistogram for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using ProfileHistogram = HistogramWrapper<StaticProfileHistogram<ND, Atomicity, Arithmetic, AxisTupleType>>;

  /// custom weighted profile histogram. See HistogramWrapper and StaticWeightedProfileHistogram for details
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using WeightedProfileHistogram =
      HistogramWrapper<StaticWeightedProfileHistogram<ND, Atomicity, Arithmetic, AxisTupleType>>;

} // namespace Gaudi::Accumulators
