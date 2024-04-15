/***********************************************************************************\
* (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

#include <GaudiKernel/Point3DTypes.h>
#include <cstdint>

namespace mylee::mc {
  /// Energy deposition of a particle in a detector sensitive volume
  struct hit {
    /// Id of the detector sensitive volume.
    std::uint32_t detector_id;
    /// Position of the hit in the global reference frame
    Gaudi::XYZPointF position;
    /// Energy deposited by the particle (in MeV)
    float energy;
  };
} // namespace mylee::mc
