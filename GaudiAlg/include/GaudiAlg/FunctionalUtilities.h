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

#include <Gaudi/Accumulators.h>
#include <Gaudi/Functional/utilities.h>
#include <GaudiAlg/GaudiAlgorithm.h>
#include <GaudiAlg/GaudiHistoAlg.h>

namespace Gaudi::Functional::Traits {
  // this example uses GaudiHistoAlg as base class, and the default handle types for
  // input and output
  using useGaudiHistoAlg = use_<BaseClass_t<GaudiHistoAlg>>;

  // this is the default, but is kept for backward compatibility
  using useAlgorithm = use_<BaseClass_t<Gaudi::Algorithm>>;

  // use legacy GaudiAlgorithm as base class
  using useLegacyGaudiAlgorithm = use_<BaseClass_t<GaudiAlgorithm>>;
} // namespace Gaudi::Functional::Traits
