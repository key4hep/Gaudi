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

#include <Gaudi/ParticleID.h>
#include <GaudiKernel/Vector4DTypes.h>
#include <cstdint>

namespace mylee::mc {
  /// Details of a MC particle
  struct particle {
    /// Unique id of the particle in the MC event
    std::uint32_t id;
    /// Id of the vertex this particle originates from
    std::uint32_t origin_vertex_id;
    /// Id of the decay of the particle (-1 for stable particles)
    std::uint32_t decay_vertex_id;
    /// Id of the type of particle (see GaudiPartProp)
    Gaudi::ParticleID part_id;
    /// Momentum of the particle (in MeV)
    Gaudi::LorentzVector momentum;
  };
} // namespace mylee::mc
