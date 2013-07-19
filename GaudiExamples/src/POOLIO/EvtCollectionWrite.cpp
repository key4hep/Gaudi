// $Id: EvtCollectionWrite.cpp,v 1.7 2008/11/04 22:49:24 marcocle Exp $
//      ====================================================================
//  EvtCollection.Write.cpp
//      --------------------------------------------------------------------
//
//      Author    : Markus Frank
//
//      ====================================================================
#define EVTCOLLECTION_WRITE_CPP

// Framework include files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/SmartDataPtr.h"

// Example related include files
#include "EvtCollectionWrite.h"
#include "Event.h"
#include "MyTrack.h"

#include <cmath>

using namespace Gaudi::Examples;

DECLARE_COMPONENT(EvtCollectionWrite)

/**  Algorithm parameters which can be set at run time must be declared.
     This should be done in the constructor.
*/
EvtCollectionWrite::EvtCollectionWrite(const std::string& name, ISvcLocator* pSvcLocator)
:       Algorithm(name, pSvcLocator),
  m_evtTupleSvc(0)
{
  declareProperty("NumMcTracks", m_nMCcut=50);
}

// Standard destructor
EvtCollectionWrite::~EvtCollectionWrite()   {
}

StatusCode EvtCollectionWrite::initialize()   {
  StatusCode status = service("EvtTupleSvc", m_evtTupleSvc);
  if ( status.isSuccess() )   {
    NTuplePtr nt(m_evtTupleSvc,"/NTUPLES/EvtColl/Dir1/Dir2/Dir3/Collection");
    if ( !nt )    {    // Check if already booked
      nt = m_evtTupleSvc->book ("/NTUPLES/EvtColl/Dir1/Dir2/Dir3/Collection", CLID_ColumnWiseTuple, "Hello World");
      if ( nt )    {
        // Add an index column
        status = nt->addItem ("Ntrack",    m_ntrkColl, 0, 5000 );
        status = nt->addItem ("Energy",    m_eneColl );
        status = nt->addItem ("Track",     m_trackItem);
        status = nt->addItem ("Addr",      m_evtAddrColl);
        status = nt->addItem ("TrkMom",    m_ntrkColl, m_trkMom);
        status = nt->addItem ("TrkMomFix", 100, m_trkMomFixed);
        status = nt->addItem ("Address",   m_evtAddrCollEx);
      }
      else    {   // did not manage to book the N tuple....
        return StatusCode::FAILURE;
      }
    }
    else  {  // Just reconnect to existing items
      status = nt->item ("Ntrack",    m_ntrkColl );
      status = nt->item ("Energy",    m_eneColl );
      status = nt->item ("TrkMom",    m_trkMom);
      status = nt->item ("Track",     m_trackItem);
      status = nt->item ("TrkMomFix", m_trkMomFixed);
      status = nt->item ("Addr",      m_evtAddrColl );
      status = nt->item ("Address",   m_evtAddrCollEx );
    }
  }
  return status;
}

// Event callback
StatusCode EvtCollectionWrite::execute() {
  MsgStream log(msgSvc(), name());
  SmartDataPtr<DataObject> evtRoot(eventSvc(),"/Event");
  SmartDataPtr<Event> evt(eventSvc(),"/Event/Header");
  if ( evt != 0 )    {
    int evt_num = evt->event();
    SmartDataPtr<MyTrackVector> trkCont(eventSvc(), "/Event/MyTracks");
    if ( trkCont != 0 )    {
      // Force an object update since now the original tracks should be
      // present and the local pointers can be updated!
      m_evtAddrCollEx = evtRoot->registry()->address();
      m_evtAddrColl = evtRoot->registry()->address();
      m_ntrkColl    = trkCont->size();
      m_eneColl     = 0.f;
      int cnt = 0;
      log << " ->Track:";
      for(size_t j=0; j<100; ++j)  {
        m_trkMomFixed[j] = 0.f;
            }
      for ( MyTrackVector::iterator i = trkCont->begin(); i != trkCont->end(); i++, cnt++ )   {
        float p = float(sqrt( (*i)->px() * (*i)->px() +
                              (*i)->py() * (*i)->py() +
                              (*i)->pz() * (*i)->pz() ));
        if ( cnt < 5000 ) m_trkMom[cnt] = p;
        if ( cnt <    5 ) m_trkMomFixed[cnt] = p;
        m_eneColl += p;
      }
      m_trackItem = (0==m_ntrkColl) ? 0 : (*trkCont->begin());
      if ( evt_num < 10 || evt_num%500==0 )  {
        log << endmsg;
        log << MSG::INFO << "================ EVENT:" << evt->event() << " RUN:" << evt->run()
            << " ====== N(Track)=" << m_ntrkColl;
        if ( m_ntrkColl < m_nMCcut )   {
          log << " FAILED selection (<" << m_nMCcut << ") ============" << endmsg;
        }
        else    {
          log << " PASSED selection (>=" << m_nMCcut << ") ============" << endmsg;
        }
        if ( m_ntrkColl >= m_nMCcut )   {
          return m_evtTupleSvc->writeRecord("/NTUPLES/EvtColl/Dir1/Dir2/Dir3/Collection");
        }
      }
      return StatusCode::SUCCESS;
    }
  }
  log << MSG::ERROR << "Unable to retrieve Event Header object" << endmsg;
  return StatusCode::FAILURE;
}
