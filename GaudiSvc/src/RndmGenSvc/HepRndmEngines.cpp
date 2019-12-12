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
//	Random Engine implementations
//--------------------------------------------------------------------
//
//	Package    : HepRndm ( The LHCb Offline System)
//	Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 29/10/99| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//  Note: Currently all engines require only ONE seed
//        We aill return only the first number, because
//        the implementation in CLHEP does not allow to
//        determine the proper number of seeds used.
#define NUMBER_OF_SEEDS 1
//
//====================================================================
#define HEPRNDM_HEPRNDMENGINES_CPP

// STL include files
#include <cfloat>
#include <cmath>
#include <ctime>
#include <iostream>

// Framework include files
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/System.h"

#include "HepRndmEngine.h"
#include "RndmGenSvc.h"

#include "CLHEP/Random/DRand48Engine.h"
#include "CLHEP/Random/DualRand.h"
#include "CLHEP/Random/Hurd160Engine.h"
#include "CLHEP/Random/Hurd288Engine.h"
#include "CLHEP/Random/JamesRandom.h"
#include "CLHEP/Random/MTwistEngine.h"
#include "CLHEP/Random/RanecuEngine.h"
#include "CLHEP/Random/Ranlux64Engine.h"
#include "CLHEP/Random/RanluxEngine.h"
#include "CLHEP/Random/RanshiEngine.h"
#include "CLHEP/Random/TripleRand.h"

// Handle CLHEP 2.0.x move to CLHEP namespace
namespace CLHEP {}
using namespace CLHEP;

namespace HepRndm {

  // Initialize engine
  template <class TYPE>
  StatusCode Engine<TYPE>::initialize() {
    auto& seeds = m_seeds.value();
    seeds.clear();
    StatusCode status = RndmEngine::initialize();
    if ( m_seeds.size() == 0 ) {
      // Default seeds
      long theSeed = 1234567;
      seeds.push_back( theSeed );
      seeds.push_back( 0 );
    }
    if ( status.isSuccess() ) {
      initEngine();
      info() << "Generator engine type:" << System::typeinfoName( typeid( *hepEngine() ) ) << endmsg;
      if ( m_useTable ) {
        if ( m_row > 214 || m_col > 1 ) {
          error() << "Generator engine seed table has dimension [215][2], you gave:"
                  << " Row=" << m_row << " Column:" << m_col << endmsg;
          status = StatusCode::FAILURE;
        } else {
          info() << "Generator engine seeds from table."
                 << " Row=" << m_row << " Column:" << m_col << endmsg;
        }
      }
      info() << "Current Seed:" << hepEngine()->getSeed();
      info() << " Luxury:" << m_lux.value();
      info() << endmsg;
      // Use the default static engine if required (e.g. for GEANT4)
      if ( m_setSingleton ) {
        HepRandom::setTheEngine( hepEngine() );
        info() << "This is the GEANT4 engine!" << endmsg;
      }
      return status;
    }
    error() << "Cannot initialze random engine of type:" << System::typeinfoName( typeid( TYPE ) ) << endmsg;
    return status;
  }

  // Finalize engine
  template <class TYPE>
  StatusCode Engine<TYPE>::finalize() {
    m_seeds.value().clear();
    return BaseEngine::finalize();
  }

