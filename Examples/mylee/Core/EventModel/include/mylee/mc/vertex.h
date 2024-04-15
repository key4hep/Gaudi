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
#include <vector>

namespace mylee::mc {
  /// Details of a decay vertex
  struct vertex {
    /// Unique id of the vertex in the event
    std::uint32_t id;
    /// List of particles entering the vertex (unique ids)
    std::vector<std::uint32_t> incoming_particles;
    /// List of particles exiting the vertex (unique ids)
    std::vector<std::uint32_t> outgoing_particles;
    /// Position of the vertex in the global reference frame
    Gaudi::XYZPointF position;
  };
} // namespace mylee::mc
