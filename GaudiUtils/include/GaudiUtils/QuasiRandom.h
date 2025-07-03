/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

// Include files
#include <string>

// boost
#include <boost/integer/integer_mask.hpp>

/// Create a hash with a large avalanche effect from 32 or 64 bit integers or a string.
///
/// note: the constants below are _not_ arbitrary, but are picked
///       carefully such that the bit shuffling has a large 'avalanche' effect...
///       Code by Brett Mulvey (old link: http://bretm.home.comcast.net/~bretm/hash/)
///
/// note: as a result, you might call this a quasi-random (not to be confused
///       with psuedo-random!) number generator, in that it generates an output
///       which satisfies a requirement on the uniformity of its output distribution.
///       (and not on the predictability of the next number in the sequence,
///       based on knowledge of the preceding numbers)
///
/// note: another way to look at this is is as an (approximation of an) evaporating
///       black hole: whatever you dump in to it, you get something uniformly
///       distributed back ;-)
///
namespace Gaudi {
  namespace Utils {
    namespace QuasiRandom {

      /// Create a hash with a large avalanche effect from a 32 bit integer
      uint32_t mix( uint32_t state );

      /// mix some 'extra' entropy into 'state' and return result
      uint32_t mix32( uint32_t state, uint32_t extra );

      /// mix some 'extra' entropy into 'state' and return result
      uint32_t mix64( uint32_t state, uint64_t extra );

      /// mix some 'extra' entropy into 'state' and return result
      uint32_t mixString( uint32_t state, const std::string& extra );
    } // namespace QuasiRandom
  }   // namespace Utils
} // namespace Gaudi
