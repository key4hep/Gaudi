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
//====================================================================
//	Random Engine implementation
//--------------------------------------------------------------------
//
//	Package    : Gaudi/RndmGen ( The LHCb Offline System)
//	Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/99| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define GAUDI_RANDOMGENSVC_RndmEngine_CPP

// STL include files
#include <cfloat>

// Framework include files
#include "RndmEngine.h"
#include <GaudiKernel/DataObject.h>
#include <GaudiKernel/IDataProviderSvc.h>
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/IMessageSvc.h>
#include <GaudiKernel/IOpaqueAddress.h>
#include <GaudiKernel/ISvcLocator.h>
#include <GaudiKernel/MsgStream.h>

/** IRndmEngine interface implementation  */
/// Input serialization from stream buffer. Restores the status of the generator engine.
StreamBuffer& RndmEngine::serialize( StreamBuffer& str ) { return str; }

/// Output serialization to stream buffer. Saves the status of the generator engine.
StreamBuffer& RndmEngine::serialize( StreamBuffer& str ) const { return str; }

/// Single shot returning single random number
double RndmEngine::rndm() const { return DBL_MAX; }

/** Multiple shots returning vector with flat random numbers.
    @param  array    Array containing random numbers
    @param  howmany  fill 'howmany' random numbers into array
    @param  start    ... starting at position start
    @return StatusCode indicating failure or success.
*/
StatusCode RndmEngine::rndmArray( std::vector<double>& array, long howmany, long start ) const {
  array.resize( start + howmany );
  std::generate_n( std::next( array.begin(), start ), howmany, [&]() { return this->rndm(); } );
  return StatusCode::SUCCESS;
}
