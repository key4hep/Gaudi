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
//====================================================================
//	Random RndmGen class definition
//--------------------------------------------------------------------
//
//	Package    : Gaudi/RndmGen ( The LHCb Offline System)
//	Author     : M.Frank
//====================================================================
#pragma once

#include <memory>
// Framework include files
#include <GaudiKernel/IRndmGen.h>
#include <GaudiKernel/SmartIF.h>

// Forward declarations
class IRndmEngine;

/** Random Generator definition

    Description:
    Definition of a interface for a generic random
    number generators.

    Dependencies:
    <UL>
    <LI> Generic Service interface: Gaudi/Interfaces/IInterface.h
    </UL>

    <P> History    :
    <PRE>
    +---------+----------------------------------------------+--------+
    |    Date |                 Comment                      | Who    |
    +---------+----------------------------------------------+--------+
    | 21/11/99| Initial version.                             | MF     |
    +---------+----------------------------------------------+--------+
    </PRE>
    Author:  M.Frank
    Version: 1.0
*/
class RndmGen : public implements<IRndmGen> {

protected:
  /// Generation parameters
  std::unique_ptr<IRndmGen::Param> m_params;
  /// Hosting service: Access must always be possible
  SmartIF<IRndmEngine> m_engine;

  /// Standard Constructor
  RndmGen( IInterface* engine );

public:
  /** IRndmGen implementation    */
  /// Initialize the generator
  StatusCode initialize( const IRndmGen::Param& par ) override;
  /// Finalize the generator
  StatusCode finalize() override;
  /// Random number generator type
  const InterfaceID& type() const override { return ( m_params != 0 ) ? m_params->type() : IID_IRndmFlat; }
  /// Random number generator ID
  long ID() const override { return long( this ); }
  /// Access to random number generator parameters
  const IRndmGen::Param* parameters() const override { return m_params.get(); }
  /** Multiple shots returning vector with random number according to specified distribution.
      @param  array    Array containing random numbers
      @param  howmany  fill 'howmany' random numbers into array
      @param  start    ... starting at position start
  */
  StatusCode shootArray( std::vector<double>& array, long howmany, long start ) const override;
};
