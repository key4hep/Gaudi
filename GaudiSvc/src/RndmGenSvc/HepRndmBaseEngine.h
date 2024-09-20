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
//	CLHEP Basic Random Engine definition file
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
//====================================================================
#ifndef HEPRNDM_HEPRNDMBASEENGINE_H
#define HEPRNDM_HEPRNDMBASEENGINE_H 1

// Framework include files
#include "RndmEngine.h"
#include <CLHEP/Random/Random.h>

// Forward declarations
namespace CLHEP {
  class HepRandomEngine;
}

namespace HepRndm {

  class BaseEngine : public RndmEngine {
  protected:
    std::unique_ptr<CLHEP::HepRandomEngine> m_hepEngine;

  public:
    BaseEngine( const std::string& name, ISvcLocator* loc ) : RndmEngine( name, loc ) {}
    CLHEP::HepRandomEngine*       hepEngine() { return m_hepEngine.get(); }
    const CLHEP::HepRandomEngine* hepEngine() const { return m_hepEngine.get(); }
    // Retrieve single random number
    double rndm() const override { return m_hepEngine->flat(); }

    StatusCode finalize() override {
      if ( m_hepEngine ) CLHEP::HepRandom::setTheEngine( nullptr );
      m_hepEngine.reset();
      return RndmEngine::finalize();
    }

  protected:
    void                                            initEngine() { m_hepEngine = createEngine(); }
    virtual std::unique_ptr<CLHEP::HepRandomEngine> createEngine() = 0;
  };
} // namespace HepRndm
#endif // HEPRNDM_HEPRNDMBASEENGINE_H
