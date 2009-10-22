// $Id: WriteAlg.cpp,v 1.6 2007/04/27 14:59:56 hmd Exp $

// Framework include files
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/RndmGenerators.h"

#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IDataManagerSvc.h"

// Example related include files
#include "WriteAlg.h"

// Event Model related classes
#include "Event.h"
#include "MyTrack.h"
#include "MyVertex.h"

DECLARE_ALGORITHM_FACTORY(WriteAlg)

//--------------------------------------------------------------------
// Initialize
//--------------------------------------------------------------------
StatusCode WriteAlg::initialize() {
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------
// Execute
//--------------------------------------------------------------------
StatusCode WriteAlg::execute() {
  StatusCode sc;

  static int evtnum = 0;
  static int runnum = 999;

  MsgStream log(msgSvc(), name());
  Rndm::Numbers rndmflat(randSvc(), Rndm::Flat(0.,1.));
  Rndm::Numbers rndmgauss(randSvc(), Rndm::Gauss(10.,1.));

  // Create the Event header and set it as "root" of the event store
  Event* event = new Event();
  event->setEvent(++evtnum);
  event->setRun(runnum);
  event->setTime(Gaudi::Time());

  SmartIF<IDataManagerSvc> evtmgr(eventSvc());
  sc = evtmgr->setRoot("/Event", event);
  if( sc.isFailure() ) {
    log << MSG::ERROR << "Unable to register /Event object" << endmsg;
    return sc;
  }
  // Create containers
  MyTrackVector* myTracks    = new MyTrackVector();
  MyVertexVector* myVertices = new MyVertexVector();
  // Create the primary trackm and vector
  MyTrack*  ptrack  = new MyTrack((float)rndmgauss(),(float)rndmgauss(),(float)rndmgauss());
  MyVertex* dvertex = new MyVertex(0,0,0);
  ptrack->setDecayVertex(dvertex);
  ptrack->setEvent(event);
  dvertex->setMotherTrack(ptrack);
  myTracks->add( ptrack );
  myVertices->add ( dvertex );
  // loop over first decays...
  int n = (int)(rndmflat() * 100.);
  for( int i = 0; i < n; i++ ) {
    // Create new track
    MyTrack* t = new MyTrack((float)rndmgauss(),(float)rndmgauss(),(float)rndmgauss());
    myTracks->add ( t );
    t->setEvent(event);
    dvertex->addDaughterTrack(t);
    if( rndmflat() > 0.5 ) {
      MyVertex* dv = new MyVertex(rndmflat(),rndmflat(),rndmflat()*10.);
      myVertices->add ( dv );
      dv->setMotherTrack(t);
      int m = (int)(rndmflat() * 10.);
      for( int j = 0; j < m; j++ ) {
        MyTrack* dt = new MyTrack(t->px()/m,t->py()/m,t->pz()/m);
        myTracks->add ( dt );
        dt->setEvent(event);
        dv->addDaughterTrack(dt);
      }
    }
  }

  sc = eventSvc()->registerObject("/Event","MyTracks",myTracks);
  if( sc.isFailure() ) {
    log << MSG::ERROR << "Unable to register MyTracks" << endmsg;
    return sc;
  }
  sc = eventSvc()->registerObject("/Event","MyVertices",myVertices);
  if( sc.isFailure() ) {
    log << MSG::ERROR << "Unable to register MyVertices" << endmsg;
    return sc;
  }

  // All done
  log << MSG::INFO << "Generated event " << evtnum << endmsg;
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------
// Finalize
//--------------------------------------------------------------------
StatusCode WriteAlg::finalize() {
  return StatusCode::SUCCESS;
}
