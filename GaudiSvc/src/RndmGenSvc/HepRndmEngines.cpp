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
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/System.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IIncidentSvc.h"

#include "RndmGenSvc.h"
#include "HepRndmEngine.h"

#include "CLHEP/Random/Random.h"
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

  // Standard destructor
  template <class TYPE> Engine<TYPE>::~Engine()          {
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
      status = initializeEngine();
      if ( status.isSuccess() )   {
        log << MSG::INFO << "Generator engine type:"
        	  << System::typeinfoName(typeid(TYPE))
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
        log << "Current Seed:" << m_hepEngine->getSeed();
        log << " Luxury:" << m_lux;
        log << endmsg;
        // Use the default static engine if required (e.g. for GEANT4)
        if ( m_setSingleton )   {
            HepRandom::setTheEngine(m_hepEngine);
          log << "This is the GEANT4 engine!" << endmsg;
        }
        return status;
      }
    }
    log << MSG::ERROR << "Cannot initialze random engine of type:"
    	  << System::typeinfoName(typeid(TYPE))
        << endmsg;
    return status;
  }

  // Finalize engine
  template <class TYPE> StatusCode Engine<TYPE>::finalize()          {
    if ( m_hepEngine )   {
      HepRandom::setTheEngine(0);
      delete m_hepEngine;
    }
    m_seeds.clear();
    m_hepEngine = 0;
    return RndmEngine::finalize();
  }

  // Retrieve single random number
  template <class TYPE> double Engine<TYPE>::rndm() const  {
    return m_hepEngine->flat();
  }

  // Retrieve seeds
  template <class TYPE> StatusCode Engine<TYPE>::setSeeds(const std::vector<long>& seed)   {
    typedef std::vector<long> seed_t;
    m_seeds.clear();
    for ( seed_t::const_iterator i = seed.begin(); i < seed.end(); i++ )   {
      m_seeds.push_back(*i);
    }
    if ( m_seeds.size() > 0 )   {
      if ( m_seeds.back() != 0 )    {
        m_seeds.push_back(0);
      }
      m_hepEngine->setSeeds(&m_seeds[0], m_lux);
      return StatusCode::SUCCESS;
    }
    return StatusCode::FAILURE;
  }

  // Retrieve seeds
  template <class TYPE> StatusCode Engine<TYPE>::seeds(std::vector<long>& seed)  const  {
    /*
    const long *s = m_hepEngine->getSeeds();
    for ( size_t i = 0; i < NUMBER_OF_SEEDS; i++ )   {
      seed.push_back(s[i]);
      if ( m_seeds.size() > i )
        m_seeds[i] = s[i];
      else
        m_seeds.push_back(s[i]);
    }
    */
    seed.push_back(m_hepEngine->getSeed());
    return StatusCode::SUCCESS;
  }

  // Specialized shoot function for DualRand engine
  template <> StatusCode Engine<DualRand>::initializeEngine()  {
    m_hepEngine = (m_useTable) ? new DualRand(m_row, m_col) : new DualRand(m_seeds[0]);
    return StatusCode::SUCCESS;
  }
  // Specialized shoot function for TripleRand engine
  template <> StatusCode Engine<TripleRand>::initializeEngine()  {
    m_hepEngine = (m_useTable) ? new TripleRand(m_row, m_col) : new TripleRand(m_seeds[0]);
    return StatusCode::SUCCESS;
  }
  // Specialized shoot function for DRand48Engine
  template <> StatusCode Engine<DRand48Engine>::initializeEngine()  {
    m_hepEngine = (m_useTable) ? new DRand48Engine(m_row, m_col) : new DRand48Engine(m_seeds[0]);
    return StatusCode::SUCCESS;
  }
  // Specialized shoot function for Hurd160Engine
  template <> StatusCode Engine<Hurd160Engine>::initializeEngine()  {
    m_hepEngine = (m_useTable) ? new Hurd160Engine(m_row, m_col) : new Hurd160Engine(m_seeds[0]);
    return StatusCode::SUCCESS;
  }
  // Specialized shoot function for Hurd288Engine
  template <> StatusCode Engine<Hurd288Engine>::initializeEngine()  {
    m_hepEngine = (m_useTable) ? new Hurd288Engine(m_row, m_col) : new Hurd288Engine(m_seeds[0]);
    return StatusCode::SUCCESS;
  }
  // Specialized shoot function for RanecuEngine
  template <> StatusCode Engine<RanecuEngine>::initializeEngine()  {
    m_hepEngine = (m_useTable) ? new RanecuEngine(m_row) : new RanecuEngine(m_seeds[0]);
    return StatusCode::SUCCESS;
  }
  // Specialized shoot function for RanshiEngine
  template <> StatusCode Engine<RanshiEngine>::initializeEngine()  {
    m_hepEngine = (m_useTable) ? new RanshiEngine(m_row, m_col) : new RanshiEngine(m_seeds[0]);
    return StatusCode::SUCCESS;
  }
  // Specialized shoot function for RanluxEngine
  template <> StatusCode Engine<RanluxEngine>::initializeEngine()  {
    m_hepEngine = (m_useTable) ? new RanluxEngine(m_row, m_col, m_lux) : new RanluxEngine(m_seeds[0], m_lux);
    return StatusCode::SUCCESS;
  }
  // Specialized shoot function for Ranlux64Engine
  template <> StatusCode Engine<Ranlux64Engine>::initializeEngine()  {
    m_hepEngine = (m_useTable) ? new Ranlux64Engine(m_row, m_col, m_lux) : new Ranlux64Engine(m_seeds[0], m_lux);
    return StatusCode::SUCCESS;
  }
  // Specialized shoot function for MTwistEngine
  template <> StatusCode Engine<MTwistEngine>::initializeEngine()  {
    m_hepEngine = (m_useTable) ? new MTwistEngine(m_row, m_col) : new MTwistEngine(m_seeds[0]);
    return StatusCode::SUCCESS;
  }
  // Specialized shoot function for HepJamesRandom
  template <> StatusCode Engine<HepJamesRandom>::initializeEngine()  {
    m_hepEngine = (m_useTable) ? new HepJamesRandom(m_row, m_col) : new HepJamesRandom(m_seeds[0]);
    return StatusCode::SUCCESS;
  }

}

typedef HepRndm::Engine<DualRand> e1; DECLARE_SERVICE_FACTORY(e1)
typedef HepRndm::Engine<TripleRand> e2; DECLARE_SERVICE_FACTORY(e2)
typedef HepRndm::Engine<DRand48Engine> e3; DECLARE_SERVICE_FACTORY(e3)
typedef HepRndm::Engine<Hurd160Engine> e4; DECLARE_SERVICE_FACTORY(e4)
typedef HepRndm::Engine<Hurd288Engine> e5; DECLARE_SERVICE_FACTORY(e5)
typedef HepRndm::Engine<HepJamesRandom> e6; DECLARE_SERVICE_FACTORY(e6)
typedef HepRndm::Engine<MTwistEngine> e7; DECLARE_SERVICE_FACTORY(e7)
typedef HepRndm::Engine<RanecuEngine> e8; DECLARE_SERVICE_FACTORY(e8)
typedef HepRndm::Engine<Ranlux64Engine> e9; DECLARE_SERVICE_FACTORY(e9)
typedef HepRndm::Engine<RanluxEngine> e10; DECLARE_SERVICE_FACTORY(e10)
typedef HepRndm::Engine<RanshiEngine> e11; DECLARE_SERVICE_FACTORY(e11)


