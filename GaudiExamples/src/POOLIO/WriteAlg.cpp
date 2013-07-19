//      ====================================================================
//  WriteAlg.cpp
//      --------------------------------------------------------------------
//
//      Package   : GaudiExamples/Example3
//
//      Author    : Markus Frank
//
//      ====================================================================
// Framework include files
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
#include "Counter.h"

using namespace Gaudi::Examples;

DECLARE_COMPONENT(WriteAlg)

//--------------------------------------------------------------------
// Register data leaf
//--------------------------------------------------------------------
StatusCode WriteAlg::put(IDataProviderSvc* s, const std::string& path, DataObject* pObj) {
  StatusCode sc = s->registerObject(path,pObj);
  if( sc.isFailure() ) {
    MsgStream log(msgSvc(), name());
    log << MSG::ERROR << "Unable to register object " << path << endmsg;
  }
  return sc;
}

//--------------------------------------------------------------------
// Initialize
//--------------------------------------------------------------------
StatusCode WriteAlg::initialize() {
  MsgStream log(msgSvc(), name());
  StatusCode sc = service("FileRecordDataSvc",m_recordSvc,true);
  if( sc.isFailure() ) {
    log << MSG::ERROR << "Unable to retrieve run records service" << endmsg;
    return sc;
  }
  return put(m_recordSvc,"/FileRecords/EvtCount",m_evtCount=new Counter());
}

//--------------------------------------------------------------------
// Finalize
//--------------------------------------------------------------------
StatusCode WriteAlg::finalize() {
  MsgStream log(msgSvc(), name());
  Counter* pObj = new Counter();
  pObj->set(123456);
  put(m_recordSvc,"/FileRecords/SumCount",pObj);
  if ( m_recordSvc ) m_recordSvc->release();
  m_recordSvc = 0;
  m_evtCount = 0;
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

  m_evtCount->increment();

  // Create the Event header and set it as "root" of the event store
  Event* evt = new Event();
  evt->setEvent(++evtnum);
  evt->setRun(runnum);
  evt->setTime(Gaudi::Time::current());

  sc = eventSvc()->registerObject("/Event","Header",evt);
  if( sc.isFailure() ) {
    log << MSG::ERROR << "Unable to register Event Header" << endmsg;
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
  sc = put(eventSvc(),"/Event/Collision_1",coll1);
  if( sc.isFailure() ) return sc;
  sc = put(eventSvc(),"/Event/Collision_2",coll2);
  if( sc.isFailure() ) return sc;

  evt->addCollision(coll0);
  evt->addCollision(coll1);
  evt->addCollision(coll2);

  // Create the collection of tracks and register them in the event store
  int n = (int)(rndmflat() * 100.);
  MyTrackVector* myTracks = new MyTrackVector();
  for( int i = 0; i < n; i++ ) {
    // Create new track
    double c = rndmgauss();
    double b = rndmgauss();
    double a = rndmgauss();
    MyTrack* track = new MyTrack(float(a),float(b),float(c));
    // the following line has been replace by the previous one since
    // the order of evaluation of the rndgauss() call is unspecified
    // in the C++ standard. Don't do that.
    // MyTrack* track = new MyTrack(rndmgauss(),rndmgauss(),rndmgauss());

    // set Link to event object
    track->setEvent(evt);
    // And add the stuff to the container
    myTracks->insert ( track );
  }

  // Create vertex container
  int m = (int)(rndmflat() * 100.)+1;
  MyVertexVector* myVertices = new MyVertexVector();
  for( int j = 0; j < m; j++ ) {
    // Create new track
    double c = rndmgauss();
    double b = rndmgauss();
    double a = rndmgauss();
    MyVertex* vtx = new MyVertex(float(a)/100.0F,float(b)/100.0F,float(c)/100.0F);
    // the following line has been replace by the previous one since
    // the order of evaluation of the rndgauss() call is unspecified
    // in the C++ standard. Don't do that.
    // MyVertex* vtx = new MyVertex(rndmgauss()/100.0,
    //                              rndmgauss()/100.0,
    //                              rndmgauss()/100.0);

    // set Link to event object
    vtx->setEvent(evt);
    vtx->addCollision(coll0);
    vtx->addCollision(coll1);
    vtx->addCollision(coll2);
    // And add the stuff to the container
    myVertices->insert( vtx );
  }
  // Now connect vertices and tracks
  for(MyTrackVector::iterator k=myTracks->begin(); k !=myTracks->end(); ++k) {
    int org = (int)(rndmflat() * float(m));
    MyVertex* orgVtx = *(myVertices->begin()+org);
    (*k)->setOriginVertex(orgVtx);
    int dec1 = (int)(rndmflat() * float(m));
    int dec2 = (int)(rndmflat() * float(m));
    int tmp = dec1;
    dec1 = (tmp<dec2) ? tmp : dec2;
    dec2 = (tmp>dec2) ? tmp : dec2;
    for ( int l = dec1; l < dec2; ++l) {
      MyVertex* decVtx = *(myVertices->begin()+l);
      (*k)->addDecayVertex(decVtx);
      decVtx->setMotherParticle(*k);
    }
  }

  sc = put(eventSvc(),"/Event/MyTracks",myTracks);
  if( sc.isFailure() ) return sc;
  sc = put(eventSvc(),"/Event/Collision_0/MyVertices",myVertices);
  if( sc.isFailure() ) return sc;
  // All done
  return StatusCode::SUCCESS;
}
