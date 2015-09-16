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
#define NUMBER_OF_SEEDS    1
//
//====================================================================
#define HEPRNDM_HEPRNDMENGINES_CPP

// STL include files
#include <iostream>
#include <cfloat>
#include <ctime>
#include <cmath>

// Framework include files
#include "GaudiKernel/System.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IIncidentSvc.h"

#include "RndmGenSvc.h"
#include "HepRndmEngine.h"

#include "CLHEP/Random/DualRand.h"
#include "CLHEP/Random/TripleRand.h"
#include "CLHEP/Random/DRand48Engine.h"
#include "CLHEP/Random/Hurd160Engine.h"
#include "CLHEP/Random/Hurd288Engine.h"
#include "CLHEP/Random/JamesRandom.h"
#include "CLHEP/Random/MTwistEngine.h"
#include "CLHEP/Random/RanecuEngine.h"
#include "CLHEP/Random/Ranlux64Engine.h"
#include "CLHEP/Random/RanluxEngine.h"
#include "CLHEP/Random/RanshiEngine.h"

// Handle CLHEP 2.0.x move to CLHEP namespace
namespace CLHEP { }
using namespace CLHEP;

namespace HepRndm  {

  // Standard constructor
  template <class TYPE> Engine<TYPE>::Engine(const std::string& nam, ISvcLocator* loc)
  : BaseEngine (nam, loc)    {
    declareProperty("Seeds",       m_seeds);
    declareProperty("Column",      m_col = 0);
    declareProperty("Row",         m_row = 1);
    declareProperty("Luxury",      m_lux = 3);
    declareProperty("UseTable",    m_useTable     = false);
    declareProperty("SetSingleton",m_setSingleton = false);
  }

  // Initialize engine
  template <class TYPE> StatusCode Engine<TYPE>::initialize()          {
    m_seeds.erase(m_seeds.begin(), m_seeds.end());
    StatusCode status = RndmEngine::initialize();
    if ( m_seeds.size() == 0 )  {
      // Default seeds
      long theSeed = 1234567;
      m_seeds.push_back(theSeed);
      m_seeds.push_back(0);
    }
    MsgStream log(msgSvc(), name());
    if ( status.isSuccess() )   {
      initEngine();
      log << MSG::INFO << "Generator engine type:"
      	  << System::typeinfoName(typeid(*hepEngine()))
          << endmsg;
      if ( m_useTable )   {
        if ( m_row > 214 || m_col > 1 )   {
          log << MSG::ERROR << "Generator engine seed table has dimension [215][2], you gave:"
              << " Row=" << m_row << " Column:" << m_col << endmsg;
          status = StatusCode::FAILURE;
        }
        else    {
          log << MSG::INFO << "Generator engine seeds from table."
              << " Row=" << m_row << " Column:" << m_col << endmsg;
        }
      }
      log << "Current Seed:" << hepEngine()->getSeed();
      log << " Luxury:" << m_lux;
      log << endmsg;
      // Use the default static engine if required (e.g. for GEANT4)
      if ( m_setSingleton )   {
          HepRandom::setTheEngine(hepEngine());
        log << "This is the GEANT4 engine!" << endmsg;
      }
      return status;
    }
    log << MSG::ERROR << "Cannot initialze random engine of type:"
    	  << System::typeinfoName(typeid(TYPE))
        << endmsg;
    return status;
  }

  // Finalize engine
  template <class TYPE> StatusCode Engine<TYPE>::finalize()          {
    m_seeds.clear();
    return BaseEngine::finalize();
  }

