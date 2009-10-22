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

#include <cfloat>
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IRndmEngine.h"
#include "RndmGen.h"

/// Standard Constructor
RndmGen::RndmGen(IInterface* engine) : m_params(0), m_engine(0)   {
  if ( 0 != engine )  {
    engine->queryInterface(IRndmEngine::interfaceID(), pp_cast<void>(&m_engine)).ignore();
  }
}

/// Standard Destructor
RndmGen::~RndmGen()   {
  if ( m_engine ) m_engine->release();
  m_engine = 0;
  if ( m_params ) delete m_params;
  m_params = 0;
}

/// Initialize the generator
StatusCode RndmGen::initialize(const IRndmGen::Param& par)   {
  m_params = par.clone();
  return (0==m_engine) ? StatusCode::FAILURE : StatusCode::SUCCESS;
}

/// Initialize the RndmGen
StatusCode RndmGen::finalize()   {
  return StatusCode::SUCCESS;
}

/// Single shot returning single random number according to specified distribution
double RndmGen::shoot()  const  {
  return DBL_MAX;
}

/// Multiple shots returning vector with random number according to specified distribution.
StatusCode RndmGen::shootArray( std::vector<double>& array, long howmany, long start) const {
  if ( 0 != m_engine )    {
    long cnt = start;
    array.resize(start+howmany);
    for ( long i = start, num = start+howmany; i < num; i++ )     {
      array[cnt++] = shoot();
    }
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

