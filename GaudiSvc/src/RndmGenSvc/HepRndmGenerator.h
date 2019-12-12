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
//	CLHEP Random Generator definition file
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
#ifndef HEPRNDM_HEPRNDMGENERATOR_H
#define HEPRNDM_HEPRNDMGENERATOR_H 1

// Framework include files
#include "RndmGen.h"
#include "RndmGenSvc.h"

#include "HepRndmBaseEngine.h"

namespace HepRndm {

  template <class TYPE>
  class Generator : public RndmGen {
  public:
    typedef TYPE Parameters;

  protected:
    CLHEP::HepRandomEngine* m_hepEngine = nullptr;
    TYPE*                   m_specs     = nullptr;

  public:
    Generator( IInterface* engine ) : RndmGen( engine ) {}
    /// Initialize the generator
    StatusCode initialize( const IRndmGen::Param& par ) override;
    /// Single shot
    double shoot() const override;
  };

  /// Initialize the generator
  template <class TYPE>
  StatusCode Generator<TYPE>::initialize( const IRndmGen::Param& par ) {
    StatusCode status = RndmGen::initialize( par );
    if ( status.isSuccess() ) {
      try {
        m_specs = dynamic_cast<TYPE*>( m_params.get() );
        if ( m_specs ) {
          BaseEngine* engine = dynamic_cast<BaseEngine*>( m_engine.get() );
          if ( engine ) {
            m_hepEngine = engine->hepEngine();
            if ( m_hepEngine ) { return StatusCode::SUCCESS; }
          }
        }
      } catch ( ... ) {}
    }
    return StatusCode::FAILURE;
  }
} // namespace HepRndm

#endif // HEPRNDM_HEPRNDMGENERATOR_H
