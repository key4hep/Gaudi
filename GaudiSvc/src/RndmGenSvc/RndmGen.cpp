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
//	Random RndmGen class implementation
//--------------------------------------------------------------------
//
//	Package    : Gaudi/RndmGen ( The LHCb Offline System)
//	Author     : M.Frank
//  History    :
//  +---------+----------------------------------------------+--------+
//  |    Date |                 Comment                      | Who    |
//  +---------+----------------------------------------------+--------+
//  | 21/11/99| Initial version.                             | MF     |
//  +---------+----------------------------------------------+--------+
//====================================================================
#define GAUDI_RANDOMGENSVC_RNDMGEN_CPP

#include "RndmGen.h"
#include <GaudiKernel/IRndmEngine.h>
#include <GaudiKernel/ISvcLocator.h>
#include <algorithm>
#include <cfloat>

/// Standard Constructor
RndmGen::RndmGen( IInterface* engine ) : m_engine{ engine } {}

/// Initialize the generator
StatusCode RndmGen::initialize( const IRndmGen::Param& par ) {
  m_params.reset( par.clone() );
  return m_engine ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

StatusCode RndmGen::finalize() { return StatusCode::SUCCESS; }

/// Multiple shots returning vector with random number according to specified distribution.
StatusCode RndmGen::shootArray( std::vector<double>& array, long howmany, long start ) const {
  if ( !m_engine ) return StatusCode::FAILURE;
  array.resize( start + howmany );
  std::generate_n( std::next( std::begin( array ), start ), howmany, [&]() { return this->shoot(); } );
  return StatusCode::SUCCESS;
}
