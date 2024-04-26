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
   * most simple Hit ever : in 2D space, and thus fully defined by x and y
   */
  struct Hit {
    // This default constructor is neede priori to C++20 in order to allow
    // emplace_back for Hits
    Hit( float _x, float _y ) : x( _x ), y( _y ) {}
    float x, y;
  };

  using Hits = std::vector<Hit>;

} // namespace Gaudi::Example::TinyExperiment
