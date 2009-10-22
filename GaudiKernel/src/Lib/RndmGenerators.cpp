// $Id: RndmGenerators.cpp,v 1.7 2006/09/13 15:25:15 hmd Exp $
#define GAUDI_RNDMGENSVC_RNDMGENGENERATORS_CPP

// Framework include files
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/GaudiException.h"

// Standard constructor
Rndm::Numbers::Numbers()
: m_generator(0)
{
}

// Copy constructor
Rndm::Numbers::Numbers(const Rndm::Numbers& copy )
: m_generator(copy.m_generator)   {
  if ( 0 != m_generator )   {
    m_generator->addRef();
  }
}

// Construct and initialize the generator
Rndm::Numbers::Numbers(const SmartIF<IRndmGenSvc>& svc, const IRndmGen::Param& par)
: m_generator(0)
{
  StatusCode status = initialize(svc, par);
  if (!status.isSuccess()) {
    throw GaudiException ("Initialization failed !", "Rndm::Numbers", status);
  }
}

// Standard destructor
Rndm::Numbers::~Numbers()    {
  finalize().ignore();
}

// Initialize the generator
StatusCode Rndm::Numbers::initialize(const SmartIF<IRndmGenSvc>& svc,
                                     const IRndmGen::Param& par)  {
  if ( svc.isValid() && 0 == m_generator )   {
    /// @FIXME: this is a hack, but I do not have the time to review the
    ///         correct constantness of all the methods
    return const_cast<IRndmGenSvc*>(svc.get())->generator( par, m_generator );
  }
  return StatusCode::FAILURE;
}

// Initialize the generator
StatusCode Rndm::Numbers::finalize()   {
  if ( 0 != m_generator )   {
    m_generator->finalize().ignore();
    m_generator->release();
    m_generator = 0;
  }
  return StatusCode::SUCCESS;
}

#if !defined(GAUDI_V22_API) || defined(G22_NEW_SVCLOCATOR)
// Construct and initialize the generator
Rndm::Numbers::Numbers(IRndmGenSvc* svc, const IRndmGen::Param& par)
: m_generator(0)
{
  StatusCode status = initialize(svc, par);
  if (!status.isSuccess()) {
    throw GaudiException ("Initialization failed !", "Rndm::Numbers", status);
  }
}

// Initialize the generator
StatusCode Rndm::Numbers::initialize(IRndmGenSvc* svc, const IRndmGen::Param& par) {
  return initialize(SmartIF<IRndmGenSvc>(svc), par);
}
#endif
