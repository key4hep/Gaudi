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
// Include files

// local
#include "HistoTimingAlg.h"

// only needed for old style histogram for speed comparison
#include "AIDA/IHistogram1D.h"

//-----------------------------------------------------------------------------
// Implementation file for class : HistoTimingAlg
//
// 2005-08-12 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT( HistoTimingAlg )

//=============================================================================
// Initialization
//=============================================================================
StatusCode HistoTimingAlg::initialize() {
  // must be called first
  const StatusCode sc = GaudiHistoAlg::initialize();
  if ( sc.isFailure() ) return sc;

  // random number generator
  auto randSvc = service<IRndmGenSvc>( "RndmGenSvc", true );
  if ( !randSvc || !m_rand.initialize( randSvc, Rndm::Flat( 0., 1. ) ) ) {
    return Error( "Unable to create Random generator" );
  }

  // book histos
  for ( unsigned int iH = 0; iH < m_nHistos; ++iH ) {
    std::ostringstream title;
    title << "Histogram Number " << iH;
    m_histos[book1D( title.str(), 0, 1, 100 )] = title.str();
  }

  return sc;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode HistoTimingAlg::execute() {

  for ( unsigned int iT = 0; iT < m_nTracks; ++iT ) {

    // fill histos
    for ( HistoMap::iterator iH = m_histos.begin(); iH != m_histos.end(); ++iH ) {
      if ( m_useGaudiAlg ) {
        plot1D( m_rand(), iH->second, 0, 1, 100 );
      } else {
        iH->first->fill( m_rand() );
      }
    }
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
