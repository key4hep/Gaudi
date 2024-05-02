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
#include <Gaudi/Accumulators/RootHistogram.h>

namespace Gaudi::Accumulators {

  /// standard custom histogram
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double,
            typename AxisTupleType = make_tuple_t<Axis<Arithmetic>, ND>>
  using CustomRootHistogram = CustomHistogramWrapper<RootHistogram<ND, Atomicity, Arithmetic, AxisTupleType>>;

} // namespace Gaudi::Accumulators
