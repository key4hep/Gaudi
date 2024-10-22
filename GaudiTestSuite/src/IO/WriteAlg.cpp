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
//      ====================================================================
//  WriteAlg.cpp
//      --------------------------------------------------------------------
//
//      Package   : GaudiTestSuite/Example3
//
//      Author    : Markus Frank
//
//      ====================================================================
// Framework include files
#include <GaudiKernel/RndmGenerators.h>
#include <GaudiKernel/SmartDataPtr.h>

#include <GaudiKernel/IDataManagerSvc.h>
#include <GaudiKernel/IDataProviderSvc.h>

// Example related include files
#include "WriteAlg.h"

// Event Model related classes
#include <GaudiTestSuite/Counter.h>
#include <GaudiTestSuite/Event.h>
#include <GaudiTestSuite/MyTrack.h>

using namespace Gaudi::TestSuite;

DECLARE_COMPONENT( WriteAlg )

//--------------------------------------------------------------------
// Register data leaf
//--------------------------------------------------------------------
StatusCode WriteAlg::put( IDataProviderSvc* s, const std::string& path, DataObject* pObj ) {
  StatusCode sc = s->registerObject( path, pObj );
  if ( sc.isFailure() ) { error() << "Unable to register object " << path << endmsg; }
  return sc;
}

//--------------------------------------------------------------------
// Initialize
//--------------------------------------------------------------------
StatusCode WriteAlg::initialize() {
  if ( auto sc = Algorithm::initialize(); !sc ) return sc;
  m_recordSvc = service( "FileRecordDataSvc", true );
  if ( !m_recordSvc ) {
    error() << "Unable to retrieve run records service" << endmsg;
    return StatusCode::FAILURE;
  }
  return put( m_recordSvc.get(), "/FileRecords/EvtCount", m_evtCount = new Counter() );
}

//--------------------------------------------------------------------
// Finalize
//--------------------------------------------------------------------
StatusCode WriteAlg::finalize() {
  Counter* pObj = new Counter();
  pObj->set( 123456 );
  auto sc = put( m_recordSvc.get(), "/FileRecords/SumCount", pObj );
  m_recordSvc.reset();
  m_evtCount = nullptr;
  // if any sc is not success, report that one
  if ( auto sc2 = Algorithm::finalize(); sc ) sc = sc2;
  return sc;
}

//--------------------------------------------------------------------
// Execute
//--------------------------------------------------------------------
StatusCode WriteAlg::execute() {
  StatusCode sc;

  static int evtnum = 0;
  static int runnum = 999;

  Rndm::Numbers rndmflat( randSvc(), Rndm::Flat( 0., 1. ) );
  Rndm::Numbers rndmgauss( randSvc(), Rndm::Gauss( 10., 1. ) );

  m_evtCount->increment();

  // Create the Event header and set it as "root" of the event store
  Event* evt = new Event();
  evt->setEvent( ++evtnum );
  evt->setRun( runnum );
  evt->setTime( Gaudi::Time( 1577836800 + evtnum, evtnum * 1e6 ) );

  sc = eventSvc()->registerObject( "/Event", "Header", evt );
  if ( sc.isFailure() ) {
    error() << "Unable to register Event Header" << endmsg;
    return sc;
  }

  Collision* coll0 = new Collision( 0 );
  Collision* coll1 = new Collision( 1 );
  Collision* coll2 = new Collision( 2 );

  sc = eventSvc()->registerObject( "/Event", "Collision_0", coll0 );
  if ( sc.isFailure() ) {
    error() << "Unable to register Collision 0" << endmsg;
    return sc;
  }
  sc = put( eventSvc(), "/Event/Collision_1", coll1 );
  if ( sc.isFailure() ) return sc;
  sc = put( eventSvc(), "/Event/Collision_2", coll2 );
  if ( sc.isFailure() ) return sc;

  evt->addCollision( coll0 );
  evt->addCollision( coll1 );
  evt->addCollision( coll2 );

  // Create the collection of tracks and register them in the event store
  int            n        = (int)( rndmflat() * 100. );
  MyTrackVector* myTracks = new MyTrackVector();
  for ( int i = 0; i < n; i++ ) {
    // Create new track
    double   c     = rndmgauss();
    double   b     = rndmgauss();
    double   a     = rndmgauss();
    MyTrack* track = new MyTrack( float( a ), float( b ), float( c ) );
    // the following line has been replace by the previous one since
    // the order of evaluation of the rndgauss() call is unspecified
    // in the C++ standard. Don't do that.
    // MyTrack* track = new MyTrack(rndmgauss(),rndmgauss(),rndmgauss());

    // set Link to event object
    track->setEvent( evt );
    // And add the stuff to the container
    myTracks->insert( track );
  }

  // Create vertex container
  int             m          = (int)( rndmflat() * 100. ) + 1;
  MyVertexVector* myVertices = new MyVertexVector();
  for ( int j = 0; j < m; j++ ) {
    // Create new track
    double    c   = rndmgauss();
    double    b   = rndmgauss();
    double    a   = rndmgauss();
    MyVertex* vtx = new MyVertex( float( a ) / 100.0F, float( b ) / 100.0F, float( c ) / 100.0F );
    // the following line has been replace by the previous one since
    // the order of evaluation of the rndgauss() call is unspecified
    // in the C++ standard. Don't do that.
    // MyVertex* vtx = new MyVertex(rndmgauss()/100.0,
    //                              rndmgauss()/100.0,
    //                              rndmgauss()/100.0);

    // set Link to event object
    vtx->setEvent( evt );
    vtx->addCollision( coll0 );
    vtx->addCollision( coll1 );
    vtx->addCollision( coll2 );
    // And add the stuff to the container
    myVertices->insert( vtx );
  }
  // Now connect vertices and tracks
  for ( MyTrackVector::iterator k = myTracks->begin(); k != myTracks->end(); ++k ) {
    int       org    = (int)( rndmflat() * double( m ) );
    MyVertex* orgVtx = *( myVertices->begin() + org );
    ( *k )->setOriginVertex( orgVtx );
    int dec1 = (int)( rndmflat() * double( m ) );
    int dec2 = (int)( rndmflat() * double( m ) );
    int tmp  = dec1;
    dec1     = ( tmp < dec2 ) ? tmp : dec2;
    dec2     = ( tmp > dec2 ) ? tmp : dec2;
    for ( int l = dec1; l < dec2; ++l ) {
      MyVertex* decVtx = *( myVertices->begin() + l );
      ( *k )->addDecayVertex( decVtx );
      decVtx->setMotherParticle( *k );
    }
  }

  sc = put( eventSvc(), "/Event/MyTracks", myTracks );
  if ( sc.isFailure() ) return sc;
  sc = put( eventSvc(), "/Event/Collision_0/MyVertices", myVertices );
  if ( sc.isFailure() ) return sc;
  // All done
  return StatusCode::SUCCESS;
}
