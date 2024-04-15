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

#include "particle.h"
#include "vertex.h"
#include <cstdint>
#include <optional>
#include <vector>

namespace mylee::mc {
  struct event {
    /// All vertices in the event
    std::vector<vertex> vertices;
    /// All particles in the event
    std::vector<particle> particles;

    /// Get the pointer to the vertex instance by id. Return nullptr if not found
    const vertex* get_vertex( std::uint32_t vertex_id ) const;
    /// Get the pointer to the particle instance by id. Return nullptr if not found
    const particle* get_particle( std::uint32_t particle_id ) const;

    /// Return the list of vertices without incoming particles
    std::vector<std::uint32_t> get_primary_vertices() const;
  };
} // namespace mylee::mc