  // Retrieve seeds
  template <class TYPE>
  StatusCode Engine<TYPE>::setSeeds( const std::vector<long>& seed ) {
    auto& seeds = m_seeds.value();
    seeds.clear();
    std::copy( seed.begin(), seed.end(), std::back_inserter( seeds ) );
    if ( !seeds.empty() ) {
      if ( seeds.back() != 0 ) { seeds.push_back( 0 ); }
      hepEngine()->setSeeds( &seeds[0], m_lux );
      return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
  }

  // Retrieve seeds
  template <class TYPE>
  StatusCode Engine<TYPE>::seeds( std::vector<long>& seed ) const {
    /*
    const long *s = hepEngine()->getSeeds();
    for ( size_t i = 0; i < NUMBER_OF_SEEDS; i++ )   {
      seed.push_back(s[i]);
      if ( m_seeds.size() > i )
        m_seeds[i] = s[i];
      else
        m_seeds.push_back(s[i]);
    }
    */
    seed.push_back( hepEngine()->getSeed() );
    return StatusCode::SUCCESS;
  }

  // Specialized create function for DualRand engine
  template <>
  std::unique_ptr<CLHEP::HepRandomEngine> Engine<DualRand>::createEngine() {
    return m_useTable ? std::make_unique<DualRand>( m_row, m_col ) : std::make_unique<DualRand>( m_seeds[0] );
  }
  // Specialized create function for TripleRand engine
  template <>
  std::unique_ptr<CLHEP::HepRandomEngine> Engine<TripleRand>::createEngine() {
    return m_useTable ? std::make_unique<TripleRand>( m_row, m_col ) : std::make_unique<TripleRand>( m_seeds[0] );
  }
  // Specialized create function for DRand48Engine
  template <>
  std::unique_ptr<CLHEP::HepRandomEngine> Engine<DRand48Engine>::createEngine() {
    return m_useTable ? std::make_unique<DRand48Engine>( m_row, m_col ) : std::make_unique<DRand48Engine>( m_seeds[0] );
  }
  // Specialized create function for Hurd160Engine
  template <>
  std::unique_ptr<CLHEP::HepRandomEngine> Engine<Hurd160Engine>::createEngine() {
    return m_useTable ? std::make_unique<Hurd160Engine>( m_row, m_col ) : std::make_unique<Hurd160Engine>( m_seeds[0] );
  }
  // Specialized create function for Hurd288Engine
  template <>
  std::unique_ptr<CLHEP::HepRandomEngine> Engine<Hurd288Engine>::createEngine() {
    return m_useTable ? std::make_unique<Hurd288Engine>( m_row, m_col ) : std::make_unique<Hurd288Engine>( m_seeds[0] );
  }
  // Specialized create function for RanecuEngine
  template <>
  std::unique_ptr<CLHEP::HepRandomEngine> Engine<RanecuEngine>::createEngine() {
    return m_useTable ? std::make_unique<RanecuEngine>( m_row ) : std::make_unique<RanecuEngine>( m_seeds[0] );
  }
  // Specialized create function for RanshiEngine
  template <>
  std::unique_ptr<CLHEP::HepRandomEngine> Engine<RanshiEngine>::createEngine() {
    return m_useTable ? std::make_unique<RanshiEngine>( m_row, m_col ) : std::make_unique<RanshiEngine>( m_seeds[0] );
  }
  // Specialized create function for RanluxEngine
  template <>
  std::unique_ptr<CLHEP::HepRandomEngine> Engine<RanluxEngine>::createEngine() {
    return m_useTable ? std::make_unique<RanluxEngine>( m_row, m_col, m_lux )
                      : std::make_unique<RanluxEngine>( m_seeds[0], m_lux );
  }
  // Specialized create function for Ranlux64Engine
  template <>
  std::unique_ptr<CLHEP::HepRandomEngine> Engine<Ranlux64Engine>::createEngine() {
    return m_useTable ? std::make_unique<Ranlux64Engine>( m_row, m_col, m_lux )
                      : std::make_unique<Ranlux64Engine>( m_seeds[0], m_lux );
  }
  // Specialized create function for MTwistEngine
  template <>
  std::unique_ptr<CLHEP::HepRandomEngine> Engine<MTwistEngine>::createEngine() {
    return m_useTable ? std::make_unique<MTwistEngine>( m_row, m_col ) : std::make_unique<MTwistEngine>( m_seeds[0] );
  }
  // Specialized create function for HepJamesRandom
  template <>
  std::unique_ptr<CLHEP::HepRandomEngine> Engine<HepJamesRandom>::createEngine() {
    return m_useTable ? std::make_unique<HepJamesRandom>( m_row, m_col )
                      : std::make_unique<HepJamesRandom>( m_seeds[0] );
  }
} // namespace HepRndm

typedef HepRndm::Engine<DualRand> e1;
DECLARE_COMPONENT( e1 )
typedef HepRndm::Engine<TripleRand> e2;
DECLARE_COMPONENT( e2 )
typedef HepRndm::Engine<DRand48Engine> e3;
DECLARE_COMPONENT( e3 )
typedef HepRndm::Engine<Hurd160Engine> e4;
DECLARE_COMPONENT( e4 )
typedef HepRndm::Engine<Hurd288Engine> e5;
DECLARE_COMPONENT( e5 )
typedef HepRndm::Engine<HepJamesRandom> e6;
DECLARE_COMPONENT( e6 )
typedef HepRndm::Engine<MTwistEngine> e7;
DECLARE_COMPONENT( e7 )
typedef HepRndm::Engine<RanecuEngine> e8;
DECLARE_COMPONENT( e8 )
typedef HepRndm::Engine<Ranlux64Engine> e9;
DECLARE_COMPONENT( e9 )
typedef HepRndm::Engine<RanluxEngine> e10;
DECLARE_COMPONENT( e10 )
typedef HepRndm::Engine<RanshiEngine> e11;
DECLARE_COMPONENT( e11 )
