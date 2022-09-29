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
// Framework include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "GaudiKernel/IDataManagerSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IRndmEngine.h"

// Example related include files
#include "MIWriteAlg.h"

// Event Model related classes
#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"

using namespace Gaudi::Examples::MultiInput;

DECLARE_COMPONENT( WriteAlg )

//--------------------------------------------------------------------
// Initialize
//--------------------------------------------------------------------
StatusCode WriteAlg::initialize() {
  return Algorithm::initialize()
      .andThen( [this]() {
        m_evtnum = 0;
        m_runnum = 999;
      } )
      .andThen( [this]() {
        return m_randomSeeds.empty() ? StatusCode::SUCCESS : randSvc()->engine()->setSeeds( m_randomSeeds );
      } );
}

//--------------------------------------------------------------------
// Execute
//--------------------------------------------------------------------
StatusCode WriteAlg::execute() {
  StatusCode sc;

  Rndm::Numbers rndmflat( randSvc(), Rndm::Flat( 0., 1. ) );
  Rndm::Numbers rndmgauss( randSvc(), Rndm::Gauss( 10., 1. ) );

  // Create the Event header and set it as "root" of the event store
  Event* hdr = new Event();
  hdr->setEvent( ++m_evtnum );
  hdr->setRun( m_runnum );
  hdr->setTime( Gaudi::Time( 1577836800 + m_evtnum, m_evtnum * 1e6 ) );

  sc = eventSvc()->registerObject( "Header", hdr );
  if ( sc.isFailure() ) {
    error() << "Unable to register Event Header" << endmsg;
    return sc;
  }

  // Create the collection of tracks and register them in the event store
  int            n        = (int)( rndmflat() * 100. );
  MyTrackVector* myTracks = new MyTrackVector();
  for ( int i = 0; i < n; ++i ) {
    // Create new track
    // (ensure that the order of the generated random numbers is stable)
    double   c     = rndmgauss();
    double   b     = rndmgauss();
    double   a     = rndmgauss();
    MyTrack* track = new MyTrack( float( a ), float( b ), float( c ) );

    // set Link to header object
    track->setEvent( hdr );

    // And add the stuff to the container
    myTracks->insert( track );
  }

  sc = eventSvc()->registerObject( "Tracks", myTracks );
  if ( sc.isFailure() ) return sc;

  // All done
  return StatusCode::SUCCESS;
}
