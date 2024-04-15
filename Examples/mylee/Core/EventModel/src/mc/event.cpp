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
#include <algorithm>
#include <mylee/mc/event.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

namespace mylee::mc {
  const vertex* event::get_vertex( std::uint32_t vertex_id ) const {
    if ( auto it = std::find_if( begin( vertices ), end( vertices ),
                                 [vertex_id]( const vertex& v ) { return v.id == vertex_id; } );
         it != end( vertices ) ) {
      return &*it;
    } else {
      return nullptr;
    }
  }

  const particle* event::get_particle( std::uint32_t particle_id ) const {
    if ( auto it = std::find_if( begin( particles ), end( particles ),
                                 [particle_id]( const particle& v ) { return v.id == particle_id; } );
         it != end( particles ) ) {
      return &*it;
    } else {
      return nullptr;
    }
  }

  std::vector<std::uint32_t> event::get_primary_vertices() const {
    using namespace ranges;
    return vertices | views::filter( []( const vertex& v ) { return v.incoming_particles.empty(); } ) |
           views::transform( []( const vertex& v ) { return v.id; } ) | to<std::vector>();
  }
} // namespace mylee::mc
