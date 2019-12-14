/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
//====================================================================
//	Random Engine definition
//--------------------------------------------------------------------
//
//	Package    : Gaudi/RndmGenSvc ( The LHCb Offline System)
//	Author     : M.Frank
//====================================================================
#ifndef GAUDI_RNDMGENSVC_RNDMENGINE_H
#define GAUDI_RNDMGENSVC_RNDMENGINE_H 1

// STL include files
#include <vector>

// Framework include files
#include "GaudiKernel/IRndmEngine.h"
#include "GaudiKernel/ISerialize.h"
#include "GaudiKernel/Service.h"

/** Random Generator engine definition

    Description:
    Definition of a interface for a generic random number generator
    giving randomly distributed numbers in the range [0...1]
    number generators.

    Dependencies:
    <UL>
    <LI> Generic Service interface: Gaudi/Interfaces/IRndmEngine.h
    </UL>

    <P> History    :
    <PRE>
    +---------+----------------------------------------------+--------+
    |    Date |                 Comment                      | Who    |
    +---------+----------------------------------------------+--------+
    | 21/04/99| Initial version.                             | MF     |
    +---------+----------------------------------------------+--------+
    </PRE>
    Author:  M.Frank
    Version: 1.0
*/
class RndmEngine : public extends<Service, IRndmEngine, ISerialize> {
protected:
  /// Standard Constructor
  using extends::extends;

public:
  /// Service override: initialization
  StatusCode initialize() override;
  /// Single shot returning single random number
  double rndm() const override;
  /** Multiple shots returning vector with flat random numbers.
      @param  array    Array containing random numbers
      @param  howmany  fill 'howmany' random numbers into array
      @param  start    ... starting at position start
      @return StatusCode indicating failure or success.
  */
  StatusCode rndmArray( std::vector<double>& array, long howmany, long start = 0 ) const override;
  /** Input serialization from stream buffer. Restores the status of the object.
      @param   str    Reference to Streambuffer to be filled
      @return  Reference to filled stream buffer
  */
  StreamBuffer& serialize( StreamBuffer& str ) override;
  /** Output serialization to stream buffer. Saves the status of the object.
      @param   str    Reference to Streambuffer to be read
      @return  Reference to read stream buffer
  */
  StreamBuffer& serialize( StreamBuffer& str ) const override;
};

#endif // GAUDI_RNDMGENSVC_RNDMENGINE_H
