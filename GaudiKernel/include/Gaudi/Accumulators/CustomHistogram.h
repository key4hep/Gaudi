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
#include <Gaudi/Accumulators/CustomHistogramWrapper.h>
#include <Gaudi/Accumulators/Histogram.h>

namespace Gaudi::Accumulators {

  /// standard custom histogram
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using CustomHistogram = CustomHistogramWrapper<Histogram<ND, Atomicity, Arithmetic, AxisTupleType>>;

  /// custom histogram with weight
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using CustomWeightedHistogram = CustomHistogramWrapper<WeightedHistogram<ND, Atomicity, Arithmetic, AxisTupleType>>;

  /// custom profile histograming
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using CustomProfileHistogram = CustomHistogramWrapper<ProfileHistogram<ND, Atomicity, Arithmetic, AxisTupleType>>;

  /// custom weighted profile histogram
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using CustomWeightedProfileHistogram =
      CustomHistogramWrapper<WeightedProfileHistogram<ND, Atomicity, Arithmetic, AxisTupleType>>;

} // namespace Gaudi::Accumulators
