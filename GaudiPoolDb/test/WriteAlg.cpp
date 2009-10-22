//	====================================================================
//  WriteAlg.cpp
//	--------------------------------------------------------------------
//
//	Package   : GaudiExamples/Example3
//
//	Author    : Markus Frank
//
//	====================================================================
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

DECLARE_ALGORITHM_FACTORY(WriteAlg);

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
  Event* evt = new Event();
  evt->setEvent(++evtnum);
  evt->setRun(runnum);
  evt->setTime(Gaudi::Time());

  IDataManagerSvc* evtmgr = dynamic_cast<IDataManagerSvc*>(eventSvc().get());
  sc = evtmgr->setRoot("/Event", evt);
  if( sc.isFailure() ) {
    log << MSG::ERROR << "Unable to register /Event object" << endmsg;
    return sc;
  }

  Collision* coll0 = new Collision(0);
  Collision* coll1 = new Collision(1);
  Collision* coll2 = new Collision(2);

  sc = eventSvc()->registerObject("/Event","Collision_0",coll0);
  if( sc.isFailure() ) {
    log << MSG::ERROR << "Unable to register Collision 0" << endmsg;
    return sc;
  }
  sc = eventSvc()->registerObject("/Event","Collision_1",coll1);
  if( sc.isFailure() ) {
    log << MSG::ERROR << "Unable to register Collision 1" << endmsg;
    return sc;
  }
  sc = eventSvc()->registerObject("/Event","Collision_2",coll2);
  if( sc.isFailure() ) {
    log << MSG::ERROR << "Unable to register Collision 2" << endmsg;
    return sc;
  }

  coll0->m_mcTags.push_back(std::pair<int,int>(1,1*1000));
  coll1->m_mcTags.push_back(std::pair<int,int>(1,1*1000));
  coll1->m_mcTags.push_back(std::pair<int,int>(1,1*1000));
  coll2->m_mcTags.push_back(std::pair<int,int>(1,1*1000));
  coll2->m_mcTags.push_back(std::pair<int,int>(1,1*1000));
  coll2->m_mcTags.push_back(std::pair<int,int>(1,1*1000));

  evt->addCollision(coll0);
  evt->addCollision(coll1);
  evt->addCollision(coll2);

  // Create the collection of tracks and register them in the event store
  MyTrackVector* myTracks = new MyTrackVector();
  MyTrack* mother = new MyTrack(rndmgauss(),rndmgauss(),rndmgauss());
  mother->setEvent(evt);
  myTracks->insert ( mother );

  MyVertexVector* myVertices = new MyVertexVector();
  MyVertex* vtx_org = new MyVertex(rndmgauss()/100.0,rndmgauss()/100.0,rndmgauss()/100.0);
  vtx_org->setEvent(evt);
  vtx_org->addCollision(coll0);
  vtx_org->addCollision(coll1);
  vtx_org->addCollision(coll2);
  vtx_org->setMotherParticle(mother);
  myVertices->insert(vtx_org);

  mother->setOriginVertex(vtx_org);

  int n = (int)(rndmflat() * 100.);
  for( int i = 0; i < n; i++ ) {
    // Create new track
    MyTrack* track = new MyTrack(rndmgauss(),rndmgauss(),rndmgauss());
    // set Link to event object
    track->setEvent(evt);
    track->setOriginVertex(vtx_org);
    // And add the stuff to the container
    myTracks->insert ( track );
  }

  // Create vertex container
  int m = (int)(rndmflat() * 100.)+1;
  for( int j = 0; j < m; j++ ) {
    // Create new track
    MyVertex* vtx = new MyVertex(rndmgauss()/100.0,rndmgauss()/100.0,rndmgauss()/100.0);
    // set Link to event object
    vtx->setEvent(evt);
    vtx->addCollision(coll0);
    vtx->addCollision(coll1);
    vtx->addCollision(coll2);
    vtx->setMotherParticle(mother);
    int org = (int)(rndmflat() * float(n));
    vtx->setMotherParticle(*(myTracks->begin()+org));
    // And add the stuff to the container
    myVertices->insert( vtx );
  }
  // Now connect vertices and tracks
  for(MyTrackVector::iterator k1=myTracks->begin(); k1 !=myTracks->end(); ++k1) {
    MyTrack* trk = *k1;
    int org = (int)(rndmflat() * float(m));
    MyVertex* orgVtx = *(myVertices->begin()+org);
    trk->setOriginVertex(orgVtx);
    int dec1 = (int)(rndmflat() * float(m));
    int dec2 = (int)(rndmflat() * float(m));
    int tmp = dec1;
    dec1 = (tmp<dec2) ? tmp : dec2;
    dec2 = (tmp>dec2) ? tmp : dec2;
    for ( int l = dec1; l < dec2; ++l) {
      MyVertex* decVtx = *(myVertices->begin()+l);
      trk->addDecayVertex(decVtx);
      decVtx->setMotherParticle(trk);
    }
  }
  for(MyTrackVector::iterator k2=myTracks->begin(); k2 !=myTracks->end(); ++k2) {
    MyTrack* trk = *k2;
    MyVertex* vtx = const_cast<MyVertex*>(trk->originVertex());
    if ( !vtx )  {
      log << MSG::ERROR << "(1) Invalid origin vertex !" << endmsg;
      continue;
    }
    trk = const_cast<MyTrack*>(vtx->motherParticle());
    if ( !trk )  {
      log << MSG::ERROR << "(2) Invalid mother particle vertex !" << endmsg;
      continue;
    }
    vtx = const_cast<MyVertex*>(trk->originVertex());
    if ( !vtx )  {
      log << MSG::ERROR << "(3) Invalid origin vertex !" << endmsg;
      continue;
    }
  }

  sc = eventSvc()->registerObject("/Event","MyTracks",myTracks);
  if( sc.isFailure() ) {
    log << MSG::ERROR << "Unable to register MyTracks" << endmsg;
    return sc;
  }

  sc = eventSvc()->registerObject("/Event","Collision_0/MyVertices",myVertices);
  if( sc.isFailure() ) {
    log << MSG::ERROR << "Unable to register MyVertices" << endmsg;
    return sc;
  }
  // All done
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------
// Finalize
//--------------------------------------------------------------------
StatusCode WriteAlg::finalize() {
  return StatusCode::SUCCESS;
}
