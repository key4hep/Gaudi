//	====================================================================
//  ReadAlg.cpp
//	--------------------------------------------------------------------
//
//	Package   : GaudiExamples/Example3
//
//	Author    : Markus Frank
//
//	====================================================================

// Framework include files
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/DataStoreItem.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/SmartIF.h"

// Example related include files
#include "ReadAlg.h"

// Event Model related classes
#include "Event.h"
#include "MyTrack.h"

#include "GaudiKernel/System.h"

DECLARE_ALGORITHM_FACTORY(ReadAlg);

StatusCode ReadAlg::initialize() {
  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------
// Execute
//--------------------------------------------------------------------
StatusCode ReadAlg::execute() {
  // This just makes the code below a bit easier to read (and type)
	MsgStream log(msgSvc(), name());
  SmartDataPtr<Event> evt(eventSvc(),"/Event");
  SmartDataPtr<MyTrackVector> myTracks(evt, "/MyTracks");
  if ( evt != 0 )    {
    int evt_num = evt->event();
    if ( evt_num <= 10 || evt_num%100==0 )  {
      std::string store_name = "Unknown";
      std::string db_name, cnt_name;
      IRegistry* pReg = evt->registry();
      if ( pReg )  {
        SmartIF<IService> isvc(pReg->dataSvc());
        if ( isvc )  {
          store_name = isvc->name();
        }
        IOpaqueAddress* paddr = pReg->address();
        if ( paddr )  {
          db_name = paddr->par()[0];
          cnt_name = paddr->par()[1];
        }
      }
      log << MSG::INFO << "========= EVENT[" << (unsigned long) evt.ptr()
        << "@" << store_name << "]:"
        << evt->event() << " RUN:" << evt->run()
        << " TIME:" << evt->time();
      log << " @" << (void*)&evt->collisions();
      log << endmsg;
      evt->collisions().size();
      for (size_t nc=0; nc<evt->collisions().size(); ++nc) {
        if ( evt->collisions()[nc] )  {
          log << " " << evt->collisions()[nc]->collision();
          log << " (" << evt->collisions()[nc].hintID() << ",";
          log << " (" << evt->collisions()[nc]->m_mcTags.size()
	      << ")";
        }
        else  {
          log << " ===";
        }
      }
      log << endmsg;
      if ( !db_name.empty() ) {
        log << "    ===== DB:" << db_name
            << " : " << cnt_name << endmsg;
      }
    }
    SmartDataPtr<MyTrackVector> myTracks(evt, "/MyTracks");
    SmartDataPtr<MyVertexVector> myVtx(evt, "/Collision_0/MyVertices");
    if ( myTracks != 0 )    {
      IRegistry* pReg = myTracks->registry();
      if ( pReg )  {
        IOpaqueAddress* paddr = pReg->address();
        if ( paddr )  {
          log << "          DB:" << paddr->par()[0]
              << " : " << paddr->par()[1] << endmsg;
        }
      }
    }
    else  {
      log << "   No tracks.";
    }
    if ( myVtx != 0 )    {
      IRegistry* pReg = myVtx->registry();
      if ( pReg )  {
        IOpaqueAddress* paddr = pReg->address();
        if ( paddr )  {
          log << "          DB:" << paddr->par()[0]
              << " : " << paddr->par()[1] << endmsg;
        }
      }
    }
    else  {
      log << "   No vertices.";
    }
    if ( myTracks != 0 )    {
      int count = 0;
      for ( MyTrackVector::iterator i = myTracks->begin(); i != myTracks->end() && count++ < 5; i++ )   {
      	try   {
	        if ( evt->event() < 10 || evt->event()%500==0 )  {
	          log << MSG::DEBUG << "Evt:";
            if ( (*i)->event() )  {
              log << (*i)->event()->event();
            }
            else  {
              log << "Unknown";
            }
            log << " Track:";
	          log.width(12);          log << (*i)->px();
	          log.width(12);          log << (*i)->py();
	          log.width(12);          log << (*i)->pz();
	          log << " Org:";
            if ( (*i)->originVertex() )  {
	            log.width(4);           log << (*i)->originVertex()->index();
	            log.width(10);          log << (*i)->originVertex()->x();
	            log.width(10);          log << (*i)->originVertex()->y();
	            log.width(10);          log << (*i)->originVertex()->z();
            }
            else {
              log << "No Origin Vertex!";
            }
	          log << endmsg << "   Decays:";
	          log.width(4);           log << (*i)->decayVertices().size();
	          log << endmsg;
	          for (size_t id = 0; id < (*i)->decayVertices().size(); ++id) {
	            const MyVertex* v = (*i)->decayVertices()[id];
              if ( v )  {
	              log << "    ";
	              log.width(10);        log << v->x();
	              log.width(10);        log << v->y();
	              log.width(10);        log << v->z() << " Collisions:";
	              for (size_t ic = 0; ic < v->collisions().size(); ++ic) {
      		        log.width(3);
                  if ( v->collisions()[ic] )  {
                    log << v->collisions()[ic]->collision();
                    log << " (" << v->collisions()[ic].hintID() << ")";
                  }
                  else {
                    log << "-";
                    log << " (" << v->collisions()[ic].hintID() << ")";
                  }
	              }
              }
              else  {
                log << "No decay vertex!";
              }
	            log << endmsg;
	          }
	        }
	      }
	      catch(...)    {
	        log << MSG::ERROR << "Exception occurred!" << endmsg;
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
    }
    return StatusCode::SUCCESS;
  }
  SmartDataPtr<DataObject> evt_root(eventSvc(),"/Event");
  if ( evt_root )   {
    log << MSG::INFO << "Type of /Event is:" << typeid(*(evt_root.ptr())).name() << endmsg;
    return StatusCode::SUCCESS;
  }
  else  {
    log << MSG::ERROR << "Unable to retrieve Event object" << endmsg;
  }
  return StatusCode::FAILURE;
}