  // Retrieve seeds
  template <class TYPE> StatusCode Engine<TYPE>::setSeeds(const std::vector<long>& seed)   {
    m_seeds.clear();
    std::copy(seed.begin(),seed.end(),std::back_inserter(m_seeds));
    if ( !m_seeds.empty() )   {
      if ( m_seeds.back() != 0 )    {
        m_seeds.push_back(0);
      }
      hepEngine()->setSeeds(&m_seeds[0], m_lux);
      return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
  }

  // Retrieve seeds
  template <class TYPE> StatusCode Engine<TYPE>::seeds(std::vector<long>& seed)  const  {
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
    seed.push_back(hepEngine()->getSeed());
    return StatusCode::SUCCESS;
  }

  // Specialized create function for DualRand engine
  template <> std::unique_ptr<CLHEP::HepRandomEngine> Engine<DualRand>::createEngine()  {
    return std::unique_ptr<CLHEP::HepRandomEngine>( m_useTable ? new DualRand(m_row, m_col) 
                                                               : new DualRand(m_seeds[0]) );
  }
  // Specialized create function for TripleRand engine
  template <> std::unique_ptr<CLHEP::HepRandomEngine> Engine<TripleRand>::createEngine()  {
    return std::unique_ptr<CLHEP::HepRandomEngine>( m_useTable ? new TripleRand(m_row, m_col) 
                                                               : new TripleRand(m_seeds[0]));
  }
  // Specialized create function for DRand48Engine
  template <> std::unique_ptr<CLHEP::HepRandomEngine> Engine<DRand48Engine>::createEngine()  {
    return std::unique_ptr<CLHEP::HepRandomEngine>( m_useTable ? new DRand48Engine(m_row, m_col) 
                                                               : new DRand48Engine(m_seeds[0]));
  }
  // Specialized create function for Hurd160Engine
  template <> std::unique_ptr<CLHEP::HepRandomEngine> Engine<Hurd160Engine>::createEngine()  {
    return std::unique_ptr<CLHEP::HepRandomEngine>( m_useTable ? new Hurd160Engine(m_row, m_col) 
                                                               : new Hurd160Engine(m_seeds[0]));
  }
  // Specialized create function for Hurd288Engine
  template <> std::unique_ptr<CLHEP::HepRandomEngine> Engine<Hurd288Engine>::createEngine()  {
    return std::unique_ptr<CLHEP::HepRandomEngine>( m_useTable ? new Hurd288Engine(m_row, m_col) 
                                                               : new Hurd288Engine(m_seeds[0]));
  }
  // Specialized create function for RanecuEngine
  template <> std::unique_ptr<CLHEP::HepRandomEngine> Engine<RanecuEngine>::createEngine()  {
    return std::unique_ptr<CLHEP::HepRandomEngine>( m_useTable ? new RanecuEngine(m_row) 
                                                               : new RanecuEngine(m_seeds[0]));
  }
  // Specialized create function for RanshiEngine
  template <> std::unique_ptr<CLHEP::HepRandomEngine> Engine<RanshiEngine>::createEngine()  {
    return std::unique_ptr<CLHEP::HepRandomEngine>( m_useTable ? new RanshiEngine(m_row, m_col) 
                                                               : new RanshiEngine(m_seeds[0]));
  }
  // Specialized create function for RanluxEngine
  template <> std::unique_ptr<CLHEP::HepRandomEngine> Engine<RanluxEngine>::createEngine()  {
    return std::unique_ptr<CLHEP::HepRandomEngine>( m_useTable ? new RanluxEngine(m_row, m_col, m_lux) 
                                                               : new RanluxEngine(m_seeds[0], m_lux));
  }
  // Specialized create function for Ranlux64Engine
  template <> std::unique_ptr<CLHEP::HepRandomEngine> Engine<Ranlux64Engine>::createEngine()  {
    return std::unique_ptr<CLHEP::HepRandomEngine>( m_useTable ? new Ranlux64Engine(m_row, m_col, m_lux) 
                                                               : new Ranlux64Engine(m_seeds[0], m_lux));
  }
  // Specialized create function for MTwistEngine
  template <> std::unique_ptr<CLHEP::HepRandomEngine> Engine<MTwistEngine>::createEngine()  {
    return std::unique_ptr<CLHEP::HepRandomEngine>( m_useTable ? new MTwistEngine(m_row, m_col) 
                                                               : new MTwistEngine(m_seeds[0]));
  }
  // Specialized create function for HepJamesRandom
  template <> std::unique_ptr<CLHEP::HepRandomEngine> Engine<HepJamesRandom>::createEngine()  {
    return std::unique_ptr<CLHEP::HepRandomEngine>( m_useTable ? new HepJamesRandom(m_row, m_col) 
                                                               : new HepJamesRandom(m_seeds[0]) );
  }

}

typedef HepRndm::Engine<DualRand> e1; DECLARE_COMPONENT(e1)
typedef HepRndm::Engine<TripleRand> e2; DECLARE_COMPONENT(e2)
typedef HepRndm::Engine<DRand48Engine> e3; DECLARE_COMPONENT(e3)
typedef HepRndm::Engine<Hurd160Engine> e4; DECLARE_COMPONENT(e4)
typedef HepRndm::Engine<Hurd288Engine> e5; DECLARE_COMPONENT(e5)
typedef HepRndm::Engine<HepJamesRandom> e6; DECLARE_COMPONENT(e6)
typedef HepRndm::Engine<MTwistEngine> e7; DECLARE_COMPONENT(e7)
typedef HepRndm::Engine<RanecuEngine> e8; DECLARE_COMPONENT(e8)
typedef HepRndm::Engine<Ranlux64Engine> e9; DECLARE_COMPONENT(e9)
typedef HepRndm::Engine<RanluxEngine> e10; DECLARE_COMPONENT(e10)
typedef HepRndm::Engine<RanshiEngine> e11; DECLARE_COMPONENT(e11)
