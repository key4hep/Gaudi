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
//	Random Generator service definition
//--------------------------------------------------------------------
//
//	Package    : Gaudi/RndmGen ( The LHCb Offline System)
//	Author     : M.Frank
//====================================================================
#ifndef GAUDI_RANDOMGENSVC_RNDMGENSVC_H
#define GAUDI_RANDOMGENSVC_RNDMGENSVC_H 1

// STL include files
#include <vector>

// Framework include files
#include "GaudiKernel/IRndmEngine.h"
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/ISerialize.h"
#include "GaudiKernel/Service.h"

// Forward declarations
class IRndmGenFactory;
class IMessageSvc;

/** Random Generator service definition

    Description:
    Definition of a interface for a service to access
    random generators according to predefined distributions.
    For more detailed explanations please see the interface definition.

    Dependencies:
    <UL>
    <LI> Base class: Gaudi/RndmGenSvc/ISerialize.h
    <LI> Base class: Gaudi/RndmGenSvc/IRandomGenSvc.h
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
class RndmGenSvc : public extends<Service, IRndmGenSvc, IRndmEngine, ISerialize> {
private:
  /// Random number engine
  mutable SmartIF<IRndmEngine> m_engine;
  /// Serialization interface of random number engine
  mutable SmartIF<ISerialize> m_serialize;

  Gaudi::Property<std::string> m_engineName{ this, "Engine", "HepRndm::Engine<CLHEP::RanluxEngine>", "engine name" };

public:
  // inherits constructor from base class
  using extends::extends;

public:
  /// Service override: initialization
  StatusCode initialize() override;
  /// Service override: finalization
  StatusCode finalize() override;
  /** IRndmGenSvc interface implementation  */
  /// Input serialization from stream buffer. Restores the status of the generator engine.
  StreamBuffer& serialize( StreamBuffer& str ) override;
  /// Output serialization to stream buffer. Saves the status of the generator engine.
  StreamBuffer& serialize( StreamBuffer& str ) const override;
  /// Retrieve engine
  IRndmEngine* engine() override;
  /// Retrieve a valid generator from the service.
  StatusCode generator( const IRndmGen::Param& par, IRndmGen*& refpGen ) override;
  /// Single shot returning single random number
  double rndm() const override;
  /** Multiple shots returning vector with flat random numbers.
      @param  array    Array containing random numbers
      @param  howmany  fill 'howmany' random numbers into array
      @param  start    ... starting at position start
      @return StatusCode indicating failure or success.
  */
  StatusCode rndmArray( std::vector<double>& array, long howmany, long start = 0 ) const override;
  /// Allow to set new seeds
  StatusCode setSeeds( const std::vector<long>& seeds ) override;
  /// Allow to get seeds
  StatusCode seeds( std::vector<long>& seeds ) const override;
};

#endif // GAUDI_RANDOMGENSVC_RNDMGENSVC_H
