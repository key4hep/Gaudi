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

#include <GaudiKernel/IInterface.h>
#include <vector>

/** @class IRndmEngine IRndmEngine.h GaudiKernel/IRndmEngine.h

    Definition of a interface for a generic random number generator
    giving randomly distributed numbers in the range [0...1]
    number generators.

    @author  M.Frank
    @version 1.0
*/
class GAUDI_API IRndmEngine : virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID( IRndmEngine, 2, 0 );
  /// Single shot returning single random number
  virtual double rndm() const = 0;
  /** Multiple shots returning vector with flat random numbers.
      @param  array    Array containing random numbers
      @param  howmany  fill 'howmany' random numbers into array
      @param  start    ... starting at position start
      @return StatusCode indicating failure or success.
  */
  virtual StatusCode rndmArray( std::vector<double>& array, long howmany, long start = 0 ) const = 0;
  /// Allow to set new seeds
  virtual StatusCode setSeeds( const std::vector<long>& seeds ) = 0;
  /// Allow to retrieve seeds
  virtual StatusCode seeds( std::vector<long>& seeds ) const = 0;
};
