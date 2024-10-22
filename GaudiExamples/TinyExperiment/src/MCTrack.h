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

#include <vector>

namespace Gaudi::Example::TinyExperiment {

  /**
   * most simple MC Track ever : in 2D space, starting from the origin and thus
   * fully defined by an angle theta
   */
  struct MCTrack {
    // This default constructor is neede priori to C++20 in order to allow
    // emplace_back for MCTracks
    MCTrack( float _theta ) : theta( _theta ) {}
    float theta;
  };

  using MCTracks = std::vector<MCTrack>;

} // namespace Gaudi::Example::TinyExperiment
